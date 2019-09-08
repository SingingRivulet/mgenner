#ifndef MGNR_VIEW
#define MGNR_VIEW
#include "editTable.h"
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_render.h>
#include <SDL/SDL_ttf.h>
#include <emscripten.h>
namespace mgnr{
    class view:public editTable{
        public:
            view();
            ~view();
            
            void drawNote_begin();
            void drawNote(int fx,int fy,int tx,int ty, int volume,const std::string & info,bool selected,bool onlydisplay=false);
            void drawNote_end();
            
            void drawTableRaw(int from,int to,int t);
            
            void pollEvent();
            
        private:
            SDL_Surface * screen;
            TTF_Font * font;
    };
}
#endif
