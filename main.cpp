#include "view.h"
mgnr::view V;
int main(){
    emscripten_set_main_loop([](){
        V.render();
        V.pollEvent();
    }, 0, 1);
}