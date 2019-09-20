#ifndef MGNR_VIEW
#define MGNR_VIEW
#include "synth.h"
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_render.h>
#include <SDL/SDL_ttf.h>
#include <unordered_map>
#include <stdlib.h>
#include <array>
namespace mgnr{
    class view:public synth{
        public:
            view();
            ~view();
            
            void drawNote_begin();
            void drawNote(int fx,int fy,int tx,int ty, int volume,const std::string & info,bool selected,bool onlydisplay=false);
            void drawNote_end();
            
            void drawTableRaw(int from,int to,int t);
            void drawTimeCol(float p);
            void drawSectionCol(float p);
            
            void pollEvent();
            
            void hideMode();
            
        private:
            
            SDL_Surface * screen;
            SDL_Surface * clearAllMsg;
            SDL_Surface * removeMsg;
            SDL_Surface * showAllMsg;
            SDL_Surface * hideModeMsg;
            SDL_Surface * maticMsg;
            SDL_Surface * selAllMsg;
            SDL_Surface * playMsg;
            SDL_Surface * stopMsg;
            SDL_Surface * startMsg;
            
            SDL_Surface * toneMap[128];
            
            TTF_Font * font;
            std::unordered_map<std::string,std::array<unsigned char,3> > colors;
            std::unordered_map<std::string,SDL_Surface * > words;
            
            bool resizeMode;
            int resizeMode_last;
            float ScrX;
            
            int lastTime;
            
            //note length
            //1/32 1/16 1/8 1/4 1/2 1
            //15   30   60  120 240 480
            SDL_Surface * noteSurfaces[6];
            int noteStatus;
            void noteLengthChange();
            void toneMapInit();
            void exportNotes();
    };
}
#endif
