(function(){
'use strict';

angular.module('addDatasetApp',[])
.controller('DatasetGeneratorController', DatasetGeneratorController)
.service('requestService', requestService)
.constant('baseURL', "http://"+document.location.host+"/skbm/api");

DatasetGeneratorController.$inject = ['requestService'];
function DatasetGeneratorController(requestService) {
	var dsgc = this;

	dsgc.distriName = "";
	dsgc.distriNameList = ["zipf","powerlaw","weibull"];

	dsgc.totalNum = "";
	dsgc.distinctNum = "";
	dsgc.param1 = "";
	dsgc.param2 = "";

	dsgc.result = "";

	dsgc.gen = function(){
		console.log("hihihi");
		var promise = requestService.generateCmd(dsgc.distriName,dsgc.totalNum,dsgc.distinctNum,dsgc.param1,dsgc.param2);

		promise.then(function(response){
			dsgc.result = response.data;
		}).catch(function(error){dsgc.result = error;});

	};

}

requestService.$inject = ['$http','baseURL'];
function requestService($http, baseURL) {
	var service = this;

	service.generateCmd = function(distriName,totalNum,distinctNum,param1,param2) {
		var response = $http({
			method: "GET",
			url: baseURL,
			params: {
				get: "gendataset",
				distriName: distriName,
				totalNum: totalNum,
				distinctNum: distinctNum,
				param1: param1,
				param2: param2
			}
		});
		return response;
	};
}


})();