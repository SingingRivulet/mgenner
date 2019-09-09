#ifndef MGNR_VIEW
#define MGNR_VIEW
#include "editTable.h"
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_render.h>
#include <SDL/SDL_ttf.h>
#include <unordered_map>
#include <stdlib.h>
#include <array>
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
            SDL_Surface * clearAllMsg;
            TTF_Font * font;
            std::unordered_map<std::string,std::array<unsigned char,3> > colors;
    };
}
#endif
