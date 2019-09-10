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
note * midiMap::addNote(float position,float tone,float delay,int v,const std::string & info){
    HBB::vec from;
    HBB::vec to;
    from.set(position , tone);
    to = from;
    to.X += delay;
    to.Y += 0.9;
    
    auto p  = ((npool*)pool)->get();
    p->construct();
    
    p->begin    = position;
    p->tone     = tone;
    p->delay    = delay;
    p->volume   = v;
    p->info     = info;
    
    auto bx = indexer.add(from,to,p);
    p->indexer = bx;
    
    notes.insert(p);
    
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
        notes.erase(p);
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

}//namespace mgnr

