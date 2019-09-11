#include "editTable.h"
#include "MidiFile.h"
#include "Options.h"
#include <iostream>
namespace mgnr{

using namespace std;
using namespace smf;

void editTable::loadMidi(const std::string & str){
    MidiFile midifile;
    midifile.read(str);
    midifile.doTimeAnalysis();
    midifile.linkNotePairs();
   
    cout << "TPQ: " << midifile.getTicksPerQuarterNote() << endl;
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
                //cout << position << " " << tone << " " << delay << " " << delayS << " " << v << " " << infoBuf << endl;
                addNote(position, tone, delay, v,infoBuf);
            }
        }
    }

}

}