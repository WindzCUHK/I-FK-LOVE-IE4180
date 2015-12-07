"use strict";

var myApp = {};
myApp.callQueue = [];

function init(e, queueFunction, funcThis, args) {
	var currentCall;

	if (!e) {
		myApp.callQueue.push([queueFunction, funcThis, args]);
	} else {
		while (myApp.callQueue.length !== 0) {
			currentCall = myApp.callQueue.shift();
			currentCall[0].apply(currentCall[1], currentCall[2]);
		}
	}
}

window.addEventListener('load', init, false);
