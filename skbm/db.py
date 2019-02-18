import click
from flask import current_app, g
from flask.cli import with_appcontext

from pymongo import MongoClient, ASCENDING, DESCENDING
from pymongo.errors import DuplicateKeyError
from bson.objectid import ObjectId
from bson.errors import InvalidId
from sys import maxsize
from datetime import datetime

from pprint import pprint
# from os import environ

#-------------------------
import json
from skbm.config import cfg
from os import path as osp
import os
import subprocess
import numpy as np
from skbm.generate_dataset import dataset_write
from pathlib import Path

def InteractiveIf(msg=None, no_func=None, divider=True):
    if msg is None:
        msg = "Do you want to execute the function?"
    if no_func is None:
        def temp_func():
            print("Ignored!")
        no_func = temp_func
    def mainDecorator(yes_func):
        def wrapper(*args, **kwargs):
            if divider:
                print('-'*20)
            flag = 'a'
            while flag not in ['y','n']:
                flag = input("{} (y/n)".format(msg))
                if flag == 'y':
                    yes_func(*args, **kwargs)
                elif flag=='n':
                    no_func(*args, **kwargs)
                else:
                    print("invalid input: {}".format(flag))
        return wrapper
    return mainDecorator

def get_db():
    if 'db' not in g:
        g.db = MongoClient(current_app.config['DATABASE'])[cfg.db_name]
    return g.db

def close_db(e=None):
    db = g.pop('db', None)
    if db is not None:
        db.client.close()

def init_db():
    dropFlag = 'a'
    while dropFlag not in ['y','n']:
        dropFlag = input("Drop current database or not? (y/n)")
        if dropFlag == 'y':
            db = get_db()
            db.client.drop_database(cfg.db_name)
            print("current database dropped!")
        elif dropFlag=='n':
            print("ignored")
        else:
            print("invalid input: {}".format(dropFlag))
    print('-'*20)

@click.command('init-db')
@with_appcontext
def init_db_command():
    rewrite_root_dir()
    init_db()
    init_existing_dataset()
    init_existing_sketch()
    # click.echo('Initialized the mongo database.')

def init_app(app):
    app.teardown_appcontext(close_db)
    app.cli.add_command(init_db_command)

def rewrite_root_dir():
    lst = ['main.cpp','test.cpp']
    for item in lst:
        file_path = Path(cfg.PATH.root_dir) / 'skbm/new_sketch/task' / item
        text = file_path.read_text()
        text = text.replace('#define ROOT_DIR "/root/pku-sketch-benchmark/"', '#define ROOT_DIR "{}/"'.format(str(cfg.PATH.root_dir)))
        with open(str(file_path), 'w') as hd:
            hd.write(text)
        # print('Rewrite root dir in file {}'.format(str(file_path)))

@InteractiveIf(msg="This is not a default dataset file. Do you want to remove this dataset?", divider=False)
def remove_one_dataset(path):
    os.remove(str(path))

@InteractiveIf(msg="Do you want to download this file?", divider=False)
def download_one_dataset(name):
    path = Path(cfg.PATH.dataset_dir) / name
    import subprocess
    print('Start downloading from cloud server...')
    p = subprocess.Popen(' '.join(['wget','-O',str(path),"http://dsabdatasets.oss-cn-beijing.aliyuncs.com/{}".format(name)]),shell=True, stdout=subprocess.PIPE,stderr=subprocess.PIPE)
    tup = p.communicate()
    if p.poll():
        errMessage = tup[1].decode()
        print(errMessage)
        raise Exception
    print('{} was downloaded to {}'.format(name, str(path)))

def init_existing_dataset():
    dropFlag = 'a'
    while dropFlag not in ['y','n']:
        dropFlag = input("Initiate dataset information or not? (y/n)")
        if dropFlag == 'y':
            db = get_db()
            db.drop_collection('dataset_info')
            from pathlib import Path
            iter_dataset = Path(cfg.PATH.dataset_dir).glob('*.dat')
            default_dct = json.loads(open(cfg.PATH.defaultDatasetFile).read())
            lst = []
            for d in iter_dataset:
                if d.name not in default_dct:
                    print(d.name)
                    remove_one_dataset(d)
            for name in default_dct:
                path = Path(cfg.PATH.dataset_dir) / name
                if not path.exists():
                    print('{} not exists!'.format(name))
                    download_one_dataset(name)
                info = default_dct[name]
                info['path'] = str(path)
                info['bytePerItem'] = 4
                lst.append(info)
            # for d in iter_dataset:
            #     info = default_dct[d.name]
            #     lst.append({
            #             'name': d.name,
            #             'path': str(d),
            #             'bytePerItem': int(info['bytePerItem']),
            #             'distinctNum': info['distinctNum'],
            #             'maxFrequency': int(info['maxFrequency']),
            #             'minFrequency': int(info['minFrequency']),
            #             'totalNum': int(info['totalNum']),
            #         })
            db.dataset_info.insert_many(lst)
            print('Initiated existing datasets!')
        elif dropFlag=='n':
            print("ignored")
        else:
            print("invalid input: {}".format(dropFlag))
    print('-'*20)

def init_existing_sketch():
    dropFlag = 'a'
    while dropFlag not in ['y','n']:
        dropFlag = input("Initiate sketch information or not? (y/n)")
        if dropFlag == 'y':
            db = get_db()
            db.drop_collection('sketch_info')
            sketchList = json.loads(open(osp.join(cfg.PATH.root_dir,'staticSketchList.json')).read())
            for dct in sketchList:
                dct['path'] = str(Path(cfg.PATH.root_dir) / dct['path'])
            msg = db.sketch_info.insert_many(sketchList)
            # print(msg)
            print('Initiated existing sketches!')
        elif dropFlag=='n':
            print("ignored")
        else:
            print("invalid input: {}".format(dropFlag))
    import subprocess
    p = subprocess.Popen(' '.join(['cd',cfg.PATH.sketch_dir+'/task','&&','make','AAA.out','&&','cd','-']),shell=True, stdout=subprocess.PIPE,stderr=subprocess.PIPE)
    tup = p.communicate()
    if p.poll():
        os.popen('rm {}'.format(dct['path']))
        errMessage = tup[1].decode()
        print(errMessage)
        return errMessage
    print("Compiled successfully!")
    print('-'*20)

def query_results(args_per_dataset):
    db = get_db()
    results = []
    args_to_experiment = []
    for arg in args_per_dataset:
        result = db.experiment.find_one(arg)
        if result:
            result.pop('_id')
        if result:
            results.append(result)
        else :
            args_to_experiment.append(arg)
    # pprint(args_to_experiment)
    cmd_args = set()
    for arg in args_to_experiment:
        lst = [
            arg['datasetName'],
            arg['sketchName'],
        ]
        if arg['taskName'] == 'topk':
            alltasks = ['topk','speed','freq']
        else :
            alltasks = ['freq', 'speed']
        lst += alltasks
        kvpairsList = []
        for key in arg:
            if key not in ['datasetName', 'sketchName', 'taskName']:
                kvpairsList.append(key+'='+str(arg[key]))
        kvpairsList.sort()
        lst += kvpairsList
        cmd_args.add('+'.join(lst))
        arg['output_filename'] = [
                arg['datasetName'] + '+' +
                arg['sketchName'] + '+' +
                tskNm + '+' +
                '+'.join(kvpairsList) for tskNm in alltasks
            ]
    cmd_args = list(cmd_args)
    # cmd = '{} {}'.format(cfg.PATH.execute_file, ' '.join(cmd_args))
    # pprint(cmd)
    # pprint(list(map(lambda arg: arg['output_filename'], args_to_experiment)))
    # print("start experiments!")
    # p = subprocess.Popen(cmd, shell=True, stderr=subprocess.STDOUT)
    # p.wait()
    # print("finished all experiments!")
    for cmd_arg in cmd_args:
        cmd = '{} {}'.format(cfg.PATH.execute_file, cmd_arg)
        print('start ',cmd)
        p = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE,stderr=subprocess.PIPE)
        tup = p.communicate()
        print(tup[0].decode())
        if p.poll():
            print(tup[1].decode())
            raise Exception
        print("finished ",cmd)
        print('-'*10)

    for arg in args_to_experiment:
        # lst = [
        #     arg['datasetName'],
        #     arg['sketchName'],
        #     arg['taskName'],
        # ]
        # for key in arg:
        #     if key not in ['datasetName', 'sketchName', 'taskName','output_filename']:
        #         lst.append(key+'='+str(arg[key]))
        # resultFile = '+'.join(lst)
        for resultFile in arg['output_filename']:
            result = parseResultFile(resultFile, arg)
            # pprint(result)
            result = eval(str(result))
            db.experiment.insert_one(result.copy())
            if resultFile.split('+')[2] == arg['taskName']:
                results.append(result)

    return results

def parseResultFile(filename, arg):
    result = arg.copy()
    # taskName = arg['taskName']
    taskName = filename.split('+')[2]
    result['taskName'] = taskName
    result['output_filename'] = filename
    if taskName == 'speed':
        with open(osp.join(cfg.PATH.output_dir, filename)) as hd:
            line = hd.read()
            lst = line.strip().split()
            result['taskResult'] = {
                'totalNum': int(lst[0]),
                'time': float(lst[1]),
                'throughput': int(lst[0])/float(lst[1])/1000000.0,
            }
    elif taskName == 'freq':
        with open(osp.join(cfg.PATH.output_dir, filename)) as hd:
            trueValueList, estimatedValueList = [],[]
            for line in hd:
                trueValue, estimatedValue = list(map(int, line.strip().split()))
                trueValueList.append(trueValue)
                estimatedValueList.append(estimatedValue)
            trueValueList = np.array(trueValueList)
            estimatedValueList = np.array(estimatedValueList)

            totalNum = len(trueValueList)
            accNum = np.sum(trueValueList == estimatedValueList)
            accuracy = accNum / totalNum
            AAE = np.mean(np.abs(estimatedValueList - trueValueList))
            ARE = np.mean(np.abs(estimatedValueList - trueValueList) / trueValueList)
            result['taskResult'] = {
                # 'totalNum': totalNum,
                # 'accNum': accNum,
                'accuracy': accuracy,
                'AAE': AAE,
                'ARE': ARE,
            }
    elif taskName == 'topk':
        with open(osp.join(cfg.PATH.output_dir, filename)) as hd:
            trueSet, querySet = set(), set()
            for line in hd:
                trueItem, trueFreq, queryItem, queryFreq = list(map(int, line.strip().split()))
                trueSet.add(trueItem)
                querySet.add(queryItem)
            totalNum = len(trueSet)
            accNum = len(trueSet & querySet)
            accuracy = accNum / totalNum
            result['taskResult'] = {
                'k': totalNum,
                # 'accNum': accNum,
                'precision': accuracy,
            }
    return result

def generate_dataset(distriName,totalNum,distinctNum,param1,param2=None):
    filename = "{}_{}_{}_{}".format(distriName,totalNum,distinctNum,param1)
    if param2:
        filename += "_{}".format(param2)
    filename += '.dat'
    fp = osp.join(cfg.PATH.gen_dataset_dir, filename)
    dataset_write(fp, distriName, cfg.bytePerStr, totalNum, distinctNum, param1, param2)
    print('Generated dataset!',filename)
    db = get_db()
    obj = {
            'name': filename,
            'path': fp,
            'bytePerItem': cfg.bytePerStr,
            'distinctNum': distinctNum,
            # 'maxFrequency': int(info['maxFrequency']),
            # 'minFrequency': int(info['minFrequency']),
            'totalNum': totalNum,
            'param1': param1,
            'param2': param2,
        }
    db.dataset_info.insert_one(obj.copy())
    return obj

















