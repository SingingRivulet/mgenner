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
        
        double tp=getTempo(lookAtX);
        if(round(tp-tempo)!=0){
            tempo=tp;
            printf("set tempo:%f\n",tempo);
        }
        
        toPlay.clear();
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
        for(auto it:toPlay){
            int ins = useInstrument(it->info);
            if(ins>=0){
                onNoteOn(it,ins);
            }else{
                printf("fail to play note:%s(%d)\n",it->info.c_str(),(int)it->tone);
            }
        }
        toPlay.clear();
        
        //位置前移
        goNextStep();
    }
}
void player::playStop(){
    std::list<note*> stopList;
    for(auto it:playing){
        stopList.push_back(it);
    }
    for(auto it:stopList){
        noteOff(it);
    }
    playing.clear();
    playingStatus = false;
}
void player::playStart(){
    for(auto it:notes){
        it->playing=false;
    }
    playing.clear();
    lastTime = getTime();
    playingStatus = true;
    for(int i=0;i<16;++i){
        playNum[i] = 0;
        playIns[i] = -1;
    }
    for(int i=0;i<128;++i){
        ins2Channel[i] = -1;
    }
}
int player::useInstrument(const std::string & n){
    int id = getInstrumentId(n);
    if(ins2Channel[id]==-1){
        for(int i=0;i<16;++i){
            if(playIns[i]==-1){
                ins2Channel[id] = i;
                playIns[i] = id;
                playNum[i] = 1;
                onSetChannelIns(i,id);
                return i;
            }
        }
        return -1;
    }else{
        ++playNum[ins2Channel[id]];
        return ins2Channel[id];
    }
}
int player::releaseInstrument(const std::string & n){
    int id = getInstrumentId(n);
    if(ins2Channel[id]==-1){
        return -1;
    }else{
        int c = ins2Channel[id];
        --playNum[c];
        if(playNum[c]<=0){
            playNum[c] = 0;
            playIns[c] = -1;
            ins2Channel[id] = -1;
        }
        return c;
    }
    return -1;
}

void player::noteOn(note * n){
    if(n){
        if(!n->playing){
            n->playing = true;
            playing.insert(n);
            //onNoteOn(n);
            toPlay.insert(n);
        }
        n->playTimes=playTimes;
    }
}
void player::noteOff(note * n){
    if(n){
        n->playing = false;
        playing.erase(n);
        int ins = releaseInstrument(n->info);
        if(ins>=0){
            onNoteOff(n,ins);
        }else{
            printf("fail to release note:%s(%d)\n",n->info.c_str(),(int)n->tone);
        }
    }
}

int player::getTime(){
    return EM_ASM_INT({
        return Date.now();
    });
}

void player::goNextStep(){
    int tt    = getTime();
    int delta = tt-lastTime;
    lastTime    = tt;
    float dtick = ticksPerSecond()*(delta/1000.0);
    lookAtX+=dtick;
}

}
