#include "synth.h"
namespace mgnr{

void synth::synthOutput(){
    splitTracks();
    //结合timeMap，计算出秒数，输出到合成引擎
    synthOutput_init(tracks.size());
    int id=0;
    for(auto it:tracks){
        synthOutput_addIntoChannel(id,*it);
        ++id;
    }
    synthOutput_start();
}
void synth::synthOutput_addIntoChannel(int channel,const voiceTrack & track){
    int nowTempo=120;//当前速度
    int nextTime=-1; //下一次改变速度的时间
    int nowTime =0;  //当前时间
    
    auto tmit=timeMap.begin();
    if(tmit!=timeMap.end()){//有时间表
        
        nowTempo=tmit->second;
        
        tmit++;//下一次的时间
        if(tmit!=timeMap.end()){
            nextTime=tmit->first;
        }
        
    }
    
    for(auto word : track.words){
        if(nextTime!=-1 && nowTime+word.length > nextTime){
            //分多次
            int levTm=word.length;
            float sumSeconds=0;
            
            while(1){
                if(nowTime>=nextTime){
                    //现在时间>=下一步时间
                    //重设nextTime，然后继续
                    
                    if(tmit!=timeMap.end()){//时间表可用
                        nowTempo=tmit->second;
                    }else{
                        //（未预料的异常情况）
                        nextTime=-1;
                        sumSeconds += (60.0 / (nowTempo * TPQ)) * levTm;
                        nowTime    += levTm;
                        break;
                    }
                    tmit++;//获取下一次的时间
                    if(tmit!=timeMap.end()){//存在下一次
                        nextTime=tmit->first;//更新下一次
                    }else{
                        nextTime=-1;
                        sumSeconds += (60.0 / (nowTempo * TPQ)) * levTm;
                        nowTime    += levTm;
                        break;
                    }
                    
                }else{//if(nowTime<nextTime)
                    //现在时间<下一步时间
                    if(nowTime+levTm > nextTime){//现在时间大于剩余时间
                        int psdTicks = nextTime-nowTime;//被处理的时间
                        nowTime = nextTime;
                        levTm  -= psdTicks;
                        sumSeconds += (60.0 / (nowTempo * TPQ)) * psdTicks;
                    }else{
                        sumSeconds += (60.0 / (nowTempo * TPQ)) * levTm;
                        nowTime    += levTm;
                        break;
                    }
                }
            }
            //sumSeconds
            if(word.tone==-1)
                synthOutput_addPause(channel , sumSeconds);
            else
                synthOutput_addWord(channel , word.word , word.tone , word.volume , sumSeconds);
        }else{
            //能一次处理完
            float tm = (60.0 / (nowTempo * TPQ)) * word.length;
            nowTime+=word.length;
            if(word.tone==-1)
                synthOutput_addPause(channel , tm);
            else
                synthOutput_addWord(channel , word.word , word.tone , word.volume , tm);
        }
    }
}
void synth::synthOutput_init(int channelNum){
    EM_ASM({
        mgnr.synth.init($0);
    },channelNum);
}
void synth::synthOutput_addWord(int channel,const std::string & word,int tone,int volume,float sec){
    EM_ASM({
        mgnr.synth.addWord($0,UTF8ToString($1),$2,$3,$4);
    },channel,word.c_str(),tone,volume,sec);
}
void synth::synthOutput_addPause(int channel,float sec){
    EM_ASM({
        mgnr.synth.addPause($0,$1);
    },channel,sec);
}
void synth::synthOutput_start(){
    EM_ASM({
        mgnr.synth.start();
    });
}

}
