(function(){
'use strict';

angular.module('addSketchApp',[])
.controller('scriptController', scriptController)
.service('requestService', requestService)
.constant('baseURL', "http://"+document.location.host+"/skbm/api");

scriptController.$inject = ['requestService'];
function scriptController(requestService) {
	var sc = this;

	sc.sketchName = "";
	sc.params = [];
	sc.codeText = "";
	sc.result="";
	sc.addParam = function() {
		sc.params.push({
			field: "",
			type: "",
			help: ""
		});
	};
	sc.removeParam = function(idx) {
		sc.params.splice(idx,1);
	};
	sc.submitCode = function() {
		var d = {
			sketchName: sc.sketchName,
			params: sc.params,
			code: sc.codeText
		};
		var promise = requestService.postCode(d);
		promise.then(function(response) {
			sc.result = response.data;
		})
		.catch(function(error){
			console.log(error);
		});
	}
}

requestService.$inject = ['$http','baseURL'];
function requestService($http, baseURL) {
	var service = this;

	service.postCode = function(d) {
		var response = $http({
			method: 'POST',
			url: baseURL,
			data: d
		});
		return response;
	}
}


})();
