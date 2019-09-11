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
    EMSCRIPTEN_KEEPALIVE void toStringData(){
        std::string tmpbuf;
        V.toString(tmpbuf);
        EM_ASM_({
            if(window._toStringData_callback)
                window._toStringData_callback(UTF8ToString($0));
        },tmpbuf.c_str());
    }
}
