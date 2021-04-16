var instrumentURL = "./soundfont/basic/";
var instrumentMap = new Int32Array(128);
var instrumentDownload = new Int32Array(128);
instrumentDownload[0] = 1;
window.onload = function() {
    //console.log("load soundfont");
    if (self != top) return;
    MIDI.loadPlugin({
        soundfontUrl: instrumentURL,
        instruments: ["acoustic_grand_piano"],
        onprogress: function(state, progress) {
            //console.log(state, progress);
            document.getElementById('soundfont-status-num').innerText = parseInt(progress * 100);
        },
        onsuccess: function() {
            MIDI.setVolume(0, 127);
            for(var i=0;i<16;++i){
                MIDI.channels[i].instrument = 0;
            }
            document.getElementById('soundfont-status').style.display = "none";
        }
    });
};
function downloadSoundFont(id){
    if(instrumentDownload[id]!=1){
        console.log("download:" + id);
        instrumentDownload[id] = 1;
        MIDI.loadPlugin({
            soundfontUrl: instrumentURL,
            instruments: [id],
            onprogress: function(state, progress) {
                
            },
            onsuccess: function() {
                instrumentMap[id] = id;
            }
        });
    }
}
window.mgnr = {
    "noteOn": function(info,channel, tone, v) {
        if (window.playOutput) {
            if (window.mgnr.engine == null) {
                window.mgnr.engine = document.getElementById("synth-engine");
            }
            window.mgnr.engine.contentWindow.postMessage({
                "mode": "noteOn",
                "note": tone,
                "vol": v,
                "info": info
            }, '*');
        } else {
            MIDI.noteOn(channel, tone, v, 0);
        }
    },
    "noteOff": function(info,channel, tone) {
        if (window.playOutput) {
            if (window.mgnr.engine == null) {
                window.mgnr.engine = document.getElementById("synth-engine");
            }
            window.mgnr.engine.contentWindow.postMessage({
                "mode": "noteOff",
                "note": tone,
                "info": info
            }, '*');
        } else {
            MIDI.noteOff(channel, tone, 0);
        }
    },
    "setChannelInstrument":function(c,i){
        try{
            MIDI.channels[c].instrument = window.instrumentMap[i];
        }catch(e){
            console.log(e);
        }
    },
    "requireInstrument": function(id) {
        try{
            downloadSoundFont(id);
        }catch(e){}
    },
    "loadName": function(info) {
        console.log("use:" + info);
    },
    "synth": {
        //mGenNer歌声合成协议

        //工程通过postMessage发送至目标引擎
        //由于使用的postMessage，引擎允许跨域加载

        //一个工程由多个音轨构成，每个音轨由按顺序排列的音符（词汇）或休止符构成，单位是秒

        "init": function(num) {
            //初始化，并且告诉引擎有多少个音轨
            window.mgnr.engine = document.getElementById("synth-engine");
            window.mgnr.engine.contentWindow.postMessage({
                "mode": "init",
                "channels": num
            }, '*');
        },
        "addWord": function(id, vname, tone, vol, sec) {
            //往一个音轨追加一个词（单位是秒）
            window.mgnr.engine.contentWindow.postMessage({
                "mode": "addWord",
                "name": vname,
                "tone": tone,
                "volume": vol,
                "length": sec
            }, '*');
        },
        "addPause": function(id, len) {
            //往一个音轨追加一个休止符（单位是秒）
            window.mgnr.engine.contentWindow.postMessage({
                "mode": "addPause",
                "id": id,
                "length": len
            }, '*');
        },
        "start": function() {
            //开始合成
            window.mgnr.engine.contentWindow.postMessage({
                "mode": "start"
            }, '*');
        }
    },
    "engine": null
};
window.onmessage = function(data) {
    if (data.data) {
        if (data.data.mode == "playOutput") {
            window.playOutput = data.data.val;
            console.log("set playOutput:", window.playOutput);
        }
    }
};
