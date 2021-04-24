#include "view.h"
#include <algorithm>
namespace mgnr{

view::view(){
    selectByBox  = false;
    selectingByBox  = false;
    windowWidth  = 1024;
    windowHeight = 530;
    SDL_Init( SDL_INIT_VIDEO );
    screen = SDL_SetVideoMode( windowWidth, windowHeight, 0,SDL_HWSURFACE | SDL_DOUBLEBUF );
    
    TTF_Init();
    font = TTF_OpenFont("sans-serif", 20);
    
    SDL_Color textColor = {255, 128, 128};
    scroll = NULL;
    clearAllMsg = TTF_RenderText_Solid(font,"清选",textColor);
    removeMsg = TTF_RenderText_Solid(font,"删除",textColor);
    showAllMsg = TTF_RenderText_Solid(font,"显示",textColor);
    hideModeMsg = TTF_RenderText_Solid(font,"隐藏",textColor);
    maticMsg = TTF_RenderText_Solid(font,"吸附",textColor);
    selAllMsg = TTF_RenderText_Solid(font,"全选",textColor);
    playMsg = TTF_RenderText_Solid(font,"播放",textColor);
    stopMsg = TTF_RenderText_Solid(font,"暂停",textColor);
    startMsg = TTF_RenderText_Solid(font,"开头",textColor);
    selectByBoxMsg = TTF_RenderText_Solid(font,"框选",textColor);
    selectByBoxOffMsg = TTF_RenderText_Solid(font,"输入",textColor);
    undoMsg = TTF_RenderText_Solid(font,"撤销",textColor);
    
    noteSurfaces[0] = TTF_RenderText_Solid(font,"[1/32]",textColor);
    noteSurfaces[1] = TTF_RenderText_Solid(font,"[1/16]",textColor);
    noteSurfaces[2] = TTF_RenderText_Solid(font,"[ 1/8]",textColor);
    noteSurfaces[3] = TTF_RenderText_Solid(font,"[ 1/4]",textColor);
    noteSurfaces[4] = TTF_RenderText_Solid(font,"[ 1/2]",textColor);
    noteSurfaces[5] = TTF_RenderText_Solid(font,"[ 3/4]",textColor);
    noteSurfaces[6] = TTF_RenderText_Solid(font,"[   1]",textColor);
    
    selectBoxX = 0;
    selectBoxY = 0;
    selectBoxXend = 0;
    selectBoxYend = 0;
    
    toneMapInit();
    
    resizeMode=false;
    ScrX=3;
    noteStatus=3;
    lastTime=0;
    
    scalar_kalman_init(&fpsSmooth ,1,1,0.01,1);
    setSection(4);
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
    SDL_Rect bd;
    bd.x=fx-2;
    bd.y=fy-2;
    bd.w=rect.w+4;
    bd.h=rect.h+4;
    if(onlydisplay)
        SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, volume, volume, 30));
    else
    if(selected){
        SDL_FillRect(screen, &bd, SDL_MapRGB(screen->format, 255, 128, 192));
    }else{
        SDL_FillRect(screen, &bd, SDL_MapRGB(screen->format, 0, 0, 0));
    }
    
    if(!info.empty()){
        if(info[0]!='@'){
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
            SDL_FillRect(screen, &rect, SDL_MapRGB(
                screen->format, 
                128, 
                128, 
                192)
            );
        }
    }else{
        SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, 64+volume, 64+volume, 30));
    }
    
    if(!info.empty() && infoFilter.empty() && (selected || info[0]=='@')){
        
        SDL_Surface * msg;
        auto it=words.find(info);
        if(it==words.end()){
        
            SDL_Color textColor = {255, 255, 255};
            msg = TTF_RenderText_Solid(font,info.c_str(),textColor);
            words[info]=msg;
            
        }else{
            
            msg = it->second;
            
        }
        SDL_BlitSurface(msg, NULL, screen, &rect);
        
    }
    
}
void view::drawNote_end(){
    if(selectingByBox){
        int bx,ex,by,ey;
        if(selectBoxX<selectBoxXend){
            bx = selectBoxX;
            ex = selectBoxXend;
        }else{
            ex = selectBoxX;
            bx = selectBoxXend;
        }
        if(selectBoxY<selectBoxYend){
            by = selectBoxY;
            ey = selectBoxYend;
        }else{
            ey = selectBoxY;
            by = selectBoxYend;
        }
        if(!(bx==ex || by==ey)){
            SDL_Rect rect;
            rect.x=bx;
            rect.y=by;
            rect.w=1;
            rect.h=ey-by;
            SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, 255 , 255 , 255));
            rect.x=ex;
            SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, 255 , 255 , 255));
            
            rect.x=bx;
            rect.y=by;
            rect.h=1;
            rect.w=ex-bx;
            SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, 255 , 255 , 255));
            rect.y=ey;
            SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, 255 , 255 , 255));
            /*
            rect.x=bx;
            rect.y=by;
            rect.h=1;
            rect.w=ey-by;
            SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, 255 , 255 , 255));
            rect.x=ex;
            SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, 255 , 255 , 255));
             */
        }
    }
    if(displayBuffer.showing){
        EM_ASM_({
            if(window.noteptr==null){
                window.noteptr={};
                window.noteptr.begin=document.getElementById("note-begin");
                window.noteptr.tone=document.getElementById("note-tone");
            }
            window.noteptr.begin.innerText=$0;
            window.noteptr.tone.innerText=$1;
        }, displayBuffer.begin/TPQ , displayBuffer.tone);
    }
    SDL_Rect rect;
    rect.x=0;
    rect.y=0;
    rect.w=windowWidth;
    rect.h=40;
    SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, 20 , 20 , 64));
    
    rect.y=8;
    SDL_BlitSurface(clearAllMsg, NULL, screen, &rect);
    
    rect.x=64;
    rect.w=128;
    SDL_Color textColor = {255, 255, 255};
    auto msg = TTF_RenderText_Solid(font,defaultInfo.c_str(),textColor);
    SDL_BlitSurface(msg, NULL, screen, &rect);
    SDL_FreeSurface(msg);
    
    rect.x=192;
    SDL_BlitSurface(removeMsg, NULL, screen, &rect);
    
    rect.x=256;
    if(infoFilter.empty()){
        SDL_BlitSurface(hideModeMsg, NULL, screen, &rect);
    }else{
        SDL_BlitSurface(showAllMsg, NULL, screen, &rect);
    }
    
    rect.x=320;
    SDL_BlitSurface(maticMsg, NULL, screen, &rect);
    
    rect.x=384;
    SDL_BlitSurface(noteSurfaces[noteStatus], NULL, screen, &rect);
    
    rect.x=448;
    char buf[64];
    snprintf(buf,64,"响度%d",defaultVolume);
    msg = TTF_RenderText_Solid(font,buf,textColor);
    SDL_BlitSurface(msg, NULL, screen, &rect);
    SDL_FreeSurface(msg);
    
    rect.x=512;
    SDL_BlitSurface(selAllMsg, NULL, screen, &rect);
    
    rect.x=576;
    if(playingStatus)
        SDL_BlitSurface(stopMsg, NULL, screen, &rect);
    else
        SDL_BlitSurface(playMsg, NULL, screen, &rect);
    
    rect.x=640;
    SDL_BlitSurface(startMsg, NULL, screen, &rect);
    
    rect.x=704;
    snprintf(buf,64,"TPQ=%d",TPQ);
    msg = TTF_RenderText_Solid(font,buf,textColor);
    SDL_BlitSurface(msg, NULL, screen, &rect);
    SDL_FreeSurface(msg);
    
    rect.x=780;
    if(selectByBox)
        SDL_BlitSurface(selectByBoxOffMsg, NULL, screen, &rect);
    else
        SDL_BlitSurface(selectByBoxMsg, NULL, screen, &rect);

    
    rect.x=844;
    SDL_BlitSurface(undoMsg, NULL, screen, &rect);

    int nowTime=EM_ASM_INT({
        return Date.now();
    });
    
    int fps=1.0/((nowTime-lastTime)/1000.0);
    lastTime=nowTime;
    rect.x=windowWidth-80;
    rect.y=windowHeight-60;
    snprintf(buf,64,"FPS %d",(int)scalar_kalman(&fpsSmooth,fps));
    msg = TTF_RenderText_Solid(font,buf,textColor);
    SDL_BlitSurface(msg, NULL, screen, &rect);
    SDL_FreeSurface(msg);
}
void view::drawTimeCol(float p){
    SDL_Rect rect;
    rect.x=p;
    rect.y=0;
    rect.w=2;
    rect.h=windowHeight;
    SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, 5, 5, 20));
}
void view::drawSectionCol(float p,int n){
    SDL_Rect rect;
    rect.x=p;
    rect.y=0;
    rect.w=3;
    rect.h=windowHeight;
    SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, 5, 5, 5));
    
    rect.y = windowHeight-90;
    rect.h = 30;
    SDL_Color textColor = {64, 128, 128};
    char buf[64];
    snprintf(buf,64,"%d",n);
    auto msg = TTF_RenderText_Solid(font,buf,textColor);
    SDL_BlitSurface(msg, NULL, screen, &rect);
    SDL_FreeSurface(msg);
}
void view::drawScroll(){
    SDL_Rect rect;
    bool u = updateTimeMax();//更新进度条长度
    if(u && scroll!=NULL){
        SDL_FreeSurface(scroll);
        scroll = NULL;
    }
    if(scroll==NULL){//更新进度条
        scroll = SDL_CreateRGBSurface(screen->flags, windowWidth, 30, 8,
            screen->format->Rmask, screen->format->Gmask,
            screen->format->Bmask, screen->format->Amask);
        rect.x=0;
        rect.y=0;
        rect.w=windowWidth;
        rect.h=30;
        SDL_FillRect(scroll, &rect, SDL_MapRGB(screen->format, 0 , 0 , 20));
        int hlen = noteToneMax-noteToneMin;//纵向的距离
        int nmax = noteToneMax;
        int nmin = noteToneMin;
        if(hlen<30){
            int s = (noteToneMax+noteToneMin)/2;
            hlen = 30;
            nmax = s+15;
            nmin = s-15;
        }
        //画音符
        rect.h=1;
        for(auto it:notes){
            if(it->tone > nmin && it->tone < nmax){
                rect.y = 30 - ((it->tone - nmin)*30)/hlen;
                rect.w = (it->delay*windowWidth)/noteTimeMax;
                if(rect.w<=0)
                    rect.w = 1;
                rect.x = (it->begin*windowWidth)/noteTimeMax;
                SDL_FillRect(scroll, &rect, SDL_MapRGB(screen->format, 128 , 128 , 128));
            }
        }
    }
    rect.x=0;
    rect.y=windowHeight-30;
    rect.w=windowWidth;
    rect.h=30;
    SDL_BlitSurface(scroll , NULL , screen, &rect);
    
    int m = noteTimeMax;
    if(m>0){
        rect.x = (lookAtX*windowWidth)/m;
        rect.w = 1;
        SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, 128 , 255 , 128));
    }
}
void view::drawTempo(float p,double t){
    SDL_Rect rect;
    rect.x=p;
    rect.y=0;
    rect.w=1;
    rect.h=windowHeight-30;
    SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, 128, 128, 128));
    
    rect.y = windowHeight-60;
    rect.h = 30;
    SDL_Color textColor = {64, 128, 128};
    char buf[64];
    snprintf(buf,64,"BPM=%d",(int)round(t));
    auto msg = TTF_RenderText_Solid(font,buf,textColor);
    SDL_BlitSurface(msg, NULL, screen, &rect);
    SDL_FreeSurface(msg);
}
void view::drawTempoPadd(){
    SDL_Rect rect;
    rect.x=0;
    rect.y=windowHeight-60;
    rect.w=windowWidth;
    rect.h=30;
    SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, 0 , 0 , 30));
}
void view::toneMapInit(){
    SDL_Color textColor = {255, 255, 255};
    static const char * pianoKeyName[]={"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
    for(int i=0;i<128;i++){
        int k = i%12;
        char buf[32];
        if(k==0)
            snprintf(buf,32,"%s\t%d",pianoKeyName[k],(i/12));
        else
            snprintf(buf,32,"%s",pianoKeyName[k]);
        toneMap[i] = TTF_RenderText_Solid(font,buf,textColor);
    }
}
void view::drawTableRaw(int from,int to,int t){
    SDL_Rect rect;
    rect.x=0;
    rect.y=from;
    rect.w=windowWidth;
    rect.h=to-from;
    
    int k = t%12;
    
    static const int    pianoKey    []={ 1 , 0  , 1 , 0  , 1 , 1 , 0  , 1 , 0  , 1 , 0  , 1};
    
    static const int    pianoColorR []={ 20,10  , 20, 10 , 20, 10, 10 , 20, 10 , 20, 10 , 10};
    static const int    pianoColorG []={ 20,10  , 20, 10 , 20, 20, 10 , 20, 10 , 20, 10 , 20};
    static const int    pianoColorB []={ 30,20  , 30, 20 , 30, 30, 20 , 30, 20 , 30, 20 , 30};
    
    SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, pianoColorR[k], pianoColorG[k], pianoColorB[k]));
    
    if(t>=0 && t<128){
        
        rect.w=30;
        if(pianoKey[k]==1)
            SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, 190, 190, 170));
        else
            SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, 0, 0, 0));
        
        SDL_BlitSurface(toneMap[t], NULL, screen, &rect);
    }
    
}
void view::hideMode(){
    if(infoFilter.empty()){
        infoFilter=defaultInfo;
    }else{
        infoFilter.clear();
    }
}
void view::pollEvent(){
    SDL_Event event;
    while (SDL_PollEvent(&event)){
        if (event.type == SDL_MOUSEBUTTONDOWN){//按键
            if(resizeMode)
                return;
            if(event.motion.y<40 && SDL_BUTTON_LEFT==event.button.button){//小于40是菜单
                if(event.motion.x<64){
                    clearSelected();
                }else
                if(event.motion.x<192){
                    char *str = (char*)EM_ASM_INT({
                        var jsString = prompt("命名",UTF8ToString($0));
                        if(!jsString)
                            return 0;
                        if(jsString[0]=="@" && jsString[1]=="T")
                            alert("不建议使用此方法设置速度，请直接点击编辑框下方的速度条设置");
                        var lengthBytes = lengthBytesUTF8(jsString)+1;
                        var stringOnWasmHeap = _malloc(lengthBytes);
                        stringToUTF8(jsString, stringOnWasmHeap, lengthBytes);
                        return stringOnWasmHeap;
                    },defaultInfo.c_str());
                    if(str!=NULL){
                        char *p=str;
                        while(*p){
                            if(*p==' ' || *p=='\r' || *p=='\n'){
                                *p='\0';
                                break;
                            }
                            ++p;
                        }
                        if(strlen(str)>0){
                            defaultInfo = str;
                            if(!infoFilter.empty()){
                                infoFilter=str;
                            }
                        }
                        renameSelected(str);
                        free(str);
                    }
                }else
                if(event.motion.x<256){
                    removeSelected();
                }else
                if(event.motion.x<320){
                    hideMode();
                }else
                if(event.motion.x<384){
                    if(automaticX)
                        automaticX=false;
                    else
                        automaticX=true;
                }else
                if(event.motion.x<448){
                    noteLengthChange();
                }else
                if(event.motion.x<512){
                    int res = EM_ASM_INT({
                        
                        var jsString = prompt("响度",$0);
                        
                        if(!jsString)
                            return $0;
                        
                        var r = parseInt(jsString);
                        
                        if(r<=0 || r>=128)
                            return $0;
                        
                        return r;
                    },defaultVolume);
                    
                    if(res>0 && res<128)
                        defaultVolume = res;
                }else
                if(event.motion.x<576){
                    selAll();
                }else
                if(event.motion.x<640){
                    if(playingStatus){
                        playStop();
                    }else{
                        playStart();
                    }
                }else
                if(event.motion.x<704){
                    lookAtX=0;
                }else
                if(event.motion.x<784){
                    lookAtX=0;
                    int t = EM_ASM_INT({
                        var jsString = prompt("TPQ",$0);
                        if(!jsString)
                            return 0;
                        return parseInt(jsString);
                    },TPQ);
                    if(t>1){
                        TPQ=t;
                        rebuildNoteLen();
                    }
                }else
                if(event.motion.x<844){
                    if(selectByBox)
                        selectByBox=false;
                    else
                        selectByBox=true;
                }else
                if(event.motion.x<904){
                    undo();
                    printf("undo\n");
                }
            }else
            if(event.motion.y>windowHeight-60){
                if(event.motion.y<windowHeight-30){
                    //速度条
                    if(SDL_BUTTON_LEFT == event.button.button){
                        clickToSetTempo(event.motion.x , event.motion.y);
                    }else if(SDL_BUTTON_RIGHT == event.button.button){
                        clickToRemoveTempo(event.motion.x , event.motion.y);
                    }
                }else{
                    //时间条
                    if(SDL_BUTTON_LEFT == event.button.button){
                        clickToLookAt(event.motion.x , event.motion.y);
                    }
                }
            }else
            if(SDL_BUTTON_LEFT == event.button.button){
                if(selectByBox){
                    selectingByBox = true;
                    selectBoxX = event.motion.x;
                    selectBoxY = event.motion.y;
                    selectBoxXend = event.motion.x;
                    selectBoxYend = event.motion.y;
                }else
                if(clickToSelect(event.motion.x , event.motion.y)<=0)
                    addDisplaied();
            }else
            if(SDL_BUTTON_RIGHT == event.button.button){
                resizeMode=true;
                resizeMode_last=event.motion.x;
            }
        }else
        if (event.type == SDL_MOUSEBUTTONUP){
            if(selectingByBox && SDL_BUTTON_LEFT == event.button.button){
                selectingByBox = false;
                {
                    int bx,ex,by,ey;
                    if(selectBoxX<selectBoxXend){
                        bx = selectBoxX;
                        ex = selectBoxXend;
                    }else{
                        ex = selectBoxX;
                        bx = selectBoxXend;
                    }
                    if(selectBoxY<selectBoxYend){
                        ey = selectBoxY;
                        by = selectBoxYend;
                    }else{
                        by = selectBoxY;
                        ey = selectBoxYend;
                    }
                    auto pb=screenToAbs(bx,by);
                    auto pe=screenToAbs(ex,ey);
                    find(pb,pe,[](note * n , void * arg){//调用HBB搜索
                        auto self = (view*)arg;
                        if(!n->selected){//未选择就加上选择
                            self->selected.insert(n);
                            n->selected=true;
                        }
                    },this);
                }
            }else
            if(SDL_BUTTON_RIGHT == event.button.button){
                resizeSelected_apply();
                resizeMode=false;
            }
        }else
        if (event.type == SDL_MOUSEMOTION){//移动鼠标
            clickToDisplay(event.motion.x , event.motion.y);
            if(selectingByBox){
                selectBoxXend = event.motion.x;
                selectBoxYend = event.motion.y;
            }
            if(resizeMode){
                resizeSelected(event.motion.x-resizeMode_last);
            }
        }else
        if (event.type == SDL_MOUSEWHEEL){
            if(event.motion.y>windowHeight-30){
                if(event.wheel.y<0){
                    noteLength/=2;
                }else
                if(event.wheel.y>0){
                    noteLength*=2;
                }
            }else if(event.motion.y>windowHeight-60){
            
            }else if(event.motion.y>40){
                if(event.wheel.y<0){
                    lookAtY-=0.7;
                }else
                if(event.wheel.y>0){
                    lookAtY+=0.7;
                }
            }else{
                
            }
        }else
        if (event.type == SDL_KEYUP){
            
        }
        if (event.type == SDL_KEYDOWN){
            if(resizeMode)
                return;
            switch(event.key.keysym.sym){
                case SDLK_UP:
                    lookAtY+=0.3;
                    
                break;
                case SDLK_DOWN:
                    lookAtY-=0.3;
                    
                break;
                case SDLK_LEFT:
                    lookAtX-=ScrX/noteLength;
                    
                break;
                case SDLK_RIGHT:
                    lookAtX+=ScrX/noteLength;
                    
                break;
                case SDLK_DELETE:
                    removeSelected();
                break;
                case SDLK_q:
                    noteLength/=2;
                break;
                case SDLK_w:
                    noteLength*=2;
                break;
            }
        }

    }
}

void view::noteLengthChange(){
    ++noteStatus;
    if(noteStatus>6)
        noteStatus=0;
    rebuildNoteLen();
}
void view::rebuildNoteLen(){
    //note length
    //1/32 1/16 1/8 1/4 1/2 1
    if(noteStatus>6 || noteStatus<0)
        noteStatus=0;
    const static float lens[]={1.0/8.0 , 1.0/4.0 , 1/2.0 , 1.0 , 2.0 , 3.0 , 4.0};
    defaultDelay=lens[noteStatus]*TPQ;
    maticBlock=lens[noteStatus]*TPQ;
    setSection();
}
void view::exportNotes(){
    std::string s;
    toString(s);
    printf("%s\n",s.c_str());
}

}
