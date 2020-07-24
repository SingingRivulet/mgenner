#include "synth.h"
#include <map>
#include <iostream>
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
static int hash(const char * str){
    unsigned int seed = 131; // 31 131 1313 13131 131313 etc..
    unsigned int hash = 0;
 
    while (*str){
        hash = hash * seed + (*str++);
    }
 
    return (hash & 0x7FFFFFFF);
}
void synth::toHashSeries(std::string & out){
    std::vector<std::pair<int,int> > ser;
    toHashSeries(ser);
    out.clear();
    char buf[64];
    for(auto it:ser){
        snprintf(buf,64,"%d %d\n",it.first,it.second);
        out+=buf;
    }
}
void synth::toHashSeries(std::vector<std::pair<int,int> > & out){
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
    std::string ser;
    ser.clear();
    int ntime=0;
    char buf[64];
    for(auto it:sortedNotes){
        snprintf(buf,64,"%d-%d-%d-%d ",
            (int)it.second->begin,
            (int)it.second->tone,
            (int)it.second->volume,
            (int)it.second->delay
        );
        //std::cout<<ser<<std::endl;
        if(it.first==ntime){
            ser+=buf;
        }else{
            if(!ser.empty())
                out.push_back(std::pair<int,int>(ntime,hash(ser.c_str())));
            ntime=it.first;
            ser=buf;
        }
    }
    
    if(!ser.empty()){
        out.push_back(std::pair<int,int>(ntime,hash(ser.c_str())));
    }
    
}
void synth::diff(const std::string & in,std::function<void (int)> const & callback_d,std::function<void (int,int,int,int)> const & callback_s){
    std::istringstream iss(in);
    char buf[1024];
    std::vector<std::pair<int,int> > data;
    while(!iss.eof()){
        bzero(buf,1024);
        iss.getline(buf,1024);
        if(strlen(buf)<2)
            continue;
        std::istringstream ibuf(buf);
        int t,h;
        ibuf>>t;
        ibuf>>h;
        data.push_back(std::pair<int,int>(t,h));
    }
    diff(data,callback_d,callback_s);
}
void synth::diff(const std::vector<std::pair<int,int> > & in,std::function<void (int)> const & callback_d,std::function<void (int,int,int,int)> const & callback_s){
    std::vector<std::pair<int,int> > me;
    toHashSeries(me);
    std::vector<int> A,B,Am,Bm;
    
    A.resize(me.size());
    Am.resize(me.size());
    
    B.resize(in.size());
    Bm.resize(in.size());
    
    for(int i=0;i<me.size();i++){
        A.at(i)  = me.at(i).second;
        Am.at(i) = 0;
    }
    for(int i=0;i<in.size();i++){
        B.at(i)  = in.at(i).second;
        Bm.at(i) = 0;
    }
    
    int id=0;
    LCS(A,B,[&](int im,int iin,int len){
        ++id;
        try{
            if(len>2){
                callback_s(me.at(im-len+1).first , me.at(im).first , in.at(iin-len+1).first , in.at(iin).first);
            }
            for(int i=0;i<len;i++){
                Am.at(im -i)=id;
                Bm.at(iin-i)=id;
            }
        }catch(...){
            
        }
    });
    
    int  last=0;
    bool inArea=false;
    //A,B中标0的部分就是不同的
    for(int i=0;i<me.size();i++){
        auto tick = me.at(i).first;
        //printf("id:%d\n",Am.at(i));
        if(Am.at(i) == 0){
            last = 0;
            if(!inArea){//差异区域开始
                callback_d(tick);
            }
            inArea=true;
        }else{
            if(inArea){//刚结束差异区
                
            }else{
                if(Am.at(i) != last && i!=0){
                    //发现删除过
                    callback_d(tick);
                }
            }
            last = Am.at(i);
            inArea=false;
        }
    }
}

}
