#include "view.h"
namespace mgnr{

view::view(){
    windowWidth  = 640;
    windowHeight = 480;
    SDL_Init( SDL_INIT_VIDEO );
    screen = SDL_SetVideoMode( windowWidth, windowHeight, 0,SDL_HWSURFACE | SDL_DOUBLEBUF );
    
    TTF_Init();
    font = TTF_OpenFont("sans-serif", 20);
}
view::~view(){
    TTF_CloseFont(font);
    SDL_Quit();
}

void view::drawNote_begin(){
    SDL_Rect rect;
    rect.x=0;
    rect.y=0;
    rect.w=windowWidth;
    rect.h=windowHeight;
    SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, 0 , 0 , 30));
}

void view::drawNote(int fx,int fy,int tx,int ty, int volume,const std::string & info,bool selected,bool onlydisplay){
    SDL_Rect rect;
    rect.x=fx;
    rect.y=fy;
    rect.w=tx-fx;
    rect.h=ty-fy;
    if(onlydisplay)
        SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, volume, volume, 30));
    else
    if(selected){
        SDL_Rect bd;
        bd.x=fx-2;
        bd.y=fy-2;
        bd.w=rect.w+4;
        bd.h=rect.h+4;
        SDL_FillRect(screen, &bd, SDL_MapRGB(screen->format, 128, 64, 72));
    }
    
    if(!info.empty()){
        unsigned char r,g,b;
        auto it=colors.find(info);
        if(it==colors.end()){
            r=rand()%64;
            g=rand()%64;
            b=rand()%64;
            std::array<unsigned char,3> arr;
            arr[0]=r;
            arr[1]=g;
            arr[2]=b;
            colors[info]=arr;
        }else{
            r=it->second[0];
            g=it->second[1];
            b=it->second[2];
        }
        SDL_FillRect(screen, &rect, SDL_MapRGB(
            screen->format, 
            r+volume, 
            g+volume, 
            b+volume)
        );
    }else{
        SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, 64+volume, 64+volume, 30));
    }
    
    if(!info.empty() && selected){
        
        SDL_Color textColor = {255, 255, 255};
        
        auto msg = TTF_RenderText_Solid(font,info.c_str(),textColor);
        
        SDL_BlitSurface(msg, NULL, screen, &rect);
        
        SDL_FreeSurface(msg);
    }
    
}
void view::drawNote_end(){
    if(displayBuffer.showing){
        EM_ASM_({
            if(window.noteptr==null){
                window.noteptr={};
                window.noteptr.begin=document.getElementById("note-begin");
                window.noteptr.tone=document.getElementById("note-tone");
            }
            window.noteptr.begin.innerText=$0;
            window.noteptr.tone.innerText=$1;
        }, displayBuffer.begin , displayBuffer.tone);
    }
    SDL_Rect rect;
    rect.x=0;
    rect.y=0;
    rect.w=windowWidth;
    rect.h=40;
    SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, 20 , 20 , 64));
    
    rect.x=8;
    rect.y=8;
    rect.w=128;
    SDL_Color textColor = {255, 255, 255};
    auto msg = TTF_RenderText_Solid(font,defaultInfo.c_str(),textColor);
    SDL_BlitSurface(msg, NULL, screen, &rect);
    SDL_FreeSurface(msg);
}
void view::drawTableRaw(int from,int to,int t){
    SDL_Rect rect;
    rect.x=0;
    rect.y=from;
    rect.w=windowWidth;
    rect.h=to-from;
    if(t%2)
        SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, 20, 20, 30));
    else
        SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, 10, 10, 20));
    
    static const int    pianoKey    []={ 1 , 0  , 1 , 0  , 1 , 1 , 0  , 1 , 0  , 1 , 0  , 1};
    static const char * pianoKeyName[]={"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
    
    if(t>=0 && t<128){
        int k = t%12;
        rect.w=30;
        if(pianoKey[k]==1)
            SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, 190, 190, 170));
        else
            SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, 0, 0, 0));
        
        SDL_Color textColor = {255, 255, 255};
        
        char buf[32];
        if(k==0)
            snprintf(buf,32,"%s\t%d",pianoKeyName[k],(int)(t/12)+1);
        else
            snprintf(buf,32,"%s",pianoKeyName[k]);
        
        auto msg = TTF_RenderText_Solid(font,buf,textColor);
        
        SDL_BlitSurface(msg, NULL, screen, &rect);
        
        SDL_FreeSurface(msg);
    }
    
}
void view::pollEvent(){
    SDL_Event event;
    if (SDL_PollEvent(&event)){
        if (event.type == SDL_MOUSEBUTTONDOWN){//按键
            if(event.motion.y<40){//小于40是菜单
                if(event.motion.x<128){
                    char *str = (char*)EM_ASM_INT({
                        var jsString = prompt("命名");
                        var lengthBytes = lengthBytesUTF8(jsString)+1;
                        var stringOnWasmHeap = _malloc(lengthBytes);
                        stringToUTF8(jsString, stringOnWasmHeap, lengthBytes);
                        return stringOnWasmHeap;
                    });
                    defaultInfo = str;
                    free(str);
                }
            }else
            if(SDL_BUTTON_LEFT == event.button.button){
                addDisplaied();
            }else
            if(SDL_BUTTON_RIGHT == event.button.button){
                clickToSelect(event.motion.x , event.motion.y);
            }
        }else
        if (event.type == SDL_MOUSEMOTION){//移动鼠标
            clickToDisplay(event.motion.x , event.motion.y);
        }else
        if (event.type == SDL_MOUSEWHEEL){
            if(event.wheel.y<0){
                lookAtY-=0.7;
            }else
            if(event.wheel.y>0){
                lookAtY+=0.7;
            }
        }else
        if (event.type == SDL_KEYDOWN){
            switch(event.key.keysym.sym){
                case SDLK_UP:
                    lookAtY+=0.3;
                    
                break;
                case SDLK_DOWN:
                    lookAtY-=0.3;
                    
                break;
                case SDLK_LEFT:
                    lookAtX-=noteLength*3;
                    
                break;
                case SDLK_RIGHT:
                    lookAtX+=noteLength*3;
                    
                break;
                case SDLK_DELETE:
                    removeSelected();
                break;
            }
        }

    }
}

}