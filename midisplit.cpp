#include "synth.h"
#include <map>
namespace mgnr{

void voiceTrack::pushPause(int len){
    length+=len;
    words.push_back(voiceWord(len,-1,0,std::string("")));
}
void voiceTrack::pushNote(int len,const std::string & info,int tone,int v){
    length+=len;
    words.push_back(voiceWord(len , tone , v , info));
}
void synth::clearTracks(){
    if(tracks.empty())
        return;
    for(auto it:tracks){
        delete it;
    }
    tracks.clear();
}
void synth::splitTracks(){
    clearTracks();
    
    std::multimap<int,note*> sortedNotes;//对音符进行排序
    for(auto it:notes){
        if(it->info.empty()){//没命名
            continue;
        }
        if(it->info[0]=='@'){//控制符
            continue;
        }
        int begin = it->begin;
        sortedNotes.insert(std::pair<int,note*>(begin,it));
    }
    
    for(auto it:sortedNotes){
        int begin = it.second->begin;
        int dur   = it.second->delay;
        
        if(begin<0 || dur<=0){//begin dur小于0是错误的音符
            continue;
        }
        
        if(tracks.empty()){//没有音轨
        
            //创建音轨
            auto track = new voiceTrack;
            tracks.push_back(track);
            
            if(begin==0){//begin=0，直接放到起始位置
                
            }else{//先插入休止符
                track->pushPause(begin);
            }
            track->pushNote(dur,it.second->info,it.second->tone,it.second->volume);//插入音符本身
            
        }else{
            
            bool successFlag = false;
            //有音轨，扫描音轨，找一个空穴，插入音符
            for(auto track:tracks){
                
                if(begin >  track->length){//起始时间大于音轨长度，说明需要插入先插入休止符
                    
                    track->pushPause(begin - track->length);//插入休止符
                    track->pushNote(dur,it.second->info,it.second->tone,it.second->volume);//插入音符本身
                    successFlag=true;
                    break;
                    
                }else
                if(begin == track->length){//等于,说明可以直接在后面追加
                
                    track->pushNote(dur,it.second->info,it.second->tone,it.second->volume);//插入音符本身
                    successFlag=true;
                    break;
                    
                }//否则，此音轨不可用
                
            }
            if(!successFlag){//如果没有插入成功，则新建音轨
            
                //创建音轨
                auto track = new voiceTrack;
                tracks.push_back(track);
                
                if(begin==0){//begin=0，直接放到起始位置
                    
                }else{//先插入休止符
                    track->pushPause(begin);
                }
                track->pushNote(dur,it.second->info,it.second->tone,it.second->volume);//插入音符本身
                
            }
        }
    }
    int tnum=tracks.size();
    printf("track num:%d\n",tnum);
    
}

}