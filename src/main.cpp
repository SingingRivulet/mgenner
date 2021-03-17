#include "view.h"
#include <emscripten/bind.h>
mgnr::view V;
bool inIframe=false;
int main(){
    inIframe=(EM_ASM_INT({
        if (self != top)
            return 1;
        else
            return 0;
    })==1);
    EM_ASM({
        window.note_name = Array("C","C#","D","D#","E","F","F#","G","G#","A","A#","B");
        window.note_number = Array("1","1#","2","2#","3","4","4#","5","5#","6","6#","7");
        window.downloadString=function(s,name){
            var blob = new Blob([s], {type: "application/octet-binary"});;
            var url = window.URL.createObjectURL(blob); 
            console.log(url);
            var link = document.createElement('a'); 
            link.style.display = 'none'; 
            link.href = url; 
            link.setAttribute('download', name); 
            document.body.appendChild(link); 
            link.click();
        };
        window.loadStringData=function(s){
            var ptr = allocate(intArrayFromString(s), 'i8', ALLOC_NORMAL);
            var retPtr = Module._loadStringData(ptr);
            _free(ptr);
        };
        window.loadMidiFile=function(s){
            var ptr = allocate(intArrayFromString(s), 'i8', ALLOC_NORMAL);
            var retPtr = Module._loadMidiFile(ptr);
            _free(ptr);
        };
        window.exportMidiFile=function(){
            Module._exportMidiFile();
            var data=FS.readFile("export.mid");
            var blob = new Blob([data.buffer], {type: "application/octet-binary"});
            return blob;
        };
        window.toStringData=function(c){
            window._toStringData_callback=c;
            Module._toStringData();
        };
        window.toThemesTrain=function(c,dt){
            window._toThemesTrain_callback=c;
            Module._toThemesTrain(parseInt(dt));
        };
        window.toThemesPredict=function(c,dt){
            window._toThemesPredict_callback=c;
            Module._toThemesPredict(parseInt(dt));
        };
        window.downloadThemesTrain=function(){
            var s = prompt("block length");
            var i = parseInt(s);
            if(i<=0){
                i=1;
            }
            toThemesTrain(function(s){
                downloadString(s,"outTunner");
            },i);
        };
        window.downloadThemesPredict=function(){
            var s = prompt("block length");
            var i = parseInt(s);
            if(i<=0){
                i=1;
            }
            toThemesPredict(function(s){
                downloadString(s,"outTunner");
            },i);
        };
        window.toRelative=function(c){
            window._toRelative_callback=c;
            Module._toRelative();
        };
        window.toHashSeries=function(c){
            window._toHashSeries_callback=c;
            Module._toHashSeries();
        };
        window.synthOutput=function(c){
            Module._synthOutput();
        };
        window.midiDiff=function(s,c,m){
            window._midiDiff_callback_diff=c;
            window._midiDiff_callback_same=m;
            var lengthBytes = lengthBytesUTF8(s)+1;
            var stringOnWasmHeap = _malloc(lengthBytes);
            stringToUTF8(s, stringOnWasmHeap, lengthBytes);
            Module._midiDiff(stringOnWasmHeap);
            _free(stringOnWasmHeap);
        };
        window.getAreaNote=function(b,d,s){
            s = s|"";
            var lengthBytes = lengthBytesUTF8(s)+1;
            var stringOnWasmHeap = _malloc(lengthBytes);
            stringToUTF8(s, stringOnWasmHeap, lengthBytes);
            var res = Module._getAreaNote(b,d,stringOnWasmHeap);
            _free(stringOnWasmHeap);
            return res;
        };
        window.getSectionNote=function(b,s){
            s = s|"";
            var lengthBytes = lengthBytesUTF8(s)+1;
            var stringOnWasmHeap = _malloc(lengthBytes);
            stringToUTF8(s, stringOnWasmHeap, lengthBytes);
            var res = Module._getSectionNote(b,stringOnWasmHeap);
            _free(stringOnWasmHeap);
            return res;
        };
        window.getMainNote=function(from,num,callback,s){
            s = s|"";
            var lengthBytes = lengthBytesUTF8(s)+1;
            var stringOnWasmHeap = _malloc(lengthBytes);
            stringToUTF8(s, stringOnWasmHeap, lengthBytes);
            for(var i=0;i<num;++i){
                callback(Module._getSectionNote(i+from,stringOnWasmHeap));
            }
            _free(stringOnWasmHeap);
        };
        window.getMainNoteName=function(from,num,shift,s){
            var res = [];
            getMainNote(from,num,function(note){
                if(note!=-1){
                    var n = note+shift;
                    if(n<0)
                        n+=24;
                    res.push(note_number[(n+Module._getBaseToneCache())%12]);
                }else{
                    res.push("None");
                }
            },s);
            return res;
        };
        window.addChord=function(p , root , name , format , length , root_base , v , info){
            info = info || 'default';
            var lroot   = lengthBytesUTF8(root)+1;
            var lname   = lengthBytesUTF8(name)+1;
            var lformat = lengthBytesUTF8(format)+1;
            var linfo   = lengthBytesUTF8(info)+1;
            var proot   = _malloc(lroot);
            var pname   = _malloc(lname);
            var pformat = _malloc(lformat);
            var pinfo   = _malloc(linfo);
            stringToUTF8(root, proot, lroot);
            stringToUTF8(name, pname, lname);
            stringToUTF8(format, pformat, lformat);
            stringToUTF8(info, pinfo, linfo);
            Module._addChord(p,proot,pname,pformat,length,root_base,v,pinfo);
            _free(proot);
            _free(pname);
            _free(pformat);
            _free(pinfo);
        };
        window.seekTick=function(t){
            Module._seek(t);
        };
        if(window.mgnr_ready){
            var l=window.mgnr_ready.length;
            for(var i=0;i<l;i++){
                window.mgnr_ready[i]();
            }
        }
    });
    if(inIframe){
        
    }else{
        emscripten_set_main_loop([](){
            V.render();
            V.playStep();
            V.pollEvent();
        }, 0, 1);
    }
}

void setDefaultDelay(float d){
    V.defaultDelay=d;
}

void setDefaultVolume(int v){
    V.defaultVolume=v;
}

EMSCRIPTEN_BINDINGS(my_module) {
   emscripten::function("setDefaultDelay", &setDefaultDelay);
   emscripten::function("setDefaultVolume", &setDefaultVolume);
}

extern "C"{
    EMSCRIPTEN_KEEPALIVE void selectByArea(int x,int y,int len){
        V.selectByArea(x,y,len);
    }
    EMSCRIPTEN_KEEPALIVE void selectByArea_unique(int x,int y,int len){
        V.selectByArea_unique(x,y,len);
    }
    EMSCRIPTEN_KEEPALIVE void addNote(int x,int y,int d ,int v){
        V.addNote(x,y,d,v,V.defaultInfo);
    }
    EMSCRIPTEN_KEEPALIVE void clearSelected(){
        V.clearSelected();
    }
    EMSCRIPTEN_KEEPALIVE void loadStringData(char *n){
        V.loadString(n);
    }
    EMSCRIPTEN_KEEPALIVE void addChord(float p , char * root , char * name , char * format , float length , int root_base,int v,char * info){
        V.addChord(p,root,name,format,length,root_base,v,info);
    }
    EMSCRIPTEN_KEEPALIVE int getAreaNote(float b,float d,char * s){
        return V.getAreaNote(b,d,s);
    }
    EMSCRIPTEN_KEEPALIVE int getSectionNote(float b,char * s){
        return V.getSectionNote(b,s);
    }
    EMSCRIPTEN_KEEPALIVE void exportMidiFile(char *n){
        V.exportMidi("export.mid");
    }
    EMSCRIPTEN_KEEPALIVE int getBaseToneCache(){
        return V.baseTone;
    }
    static std::string compareFile;
    EMSCRIPTEN_KEEPALIVE void loadMidiFile(char * n){
        compareFile.clear();
        char * cmp=NULL;
        
        if(n[0]=='h' && n[1]=='t' && n[2]=='t' && n[3]=='p')
            goto start;
        if(n[0]=='b' && n[1]=='l' && n[2]=='o' && n[3]=='b')
            goto start;
        
        cmp = n;
        while(1){
            if(*cmp=='\0')
                break;
            else
            if(*cmp==':'){
                *cmp='\0';
                ++cmp;
                if(*cmp!='\0'){
                    compareFile=cmp;
                }
            }
                
            ++cmp;
        }
        
        printf("load:%s\n",n);
        if(!compareFile.empty()){
            printf("compare:%s\n",compareFile.c_str());
        }
        
        start:
        
        emscripten_async_wget(n,"tmp.mid",
            [](const char *){
                printf("load file success\n");
                
                char tbuf[4096];
                FILE * fp = fopen("tmp.mid","r");
                if(fp==NULL)
                    return;
                bzero(tbuf,sizeof(tbuf));
                fgets(tbuf,sizeof(tbuf),fp);
                if(tbuf[0]=='M' && tbuf[1]=='G' && tbuf[2]=='N' && tbuf[3]=='R'){//是自己的专用格式
                    
                    while(!feof(fp)){
                        bzero(tbuf,sizeof(tbuf));
                        fgets(tbuf,sizeof(tbuf),fp);
                        if(strlen(tbuf)>0)
                            V.loadString(tbuf);
                    }
                    
                }else{//是midi格式
                
                    V.loadMidi("tmp.mid");
                
                }
                if(inIframe){
                    EM_ASM({//向父窗口发送hash
                        toHashSeries(function(h){
                            window.parent.postMessage({
                                'mode':'diff',
                                'hash':h
                            },'/');
                        });
                    });
                }else{
                    if(!compareFile.empty()){
                        EM_ASM({
                            var filename=UTF8ToString($0);
                            var ifm=document.body.appendChild(document.createElement('iframe'));
                            ifm.height=2;
                            ifm.width=2;
                            window.addEventListener('message',function(e) {
                                var d = e.data;
                                //console.log(d);
                                if(d.mode=='diff'){
                                    document.body.removeChild(ifm);
                                    var bx = document.getElementById("controls");
                                    if(bx){
                                        var div = bx.appendChild(document.createElement('div'));
                                        div.style['overflow']="scroll";
                                        var info = div.appendChild(document.createElement('span'));
                                        info.innerHTML="midi对比：<br>";
                                    
                                        var used=false;
                                        midiDiff(d.hash,
                                            function(t){
                                                used=true;
                                                var link = div.appendChild(document.createElement('a'));
                                                link.href = "javascript:seekTick("+t+")";
                                                link.innerText = "差异："+t;
                                                div.appendChild(document.createElement('br'));
                                            },
                                            function(a,b,c,d){
                                                var span = div.appendChild(document.createElement('span'));
                                                span.innerHTML = 
                                                    "A[<a href='javascript:seekTick("+a+")'>"+a+
                                                    "</a>,<a href='javascript:seekTick("+b+")'>"+b+"</a>]=B["+c+","+d+"]<br>";
                                            }
                                        );
                                        if(!used){
                                            var nothing = div.appendChild(document.createElement('span'));
                                            nothing.innerHTML="暂无差异<br>";
                                        }
                                    }
                                }
                                
                            });
                            var url=location.origin+location.pathname+"#"+filename;
                            //console.log(url);
                            ifm.src=url;
                        },compareFile.c_str());
                    }
                }
            }
            ,[](const char *){
                printf("load file fail\n");
            }
        );
    }
    EMSCRIPTEN_KEEPALIVE void toThemesTrain(int delta){
        std::string tmpbuf;
        V.toThemesTrain(tmpbuf,delta);
        EM_ASM_({
            if(window._toThemesTrain_callback)
                window._toThemesTrain_callback(UTF8ToString($0));
        },tmpbuf.c_str());
    }
    EMSCRIPTEN_KEEPALIVE void toThemesPredict(int delta){
        std::string tmpbuf;
        V.toThemesPredict(tmpbuf,delta);
        EM_ASM_({
            if(window._toThemesPredict_callback)
                window._toThemesPredict_callback(UTF8ToString($0));
        },tmpbuf.c_str());
    }
    EMSCRIPTEN_KEEPALIVE void toHashSeries(){
        std::string tmpbuf;
        V.toHashSeries(tmpbuf);
        EM_ASM_({
            if(window._toHashSeries_callback)
                window._toHashSeries_callback(UTF8ToString($0));
        },tmpbuf.c_str());
    }
    EMSCRIPTEN_KEEPALIVE void toRelative(){
        std::string tmpbuf;
        V.selectedToRelative(tmpbuf);
        EM_ASM_({
            if(window._toRelative_callback)
                window._toRelative_callback(UTF8ToString($0));
        },tmpbuf.c_str());
    }
    EMSCRIPTEN_KEEPALIVE void seek(int posi){
        V.lookAtX=posi;
    }
    EMSCRIPTEN_KEEPALIVE void toStringData(){
        std::string tmpbuf;
        V.toString(tmpbuf);
        EM_ASM_({
            if(window._toStringData_callback)
                window._toStringData_callback(UTF8ToString($0));
        },tmpbuf.c_str());
    }
    EMSCRIPTEN_KEEPALIVE void synthOutput(){
        V.synthOutput();
    }
    EMSCRIPTEN_KEEPALIVE int getBaseTone(){
        return V.getBaseTone();
    }
    EMSCRIPTEN_KEEPALIVE void midiDiff(const char * s){
        V.diff(s,
            [&](int posi){
                EM_ASM_({
                    if(window._midiDiff_callback_diff)
                        window._midiDiff_callback_diff($0);
                },posi);
            },
            [&](int as,int ae,int bs,int be){
                EM_ASM_({
                    if(window._midiDiff_callback_same)
                        window._midiDiff_callback_same($0,$1,$2,$3);
                },as,ae,bs,be);
            }
        );
    }
}
