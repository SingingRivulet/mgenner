#include "view.h"
#include <emscripten/bind.h>
mgnr::view V;
int main(){
    EM_ASM({
        if(window.mgnr_ready){
            var l=window.mgnr_ready.length;
            for(var i=0;i<l;i++){
                window.mgnr_ready[i]();
            }
        }
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
    });
    emscripten_set_main_loop([](){
        V.render();
        V.pollEvent();
    }, 0, 1);
    
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
    EMSCRIPTEN_KEEPALIVE void loadMidiFile(char *n){
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
            }
            ,[](const char *){
                printf("load file fail\n");
            }
        );
    }
    EMSCRIPTEN_KEEPALIVE void toStringData(){
        std::string tmpbuf;
        V.toString(tmpbuf);
        EM_ASM_({
            if(window._toStringData_callback)
                window._toStringData_callback(UTF8ToString($0));
        },tmpbuf.c_str());
    }
}
