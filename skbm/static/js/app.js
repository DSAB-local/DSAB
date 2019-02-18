(function() {
'use strict';

angular.module('SketchApp', [])
.controller('datasetController', datasetController)
.controller('expController', expController)
.controller('submitController', submitController)
.controller('graphController', graphController)
.controller('graphController2', graphController2)
.service('metaService', metaService)
.service('requestService', requestService)
.constant('baseURL', "http://"+document.location.host+"/skbm/api")
.component('loadingSpinner', {
  template: '<img ng-if="$ctrl.showSpinner" class="loading-icon" src="../static/flickr-loading.gif" alt="loading">',
  controller: SpinnerController
});

SpinnerController.$inject = ['$rootScope']
function SpinnerController($rootScope) {
  var $ctrl = this;

  var cancelListener = $rootScope.$on('experiments:running', function (event, data) {
    console.log("Event: ", event);
    console.log("Data: ", data);

    if (data.on) {
      $ctrl.showSpinner = true;
    }
    else {
      $ctrl.showSpinner = false;
    }
  });

  $ctrl.$onDestroy = function () {
    cancelListener();
  };

};

function gen_popover_html(datasetList){
    for(var i in datasetList){
        var html = "";
        var dataset = datasetList[i]
        for(var key in dataset){
            if(key != '$$hashKey' && key != 'name'){
                html = html + '<div>' + key + ': ' + dataset[key] + '</div>';
            }
        }
        dataset['html'] = html;
    }
}

datasetController.$inject = ['requestService','metaService'];
function datasetController(requestService, metaService){
    var $ctrl = this;

    var promise = requestService.requestList('datasetList');
    promise.then(function(response){
        metaService.setDatasetList(response.data);
        $ctrl.datasetList = metaService.getDatasetList();
        console.log($ctrl.datasetList);
        $(function(){
            $('[data-toggle="popover"]').popover()
        });
        gen_popover_html($ctrl.datasetList);

        $ctrl.choose(2);  // default choice
    })
    .catch(function(error){
        console.log('Error when requesting datasetList');
    });

    $ctrl.choose = function(idx) {
        if($ctrl.datasetList[idx].state){
            $ctrl.datasetList[idx].state = '';
        }
        else {
            $ctrl.datasetList[idx].state = 'active';
        }
    };
}

function deepCopy(obj){
    if(typeof obj != 'object'){
        return obj;
    }
    var newobj = {};
    for ( var attr in obj) {
        if(attr!=="$$hashKey")
            newobj[attr] = deepCopy(obj[attr]);
    }
    return newobj;
}

expController.$inject = ['requestService','metaService','$rootScope'];
function expController(requestService,metaService, $rootScope) {
    var $ctrl = this;

    $ctrl.running = false;
    var cancelListener = $rootScope.$on('experiments:running', function (event, data) {
        console.log("Event: ", event);
        console.log("Data: ", data);

        if (data.on) {
          $ctrl.running = true;
        }
        else {
          $ctrl.running = false;
        }
    });
    $ctrl.$onDestroy = function () {
        cancelListener();
    };

    var promise = requestService.requestList('sketchList');
    promise.then(function(response){
        metaService.setSketchList(response.data);
        $ctrl.sketchList = metaService.getSketchList();

        // default choices
        $ctrl.writeSketchName(25, 0);
        $ctrl.writeTaskName('freq', 0);
        for(var i in $ctrl.experimentList[0].params){
            var param = $ctrl.experimentList[0].params[i];
            if(param.field === 'memory_in_bytes'){
                param.from = "200000";
                param.to = "1000000";
                param.step = "200000";
            }
            else if(param.field === 'hash_num'){
                param.from = "3";
            }
        }
    })
    .catch(function(error){
        console.log('Error when requesting sketchList');
    });

    $ctrl.experimentList = metaService.getExperimentList();

    $ctrl.writeSketchName = function(sketchIdx, experimentIdx) {
        $ctrl.experimentList[experimentIdx].sketchName = $ctrl.sketchList[sketchIdx].name;
        $ctrl.experimentList[experimentIdx].chosenSketch = $ctrl.sketchList[sketchIdx];
        // refresh params
        var taskName = $ctrl.experimentList[experimentIdx].taskName;
        if(taskName === 'freq' || taskName === 'topk' || taskName === 'speed'){
            var task = $ctrl.experimentList[experimentIdx].chosenSketch.tasks.find(function(task){return taskName==task.name;})
            $ctrl.experimentList[experimentIdx].params = $ctrl.experimentList[experimentIdx].chosenSketch.params.concat(task.params);
            for(var i in $ctrl.experimentList[experimentIdx].params){
                var param = $ctrl.experimentList[experimentIdx].params[i];
                param.from = "";
                param.to = "";
                param.step = "";
            }
        }
    };
    $ctrl.writeTaskName = function(name, experimentIdx) {
        $ctrl.experimentList[experimentIdx].taskName = name;
        var task = $ctrl.experimentList[experimentIdx].chosenSketch.tasks.find(function(task){return name==task.name;})
        $ctrl.experimentList[experimentIdx].params = $ctrl.experimentList[experimentIdx].chosenSketch.params.concat(task.params);
        for(var i in $ctrl.experimentList[experimentIdx].params){
            var param = $ctrl.experimentList[experimentIdx].params[i];
            param.from = "";
            param.to = "";
            param.step = "";
        }
    };

    $ctrl.addExperiment = function(){
        var L = $ctrl.experimentList.length;
        if(L==0){
            $ctrl.experimentList.push({
             sketchName: "Sketch",
             taskName: "Task"
            });
        }
        else {
            // var lastExperiment = $ctrl.experimentList[L-1];
            // console.log(lastExperiment);
            // $ctrl.experimentList.push(deepCopy(lastExperiment));
            $ctrl.experimentList.push({
             sketchName: "Sketch",
             taskName: "Task"
            });
        }
    }

    $ctrl.remove = function(idx) {
        console.log("remove",idx);
        $ctrl.experimentList.splice(idx, 1);
    }
}

submitController.$inject = ['requestService','metaService', '$rootScope'];
function submitController(requestService,metaService, $rootScope) {
    var $ctrl = this;

    $ctrl.running = false;

    $ctrl.submitExperiments = function(){
        $rootScope.$broadcast('experiments:running', {on: true});
        $ctrl.running = true;

        $ctrl.datasetList = metaService.getDatasetList();
        $ctrl.experimentList = metaService.getExperimentList();
        var dsList = [];
        for(var i in $ctrl.datasetList) {
            var dataset = $ctrl.datasetList[i];
            if(dataset.state) dsList.push(dataset.name);
        }
        // console.log(dsList);
        var d = {
            'flag': "experiment",
            'datasetList': dsList,
            'experimentList': $ctrl.experimentList
        };
        var promise = requestService.postExperiments(d);
        promise
        .then(function(response){
            metaService.setResult(response.data);
        })
        .catch(function(error){
            console.log(error);
        })
        .finally(function () {
            $rootScope.$broadcast('experiments:running', {on: false});
            $ctrl.running = false;
        });
    };
}

graphController.$inject = ['requestService','metaService'];
function graphController(requestService,metaService){
    var $ctrl = this;

    $ctrl.result = metaService.getResult();

    $ctrl.chosen_yaxis = "";
    $ctrl.chosen_xaxis = "";
    $ctrl.chosen_multilines = "";

    $ctrl.graphLink = "";

    $ctrl.showSelection = function() {
        console.log($ctrl.chosen_yaxis+" "+$ctrl.chosen_xaxis+" "+$ctrl.chosen_multilines);
    };
    $ctrl.draw = function() {
        var d = {
            "flag": "graph",
            "results": $ctrl.result.results,
            "yaxis": $ctrl.chosen_yaxis,
            "xaxis": $ctrl.chosen_xaxis,
            "multilines": $ctrl.chosen_multilines
        };
        var promise = requestService.postGraph(d);
        promise.then(function(response){
            $ctrl.graphLink = "http://"+document.location.host+"/skbm/graph?uuid="+response.data;
        }).catch(function(error){
            console.log(error);
        });
    }
}

graphController2.$inject = ['requestService','metaService'];
function graphController2(requestService,metaService){
    var $ctrl = this;

    $ctrl.chosen_yaxis = "default";
    $ctrl.xlabel = "X Label";

    $ctrl.result = metaService.getResult();

    $ctrl.pointList = [
        {
            "line": "CM",
            "index": "1",
            "experimentIdx": "0",
        }
    ];

    $ctrl.num_points = "1";
    $ctrl.num_lines = "1";

    $ctrl.search = "";

    $ctrl.addPoint = function() {
        var L = $ctrl.pointList.length;
        if($ctrl.pointList[L-1]["index"] && $ctrl.pointList[L-1]["experimentIdx"]){
            $ctrl.pointList.push(
                {
                    "line": $ctrl.pointList[L-1]["line"],
                    "index": (parseInt($ctrl.pointList[L-1]["index"]) + 1).toString(),
                    "experimentIdx": (parseInt($ctrl.pointList[L-1]["experimentIdx"]) + 1).toString(),
                }
            );
        }
        else {
            $ctrl.pointList.push(
                {
                    "line": $ctrl.pointList[L-1]["line"],
                    "index": $ctrl.pointList[L-1]["index"],
                    "experimentIdx": $ctrl.pointList[L-1]["experimentIdx"],
                }
            );
        }
    };

    $ctrl.deletePoint = function() {
        console.log("hello");
        $ctrl.pointList.splice($ctrl.pointList.length-1,1);
    };

    $ctrl.draw = function() {
        var d = {
            "flag": "graph2",
            "pointList": $ctrl.pointList,
            "results": $ctrl.result.results,
            "yaxis": $ctrl.chosen_yaxis,
            "xlabel": $ctrl.xlabel
        };
        var promise = requestService.postGraph2(d);
        promise.then(function(response){
            $ctrl.graphLink = "http://"+document.location.host+"/skbm/graph?uuid="+response.data;
            window.open($ctrl.graphLink);
        }).catch(function(error){
            console.log(error);
        });
    };
}


function metaService() {
    var service = this;

    var datasetList = [];
    var sketchList = [];
    var experimentList = [
        {
            sketchName: "Sketch",
            taskName: "Task"
        }
    ];
    var result = {};
    service.getDatasetList = function(){return datasetList};
    service.getSketchList = function(){return sketchList};
    service.getExperimentList = function(){return experimentList};
    service.getResult = function(){return result};
    service.setDatasetList = function(data){datasetList = data;};
    service.setSketchList = function(data){sketchList = data;};
    service.setResult = function(data){
        for(var key in data){
            result[key] = data[key];
        }
        console.log(result);
    };
}

requestService.$inject = ['$http','baseURL'];
function requestService($http,baseURL) {
    var service = this;

    service.requestList = function(listname) {
        var response = $http({
            method: 'GET',
            url: baseURL,
            params: {
                get: listname
            }
        });
        return response
    };

    service.postExperiments = function(d) {
        var response = $http({
            method: 'POST',
            url: baseURL,
            data: d
        });
        return response;
    };

    service.postGraph = function(d) {
        var response = $http({
            method: 'POST',
            url: baseURL,
            data: d
        });
        return response;
    };

    service.postGraph2 = function(d) {
        var response = $http({
            method: "POST",
            url: baseURL,
            data: d
        });
        return response;
    };
}

function incrementSteps(steps,grids){
    for(var i in steps){
        var step = steps[i];
        var grid = grids[i];
        if(step < grid){
            steps[i]++;
            return;
        }
        else {
            continue;
        }
    }
}


})();


















