from flask import (
    Blueprint, flash, g, redirect, render_template, request, url_for, send_from_directory, send_file
)
from werkzeug.exceptions import abort

#----------------------added by Yucheng
from pprint import pprint
from os import path as osp
import os
from skbm.db import get_db, query_results, generate_dataset
from skbm.config import cfg
import json
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import uuid
import subprocess

bp = Blueprint('skbm', __name__, url_prefix='/skbm')

@bp.route('/test')
def test():
    return render_template('test.html')

@bp.route('/index.html')
def index():
    # return send_from_directory('templates','index.html')
    return render_template('index.html')

@bp.route('/addSketch.html')
def addSketch():
    return render_template('addSketch.html')

@bp.route('/addDataset.html')
def addDataset():
    return render_template('addDataset.html')

@bp.route('/api', methods=['GET','POST'])
def api():
    arg_get = request.args.get('get','');
    if request.method == 'GET':
        if arg_get:
            if arg_get == 'datasetList':
                db = get_db()
                # ret = []
                # for dct in db.dataset_info.aggregate([{'$project': {'name': 1,'_id':0}}]):
                #     ret.append(dct['name'])
                ret = list(db.dataset_info.aggregate([{'$project': {'name': 1,'_id':0,'distriName':1, 'distinctNum':1, 'totalNum': 1, 'bytePerItem': 1}}]))
                return json.dumps(ret)
            elif arg_get == 'sketchList':
                db = get_db()
                # ret = []
                # for dct in db.sketch_info.aggregate([{'$project': {'name': 1,'_id':0}}]):
                #     ret.append(dct['name'])
                ret = list(db.sketch_info.aggregate([{'$project': {'name': 1,'_id':0,'params': 1,'tasks': 1}}]))
                return json.dumps(ret)
            elif arg_get == 'taskList' and request.args.get('sketch',''):
                sketchName = request.args.get('sketch','')
                db = get_db()
                dct = db.sketch_info.find_one({'name': sketchName})
                return json.dumps({'tasks': dct['tasks']})
            elif arg_get == 'args' and request.args.get('sketch','') and request.args.get('task',''):
                sketchName = request.args.get('sketch','')
                taskName = request.args.get('task','')
                db = get_db()
                dct = db.sketch_info.find_one({'name': sketchName})
                return json.dumps({'params': dct['params']})
            elif arg_get == 'gendataset':
                distriName = request.args.get('distriName',"")
                totalNum = int(request.args.get('totalNum',""))
                distinctNum = int(request.args.get('distinctNum',""))
                param1 = float(request.args.get('param1',""))
                param2 = request.args.get('param2',"")

                dsname = "{}_{}_{}_{}".format(distriName,totalNum,distinctNum,param1)
                if param2:
                    param2 = float(param2)
                    dsname += '_{}'.format(param2)
                    obj = generate_dataset(distriName,totalNum,distinctNum,param1,param2)
                else:
                    obj = generate_dataset(distriName,totalNum,distinctNum,param1)
                dsname += '.dat'

                return "Generate dataset: {}".format(dsname)

    elif request.method == 'POST':
        d = json.loads(request.data.decode())
        if 'flag' in d and d['flag'] == 'experiment':
            # pprint(d)
            results = []
            def incrementSteps(steps,grids):
                for idx,step in enumerate(steps):
                    grid = grids[idx]
                    if step<grid:
                        steps[idx] += 1
                        return True
                    else :
                        steps[idx] = 0
                        continue
                return False
            for datasetName in d['datasetList']:
                args_per_dataset = []
                for experiment in d['experimentList']:
                    sketchName = experiment['sketchName']
                    taskName = experiment['taskName']
                    params = experiment['params']
                    steps, grids = [0 for __ in range(len(params))], []
                    totSteps = 1
                    for param in params:
                        if not (param['to'] or param['step']):
                            # only have param['from']
                            param['to'], param['step'] = 0, 1
                            grids.append(0)
                            continue
                        else:
                            # have a range
                            tmp = int((int(param['to'])-int(param['from']))//int(param['step']))
                            if tmp < 1:
                                grids.append(0)
                                continue
                            else :
                                grids.append(tmp)
                                totSteps *= tmp + 1
                    # pprint(totSteps)
                    while totSteps>0:
                        totSteps -= 1
                        arg = {
                            'datasetName': datasetName,
                            'sketchName': sketchName,
                            'taskName': taskName,
                        }
                        for idx,step in enumerate(steps):
                            param = params[idx]
                            v = steps[idx] * int(param['step']) + int(param['from'])
                            arg[param['field']] = v
                        args_per_dataset.append(arg)
                        if not incrementSteps(steps,grids):
                            break
                # pprint(args_per_dataset)
                queryResults = query_results(args_per_dataset)
                results += queryResults
            # pprint(results)
            ret = getGraphOptions(results)
            pprint(ret)
            return json.dumps(ret)

        elif 'flag' in d and d['flag'] == 'graph':
            yaxis, xaxis, multilines = d['yaxis'], d['xaxis'], d['multilines']
            line2results = {}
            lines = []
            for result in d['results']:
                if yaxis in result['taskResult'] and xaxis in result and multilines in result:
                    lineName = result[multilines]
                    line2results[lineName] = line2results.get(lineName,[]) + [result]
            for lineName, results in line2results.items():
                X,Y = [],[]
                for result in results:
                    X.append(result[xaxis])
                    Y.append(result['taskResult'][yaxis])
                line = {
                    'X': X,
                    'Y': Y,
                    'xlabel': xaxis,
                    'ylabel': yaxis,
                    'linelabel': str(lineName),
                }
                lines.append(line)
            img_path = drawGraph(lines)
            return img_path

        elif 'flag' in d and d['flag'] == 'graph2':
            # pprint(d)
            results, pointList, yaxis, xlabel = d['results'], d['pointList'], d['yaxis'], d['xlabel']
            lines = {}
            for point in pointList:
                experimentIdx = int(point['experimentIdx'])
                lines[point['line']] = lines.get(point['line'], []) + [(float(point['index']), results[experimentIdx]['taskResult'][yaxis])]
            lst = []
            for lineName, points in lines.items():
                Y = list(map(lambda tup: tup[1], sorted(points, key=lambda tup: tup[0])))
                X = list(map(lambda tup: tup[0], sorted(points, key=lambda tup: tup[0])))
                line = {
                    'X': X,
                    'Y': Y,
                    'xlabel': xlabel,
                    'ylabel': yaxis,
                    'linelabel': lineName,
                }
                lst.append(line)
            img_path = drawGraph(lst)
            return img_path

        else :
            dct = {
                'name': d['sketchName'],
                'path': osp.join(cfg.PATH.sketch_dir, 'sketch',d['sketchName']+'.h'),
                'params': d['params'],
                'code': d['code'],
                'tasks': cfg.TEMP.tasks,
            }
            with open(dct['path'],'w') as hd:
                hd.write(dct['code'])
            try:
                p = subprocess.Popen(' '.join(['cd',cfg.PATH.sketch_dir+'/task','&&','make','AAA.out','&&','cd','-']),shell=True, stdout=subprocess.PIPE,stderr=subprocess.PIPE)
                # p = subprocess.Popen('cd {} && ls'.format(cfg.PATH.sketch_dir+'/task'),shell=True,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
                # p.wait()
                tup = p.communicate()
                if p.poll():
                    os.popen('rm {}'.format(dct['path']))
                    errMessage = tup[1].decode()
                    print(errMessage)
                    return errMessage
                print("Compile successfully!")
                # compiledResults = os.popen('cd {} && make AAA.out && cd -'.format(cfg.PATH.sketch_dir+'/task')).read()
                db = get_db()
                db.sketch_info.remove({'name':dct['name']})
                db.experiment.delete_many({'sketchName': dct['name']})
                msg = db.sketch_info.insert(dct)
                print(msg)
                # return compiledResults
                return "Compile successfully!"
            except Exception as e:
                os.popen('rm {}'.format(dct['path']))
                print(str(e))
                return str(e)


    return 'NoNoNo...'

@bp.route('/graph')
def graph():
    img_path = request.args.get('uuid',"")
    print(img_path)
    return send_file(img_path)

def getGraphOptions(results):
    ret = {
        'results': results,
    }
    yaxis = set()
    variables = set()
    for idx,result in enumerate(results):
        # Y axis
        for y in result['taskResult'].keys(): 
            if y not in ['totalNum']:
                yaxis.add(y)
        # variables
        for var in result.keys():
            if var not in ['output_filename','taskResult']:
                variables.add(var)
    ret['yaxis'] = list(yaxis)
    ret['variables'] = list(variables)

    return ret

def drawGraph(lines):

    generateDataFile(lines)
    unique_id = generateListJson(lines)

    p = subprocess.Popen(' '.join(['python', cfg.PATH.figure_dir+'/run.py', cfg.PATH.figure_dir+'/list.json']), shell=True, stdout=subprocess.PIPE,stderr=subprocess.PIPE)
    tup = p.communicate()
    if p.poll():
        errMessage = tup[1].decode()
        print(errMessage)
        return errMessage
    print("Generated graph {}.png".format(unique_id))
    img_path = osp.join(cfg.PATH.graph_dir,str(unique_id)+'.png')
    return img_path

    # plt.figure()
    # for line in lines:
    #     plt.plot(line['X'],line['Y'], label=line['linelabel'])
    # plt.legend()
    # plt.xlabel(line['xlabel'])
    # plt.ylabel(line['ylabel'])
    # unique_id = uuid.uuid1()
    # img_path = osp.join(cfg.PATH.graph_dir,str(unique_id)+'.png')
    # plt.savefig(img_path)
    # plt.clf()
    # return img_path


def transfertoLatex(str):
    return str.replace('_','\_')
def generateDataFile(lines):
    file_path = osp.join(cfg.PATH.figure_dir,'data.dat')
    f = open(file_path,'w')
    f.write(transfertoLatex(lines[0]['xlabel']))
    for line in lines:
        f.write('\t'+transfertoLatex(line['linelabel']))
    for i in range(len(lines[0]['X'])):
        f.write('\n')
        f.write(yvalueMake(str(lines[0]['X'][i])))
        for tmp in lines:
            f.write('\t' + str(tmp['Y'][i]))

def generateListJson(lines):
    dict  ={}
    dict['file']  = osp.join(cfg.PATH.figure_dir,'data.dat')
    unique_id = uuid.uuid1()
    dict['output'] = osp.join(cfg.PATH.graph_dir,"{}.png".format(unique_id))
    if ylabelMake(lines[0]['ylabel']).find('Throughput') != -1:
        dict['chart.type'] = 'bar'
        dict['style'] = 2
    else:
        dict['chart.type']  = 'line'
        dict['style'] = 2
    dict['separator'] = '\t'
    dict['y_label'] = ylabelMake(lines[0]['ylabel'])
    json_path  =osp.join(cfg.PATH.figure_dir,'list.json')
    f = open(json_path,'w')
    jsonObj = json.dumps(dict)
    f.write('['+jsonObj+']')
    return unique_id

def ylabelMake(str):
    if str.find('precision')!=-1:
        return  str.replace('precision','Precision')
    if str.find('throughput')!=-1:
        return str.replace('throughput','Throughput (MIPS)')
    return str
def yvalueMake(str):
    tmp = float(str)
    if(tmp>1000000):
        return str(round(tmp/1000000,2))+"M"
    if(tmp>10000):
        return  str(round(tmp/1000,2))+"K"
    return str



# from . import auth
# app.register_blueprint(auth.bp)

# @bp.route('/')
# def index():
#     db = get_db()
#     datasets = db.dataset_info.find({})
#     sketches = db.sketch_info.find({})
#     return render_template('blog/index.html', 
#       datasets=datasets,
#       sketches=sketches,
#       )
