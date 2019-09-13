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
    cout << "TPQ: " << TPQ << endl;
    int tracks = midifile.getTrackCount();
    
    if (tracks > 1)
        cout << "TRACKS: " << tracks << endl;
    
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
            }
        }
    }

}

void editTable::exportMidi(const std::string & filename){
    map<string,int> tracks;
    int trackNum=1;//0音轨存没有info的音符
    int track;
    MidiFile midifile;
    
    midifile.setTPQ(TPQ);//0音轨
    midifile.addTrack();//0音轨
    
    for(auto it:notes){
        if(it->info.empty()){
            track=0;
            
            midifile.addNoteOn(track, it->begin , 0, it->tone , it->volume > 100 ? 100 : it->volume);
            midifile.addNoteOff(track, it->begin + it->delay , 0, it->tone);
            
        }else{
            if(it->info.at(0)!='@'){//为@是控制字符
                auto tit=tracks.find(it->info);
                
                if(tit==tracks.end()){//没有音轨
                    midifile.addTrack();
                    tracks[it->info]=trackNum;
                    track=trackNum;
                    ++trackNum;
                    
                    midifile.addNoteOn(track, it->begin , 0, it->tone , it->volume > 100 ? 100 : it->volume);
                    midifile.addNoteOff(track, it->begin + it->delay , 0, it->tone);
                    
                }else{
                    track=tit->second;
                    
                    midifile.addNoteOn(track, it->begin , 0, it->tone , it->volume > 100 ? 100 : it->volume);
                    midifile.addNoteOff(track, it->begin + it->delay , 0, it->tone);
                    
                }
                
            }
        }
    }
    midifile.write(filename);
}

}