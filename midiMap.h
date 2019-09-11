#ifndef MGNR_MIDI_MAP
#define MGNR_MIDI_MAP
#include "hbb.h"
#include "note.h"
#include <string>
#include <set>
namespace mgnr{
    class midiMap{
        public:
            midiMap();
            ~midiMap();
            note * addNote(float position,float tone,float delay,int v,const std::string & info);
            void removeNote(note * p);
            void resizeNote(note * p);
            void clear();
            
            int find(const HBB::vec & from,const HBB::vec & to,void(*callback)(note*,void*),void * arg);
            int find(float step,void(*callback)(note*,void*),void * arg);
            int find(const HBB::vec & p,void(*callback)(note*,void*),void * arg);
            //获取范围内的音符
            
            std::string infoFilter;
            std::set<note*> notes;
            
        private:
            HBB indexer;
            void * pool;
    };
}
#endif