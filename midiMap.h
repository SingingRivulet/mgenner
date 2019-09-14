#ifndef MGNR_MIDI_MAP
#define MGNR_MIDI_MAP
#include "hbb.h"
#include "note.h"
#include <string>
#include <set>
#include <map>
namespace mgnr{
    class midiMap{
        public:
            midiMap();
            ~midiMap();
            note * addNote(float position,float tone,float delay,int v,const std::string & info);
            void removeNote(note * p);
            void resizeNote(note * p);
            
            virtual void onUseInfo(const std::string & info)=0;
            
            void clear();
            
            int find(const HBB::vec & from,const HBB::vec & to,void(*callback)(note*,void*),void * arg);
            int find(float step,void(*callback)(note*,void*),void * arg);
            int find(const HBB::vec & p,void(*callback)(note*,void*),void * arg);
            //获取范围内的音符
            
            std::string infoFilter;
            std::set<note*> notes;
            std::map<noteIndex,note*> timeIndex;
            
            std::map<float,int> timeMap;
            
            int TPQ;
            
            int getTempo(float tick);
            void addTempo(float tick,int tp);
            
            void removeControl(float begin,const std::string & info);
            void addControl(float begin,const std::string & info);
            
        private:
            HBB indexer;
            void * pool;
            int id;
    };
}
#endif