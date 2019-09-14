#include "player.h"
namespace mgnr{

player::player(){
    tempo=120;
    TPQ=120;
    playingStatus = false;
    playTimes=0;
}

void player::playStep(){
    if(playingStatus){
        playTimes++;
        
        int tp=getTempo(lookAtX);
        if(tp!=tempo){
            tempo=tp;
            printf("set tempo:%d\n",tempo);
        }
        
        //获取所有此时刻的音符
        find(lookAtX,[](note * n,void * arg){
            auto self = (player*)arg;
            self->noteOn(n);
        },this);
        
        //扫描没播放的音符
        std::list<note*> stopList;
        for(auto it:playing){
            if(it->playTimes!=playTimes){//playTimes没更新，说明音符停止
                stopList.push_back(it);
            }
        }
        for(auto it:stopList){
            noteOff(it);
        }
        
        //位置前移
        goNextStep();
    }
}
void player::playStop(){
    playingStatus = false;
}
void player::playStart(){
    for(auto it:notes){
        it->playing=false;
    }
    playing.clear();
    lastTime = getTime();
    playingStatus = true;
}

void player::noteOn(note * n){
    if(n){
        if(!n->playing){
            n->playing = true;
            playing.insert(n);
            onNoteOn(n);
        }
        n->playTimes=playTimes;
    }
}
void player::noteOff(note * n){
    if(n){
        n->playing = false;
        playing.erase(n);
        onNoteOff(n);
    }
}

float player::getTime(){
    int res=EM_ASM_INT({
        return Date.now();
    });
    return ((float)res)/1000.0;
}

void player::goNextStep(){
    float tt    = getTime();
    float delta = tt-lastTime;
    lastTime    = tt;
    float dtick = ticksPerSecond()*delta;
    lookAtX+=dtick;
}

}