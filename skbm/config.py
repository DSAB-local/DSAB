from easydict import EasyDict as edict
from os import path as osp
from pathlib import Path
import os

__C = edict()

__C.PATH = edict()
__C.PATH.root_dir = os.environ['ROOT_DIR'] 
__C.PATH.pkg_dir = osp.join(__C.PATH.root_dir, 'skbm')
__C.PATH.figure_dir = osp.join(__C.PATH.root_dir, 'figure')
__C.PATH.dataset_dir = osp.join(__C.PATH.root_dir, 'dataset')
# __C.PATH.gen_dataset_dir = osp.join(__C.PATH.dataset_dir, 'gen')
__C.PATH.gen_dataset_dir = __C.PATH.dataset_dir
Path(__C.PATH.gen_dataset_dir).mkdir(exist_ok=True, parents=True)
__C.PATH.defaultDatasetFile = osp.join(__C.PATH.dataset_dir, 'defaultDataset.json')

__C.PATH.sketch_dir = osp.join(__C.PATH.pkg_dir, 'new_sketch')
__C.PATH.execute_file =  osp.join(__C.PATH.sketch_dir, 'task', 'AAA.out')
__C.PATH.output_dir = osp.join(__C.PATH.root_dir, 'experiment', 'output')
Path(__C.PATH.output_dir).mkdir(exist_ok=True, parents=True)
__C.PATH.graph_dir = osp.join(__C.PATH.root_dir, 'experiment', 'graph')
Path(__C.PATH.graph_dir).mkdir(exist_ok=True, parents=True)

__C.bytePerStr = 4

__C.db_name = 'sketch_test'

__C.TEMP = edict()
__C.TEMP.tasks = [
	                {
	                    'name': 'freq',
	                    'params': [],
	                },
	                {
	                    'name': 'topk',
	                    'params': [{
	                        'field': 'k',
	                        'type': 'int',
	                        'help': 'xxx',
	                    }],
	                },
	                {
	                    'name': 'speed',
	                    'params': [],
	                },
	            ]

cfg = __C
