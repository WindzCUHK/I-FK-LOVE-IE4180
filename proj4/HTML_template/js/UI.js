var UI = {};

UI.abc = function (b) {
	console.log([b[1], b[0]]);
}

UI.eeee = function (a) {
	console.log(a);
}

init(false, UI.eeee, this, ['asdfasdfas']);
init(false, UI.abc, this, [[2, 3]]);
