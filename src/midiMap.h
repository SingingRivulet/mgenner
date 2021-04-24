#ifndef MGNR_MIDI_MAP
#define MGNR_MIDI_MAP
#include "hbb.h"
#include "note.h"
#include <string>
#include <functional>
#include <set>
#include <map>
#include <unordered_map>
#include <tuple>
#include <vector>
#include <algorithm>
#include <sstream>
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
            
            std::map<int,double> timeMap;
            
            int TPQ;
            float XShift;
            int baseTone;
            virtual void rebuildNoteLen()=0;
            
            float sectionLen;//小节线
            int section;
            
            inline void setSection(int s){
                section = s;
                setSection();
            }
            inline void setSection(){
                if(section<=0 || section>7)
                    section=4;
                sectionLen=section*TPQ;
            }
            
            double getTempo(int tick);
            bool addTempo(int tick,double tp);
            std::tuple<bool,int,double> removeTempoBeforePos(int tick);
            void getTempo(int begin,const std::function<bool(int,double)> & callback);//获取一段区域的速度
            
            void removeControl(float begin,const std::string & info);
            void addControl(float begin,const std::string & info);
            
            int getAreaNote(float begin,float len,const std::string & info,float forceLen=0.6666,float minLen=0.25);//获取一个范围内的主要音符
            int getSectionNote(float sec,const std::string & info,float forceLen=0.6666,float minLen=0.25);
            
            int getBaseTone();//获取调性
            
            inline void removeNoteById(int id){
                auto p = seekNoteById(id);
                if(p){
                    removeNote(p);
                }
            }

            inline note * seekNoteById(int id){
                auto it = noteIDs.find(id);
                if(it!=noteIDs.end()){
                    return it->second;
                }
                return NULL;
            }

            std::unordered_map<std::string,std::vector<int> > chord_map;
            std::unordered_map<std::string,int> note_number_map;
            std::map<std::string,int> chord_map_note;
            
            float noteTimeMax;
            float noteToneMax;
            float noteToneMin;
            bool noteUpdated;
            bool updateTimeMax();
        private:
            std::map<int,note*> noteIDs;
            HBB indexer;
            void * pool;
            int id;
    };
}
#endif
