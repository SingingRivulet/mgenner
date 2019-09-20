#include "midiMap.h"
#include "mempool.h"
namespace mgnr{

typedef mempool<note> npool;

midiMap::midiMap(){
    pool=new npool;
}
midiMap::~midiMap(){
    clear();
    if(pool)
        delete (npool*)pool;
}
void midiMap::onUseInfo(const std::string & info){
    
}
note * midiMap::addNote(float position,float tone,float delay,int v,const std::string & info){
    onUseInfo(info);
    
    HBB::vec from;
    HBB::vec to;
    from.set(position , tone);
    to = from;
    to.X += delay;
    to.Y += 0.9;
    
    auto p  = ((npool*)pool)->get();
    p->construct();
    p->startId=++id;//0是默认值，用0可能会出问题
    p->endId=++id;//分配两个id，防止时间为0的情况
    
    p->begin    = position;
    p->tone     = tone;
    p->delay    = delay;
    p->volume   = v;
    p->info     = info;
    
    p->getBeginIndex();
    p->getEndIndex();
    timeIndex[p->beginIndex] = p;
    timeIndex[p->endIndex] = p;
    
    auto bx = indexer.add(from,to,p);
    p->indexer = bx;
    
    notes.insert(p);
    
    if(!info.empty()){
        if(info[0]=='@'){
            addControl(position,info);
        }
    }
    
    return p;
}

void midiMap::resizeNote(note * p){
    if(p){
        if(p->indexer){
            auto i = (HBB::AABB*)(p->indexer);
            i->autodrop();
        }
        
        HBB::vec from;
        HBB::vec to;
        from.set(p->begin , p->tone);
        to = from;
        to.X += p->delay;
        to.Y += 0.9;
        
        timeIndex.erase(p->endIndex);
        p->getEndIndex();//更新尾部索引
        timeIndex[p->endIndex] = p;
        
        auto bx = indexer.add(from,to,p);
        p->indexer = bx;
    }
}
void midiMap::removeNote(note * p){
    if(p){
        if(p->indexer){
            auto i = (HBB::AABB*)(p->indexer);
            i->autodrop();
        }
        if(!p->info.empty()){
            if(p->info[0]=='@'){
                removeControl(p->begin,p->info);
            }
        }
        notes.erase(p);
        timeIndex.erase(p->beginIndex);
        timeIndex.erase(p->endIndex);
        ((npool*)pool)->del(p);
    }
}

void midiMap::clear(){
    for(auto p : notes){
        if(p->indexer){
            auto i = (HBB::AABB*)(p->indexer);
            i->autodrop();
        }
        ((npool*)pool)->del(p);
        //不能直接removeNote，否则会出现野指针
    }
    notes.clear();
    timeIndex.clear();
}

int midiMap::find(const HBB::vec & from,const HBB::vec & to,void(*callback)(note*,void*),void * arg){
    HBB::AABB tmpbox;
    tmpbox.from=from;
    tmpbox.to=to;
    
    struct self{
        void(*callback)(note*,void*);
        void * arg;
        midiMap * s;
        int num;
    }s;
    s.arg=arg;
    s.callback=callback;
    s.s=this;
    s.num=0;
    
    indexer.collisionTest(&tmpbox,[](HBB::AABB * p,void * arg){
        auto s  = (self*)arg;
        auto np = (note*)(p->data);
        if(np && (s->s->infoFilter.empty() || np->info==s->s->infoFilter)){
            s->callback(np,s->arg);
            s->num++;
        }
    },&s);
    return s.num;
}

int midiMap::find(const HBB::vec & pt,void(*callback)(note*,void*),void * arg){
    
    struct self{
        void(*callback)(note*,void*);
        void * arg;
        midiMap * s;
        int num;
    }s;
    s.arg=arg;
    s.callback=callback;
    s.s=this;
    s.num=0;
    
    indexer.fetchByPoint(pt,[](HBB::AABB * p,void * arg){
        auto s  = (self*)arg;
        auto np = (note*)(p->data);
        if(np && (s->s->infoFilter.empty() || np->info==s->s->infoFilter)){
            s->callback(np,s->arg);
            s->num++;
        }
    },&s);
    return s.num;
}

int midiMap::find(float step,void(*callback)(note*,void*),void * arg){
    struct self{
        void(*callback)(note*,void*);
        void * arg;
        midiMap * s;
        int num;
    }s;
    s.arg=arg;
    s.callback=callback;
    s.s=this;
    s.num=0;
    
    indexer.fetchByStep(step,[](HBB::AABB * p,void * arg){
        auto s  = (self*)arg;
        auto np = (note*)(p->data);
        if(np && (s->s->infoFilter.empty() || np->info==s->s->infoFilter)){
            s->callback(np,s->arg);
            s->num++;
        }
    },&s);
    return s.num;
}
int midiMap::getTempo(float tick){
    if(timeMap.empty())
        return 120;
    auto it = timeMap.upper_bound(tick);//获取大于tick的第一个元素
    if(it==timeMap.end()){//指向结尾
        it--;
        return it->second;//返回最后一个元素
    }
    if(it==timeMap.begin()){//指向开头
        return it->second;
    }
    it--;//向前移动一步
    return it->second;
}
void midiMap::addTempo(float tick,int tp){
    char str[64];
    snprintf(str,64,"@T%d",tp);
    addNote(tick,-1,120,100,str);
}
void midiMap::removeControl(float begin,const std::string & info){
    if(!info.empty() && info[0]=='@' && info.size()>2){
        
        auto str=info.c_str();
        if(str[1]=='T'){
            auto r=str+2;
            
            if(strlen(r)>0){
                int tick=atoi(r);
                if(tick>0){
                    printf("remove tempo:%f %d\n",begin,tick);
                    timeMap.erase(begin);
                }
            }
        }
    }
}
void midiMap::addControl(float begin,const std::string & info){
    if(!info.empty() && info[0]=='@' && info.size()>2){
        
        auto str=info.c_str();
        if(str[1]=='T'){
            auto r=str+2;
            
            if(strlen(r)>0){
                int tick=atoi(r);
                if(tick>0){
                    printf("add tempo:%f %d\n",begin,tick);
                    timeMap[begin]=tick;
                }
            }
        }else
        if(str[1]=='S'){
            auto r=str+2;
            
            if(strlen(r)>0){
                int sec=atoi(r);
                if(sec>0 && sec<8){
                    printf("set section:%d\n",sec);
                    setSection(sec);
                }
            }
        }
    }
}

}//namespace mgnr

