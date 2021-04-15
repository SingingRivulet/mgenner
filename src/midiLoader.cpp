#include "editTable.h"
#include "MidiFile.h"
#include "Options.h"
#include <iostream>
#include <map>
#include <string>
namespace mgnr{

using namespace std;
using namespace smf;

void editTable::loadMidi(const std::string & str){
    MidiFile midifile;
    midifile.read(str);
    midifile.doTimeAnalysis();
    midifile.linkNotePairs();
    
    TPQ=midifile.getTicksPerQuarterNote();
    rebuildNoteLen();
    cout << "TPQ: " << TPQ << endl;
    int tracks = midifile.getTrackCount();
    
    if (tracks > 1)
        cout << "TRACKS: " << tracks << endl;
    
    std::set<std::pair<int, int> > iset;
    
    for (int track=0; track<tracks; track++) {
        //if (tracks > 1) cout << "\nTrack " << track << endl;
        
        char infoBuf[128];
        snprintf(infoBuf,sizeof(infoBuf),"track-%d",track);
        
        for (int event=0; event<midifile[track].size(); event++) {
            
            if (midifile[track][event].isNoteOn() && midifile[track][event].size()>=3){
                int position = midifile[track][event].tick;
                int delay = midifile[track][event].getTickDuration();
                int delayS = midifile[track][event].getDurationInSeconds();
                int tone = (int)midifile[track][event][1];
                int v = (int)midifile[track][event][2];
                addNote(position, tone, delay, v,infoBuf);
            }else if(midifile[track][event].isTimbre()){
                iset.insert(std::pair<int, int>(track,midifile[track][event].getP1()));
            }
        }
    }
    
    for(int i=0; i<midifile.getNumEvents(0); i++){
        if(midifile.getEvent(0,i).isTempo()){//是设置时间
            double tp = midifile.getEvent(0,i).getTempoBPM();
            addTempo(midifile.getEvent(0,i).tick,tp);
        }
    }
    
    for (auto it : iset){
        std::cout << "Track:" << it.first << "\tInstrument:" << it.second << std::endl;
    }
}

void editTable::exportMidi(const std::string & filename){
    map<string,int> tracks;
    int trackNum=1;//0音轨存没有info的音符
    int track;
    MidiFile midifile;
    
    midifile.setTPQ(TPQ);//0音轨
    midifile.addTrack();//0音轨
    
    struct noteMap_t{
        int tone,volume,time;
        bool isNoteOn;
    };
    
    std::map<int,std::vector<noteMap_t*> > noteMap;
    
    for(auto it:notes){
        if(it->info.empty()){
            track=0;
            
            //midifile.addNoteOn(track, it->begin , 0, it->tone , it->volume > 100 ? 100 : it->volume);
            //midifile.addNoteOff(track, it->begin + it->delay , 0, it->tone);
            
        }else{
            if(it->info.at(0)!='@'){//为@是控制字符
                auto tit=tracks.find(it->info);
                
                if(tit==tracks.end()){//没有音轨
                    midifile.addTrack();
                    tracks[it->info]=trackNum;
                    track=trackNum;
                    ++trackNum;
                    
                    auto p1 = new noteMap_t;
                    p1->tone   = it->tone;
                    p1->volume = it->volume > 100 ? 100 : it->volume;
                    p1->time   = it->begin;
                    p1->isNoteOn = true;
                    
                    auto p2 = new noteMap_t;
                    p2->tone   = it->tone;
                    p2->volume = 0;
                    p2->time   = it->begin + it->delay;
                    p2->isNoteOn = false;
                    
                    auto & lst = noteMap[track];
                    lst.push_back(p1);
                    lst.push_back(p2);
                    
                    //midifile.addNoteOn(track, it->begin , 0, it->tone , it->volume > 100 ? 100 : it->volume);
                    //midifile.addNoteOff(track, it->begin + it->delay , 0, it->tone);
                    
                }else{
                    track=tit->second;
                    
                    
                    auto p1 = new noteMap_t;
                    p1->tone   = it->tone;
                    p1->volume = it->volume > 100 ? 100 : it->volume;
                    p1->time   = it->begin;
                    p1->isNoteOn = true;
                    
                    auto p2 = new noteMap_t;
                    p2->tone   = it->tone;
                    p2->volume = 0;
                    p2->time   = it->begin + it->delay;
                    p2->isNoteOn = false;
                    
                    auto & lst = noteMap[track];
                    lst.push_back(p1);
                    lst.push_back(p2);
                    
                    //midifile.addNoteOn(track, it->begin , 0, it->tone , it->volume > 100 ? 100 : it->volume);
                    //midifile.addNoteOff(track, it->begin + it->delay , 0, it->tone);
                    
                }
                
            }
        }
    }
    for(auto it:timeMap){//添加time map
        midifile.addTempo(0,it.first,it.second);
    }
    for(auto itlst:noteMap){
        int tk = itlst.first;
        
        sort(itlst.second.begin(),itlst.second.end(),[](noteMap_t * a,noteMap_t * b){
            return a->time < b->time;
        });
        
        for(auto it:itlst.second){//扫描音轨
            if(it->isNoteOn){
                midifile.addNoteOn(tk, it->time , 0, it->tone , it->volume);
            }else{
                midifile.addNoteOff(tk, it->time , 0, it->tone);
            }
            delete it;
        }
    }
    midifile.write(filename);
}

}
