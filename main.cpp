#include "view.h"
#include <emscripten/bind.h>
mgnr::view V;
int main(){
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
