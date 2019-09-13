window.onload = function () {
	console.log("load soundfont");
	MIDI.loadPlugin({
		soundfontUrl: "./soundfont/basic/",
		instrument: "acoustic_grand_piano",
		onprogress: function(state, progress) {
			console.log(state, progress);
		},
		onsuccess: function() {
			MIDI.setVolume(0, 127);
			MIDI.noteOn(0, note, velocity, delay);
			MIDI.noteOff(0, note, delay + 0.75);
			MIDI.noteOn(0, note, velocity, 3);
		}
	});
};