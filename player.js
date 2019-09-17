window.onload = function () {
	//console.log("load soundfont");
	MIDI.loadPlugin({
		soundfontUrl: "./soundfont/basic/",
		instruments: [ "acoustic_grand_piano", "synth_drum" ],
		onprogress: function(state, progress) {
			//console.log(state, progress);
		},
		onsuccess: function() {
			MIDI.setVolume(0, 127);
			MIDI.channels[0].instrument=0;
		}
	});
};
window.mgnr={
	"noteOn":function (info,tone,v) {
		MIDI.noteOn(0, tone, v, 0);
	},
	"noteOff":function (info,tone) {
		MIDI.noteOff(0, tone, 0);
	},
	"loadName":function (info) {
		console.log("use:"+info);
	},
	"synth":{
		"init":function (num) {
			window.mgnr.engine=document.getElementById("synth-engine");
			window.mgnr.engine.contentWindow.postMessage({
				"mode"		:"init",
				"channels"	:num
			},'*');
		},
		"addWord":function (id,vname,tone,vol,sec) {
			window.mgnr.engine.contentWindow.postMessage({
				"mode"	:"addWord",
				"name"	:vname,
				"tone"	:tone,
				"volume"	:vol,
				"length"	:sec
			},'*');
		},
		"addPause":function (id,len) {
			window.mgnr.engine.contentWindow.postMessage({
				"mode"	:"addPause",
				"id"		:id,
				"length"	:len
			},'*');
		},
		"start":function () {
			window.mgnr.engine.contentWindow.postMessage({
				"mode":"start"
			},'*');
		}
	},
	"engine":null
};