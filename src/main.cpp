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
        window.toRelative=function(c){
            window._toRelative_callback=c;
            Module._toRelative();
        };
        window.toHashSerious=function(c){
            window._toHashSerious_callback=c;
            Module._toHashSerious();
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
    EMSCRIPTEN_KEEPALIVE void loadStringData(char *n){
        V.loadString(n);
    }
    EMSCRIPTEN_KEEPALIVE void exportMidiFile(char *n){
        V.exportMidi("export.mid");
    }
    static std::string compareFile;
    EMSCRIPTEN_KEEPALIVE void loadMidiFile(char * n){
        compareFile.clear();
        char * cmp = n;
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
                        toHashSerious(function(h){
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
                                                var link = div.appendChild(document.createElement('a'));
                                                link.href = "javascript:seekTick("+a+")";
                                                link.innerText = "A["+a+","+b+"]=B["+c+","+d+"]";
                                                div.appendChild(document.createElement('br'));
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
    EMSCRIPTEN_KEEPALIVE void toHashSerious(){
        std::string tmpbuf;
        V.toHashSerious(tmpbuf);
        EM_ASM_({
            if(window._toHashSerious_callback)
                window._toHashSerious_callback(UTF8ToString($0));
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
