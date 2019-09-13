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
		}
	});
};