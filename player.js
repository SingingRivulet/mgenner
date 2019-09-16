window.onload = function () {
	console.log("load soundfont");
	MIDI.loadPlugin({
		soundfontUrl: "./soundfont/basic/",
		instruments: [ "acoustic_grand_piano", "synth_drum" ],
		onprogress: function(state, progress) {
			console.log(state, progress);
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
	}
};