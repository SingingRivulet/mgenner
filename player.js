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
		//mGenNer歌声合成协议
		
		//工程通过postMessage发送至目标引擎
		//由于使用的postMessage，引擎允许跨域加载
		
		//一个工程由多个音轨构成，每个音轨由按顺序排列的音符（词汇）或休止符构成，单位是秒
		
		"init":function (num) {
			//初始化，并且告诉引擎有多少个音轨
			window.mgnr.engine=document.getElementById("synth-engine");
			window.mgnr.engine.contentWindow.postMessage({
				"mode"		:"init",
				"channels"	:num
			},'*');
		},
		"addWord":function (id,vname,tone,vol,sec) {
			//往一个音轨追加一个词（单位是秒）
			window.mgnr.engine.contentWindow.postMessage({
				"mode"	:"addWord",
				"name"	:vname,
				"tone"	:tone,
				"volume"	:vol,
				"length"	:sec
			},'*');
		},
		"addPause":function (id,len) {
			//往一个音轨追加一个休止符（单位是秒）
			window.mgnr.engine.contentWindow.postMessage({
				"mode"	:"addPause",
				"id"		:id,
				"length"	:len
			},'*');
		},
		"start":function () {
			//开始合成
			window.mgnr.engine.contentWindow.postMessage({
				"mode":"start"
			},'*');
		}
	},
	"engine":null
};