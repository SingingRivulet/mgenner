#include "editTable.h"
#include "MidiFile.h"
#include "Options.h"
#include <iostream>
#include <map>
#include <string>
namespace mgnr{

using namespace std;
using namespace smf;

static const char * instrumentName[] = {
    "Piano",
    "BrightPiano",
    "ElectricPiano",
    "HonkyTonkPiano",
    "RhodesPiano",
    "ChorusedPiano",
    "Harpsichord",
    "Clavinet",
    "Celesta",
    "Glockenspiel",
    "MusicBoX",
    "Vibraphone",
    "Marimba",
    "Xylophone",
    "TubularBells",
    "Dulcimer",
    "HammondOrgan",
    "PercussiveOrgan",
    "RockOrgan",
    "ChurchOrgan",
    "ReedOrgan",
    "Accordion",
    "Harmonica",
    "TangoAccordian",
    "Guitar-nylon",
    "Guitar-steel",
    "Guitar-jazz",
    "Guitar-clean",
    "Guitar-muted",
    "OverdrivenGuitar",
    "DistortionGuitar",
    "GuitarHarmonics",
    "AcousticBass",
    "ElectricBass-finger",
    "ElectricBass-pick",
    "FretlessBass",
    "SlapBass1",
    "SlapBass2",
    "SynthBass1",
    "SynthBass2",
    "Violin",
    "Viola",
    "Cello",
    "Contrabass",
    "TremoloStrings",
    "PizzicatoStrings",
    "OrchestralHarp",
    "Timpani",
    "StringEnsemble1",
    "StringEnsemble2",
    "SynthStrings1",
    "SynthStrings2",
    "ChoirAahs",
    "VoiceOohs",
    "SynthVoice",
    "OrchestraHit",
    "Trumpet",
    "Trombone",
    "Tuba",
    "MutedTrumpet",
    "FrenchHorn",
    "BrassSection",
    "SynthBrass1",
    "SynthBrass2",
    "SopranoSaX",
    "AltoSaX",
    "TenorSaX",
    "BaritoneSaX",
    "Oboe",
    "EnglishHorn",
    "Bassoon",
    "Clarinet",
    "Piccolo",
    "Flute",
    "Record",
    "PanFlute",
    "BottleBlow",
    "Skakuhachi",
    "Whistle",
    "Ocarina",
    "Lead1-square",
    "Lead2-sawtooth",
    "Lead3-calliope",
    "Lead4-chiff",
    "Lead5-charang",
    "Lead6-voice",
    "Lead7-fifths",
    "Lead8-bass",
    "Pad1-newage",
    "Pad2-warm",
    "Pad3-polysynth",
    "Pad4-choir",
    "Pad5-bowed",
    "Pad6-metallic",
    "Pad7-halo",
    "Pad8-sweep",
    "FX1-rain",
    "FX2-soundtrack",
    "FX3-crystal",
    "FX4-atmosphere",
    "FX5-brightness",
    "FX6-goblins",
    "FX7-echoes",
    "FX8-sci-fi",
    "Sitar",
    "Banjo",
    "Shamisen",
    "Koto",
    "Kalimba",
    "Bagpipe",
    "Fiddle",
    "Shanai",
    "Tinkle Bell",
    "Agogo",
    "SteelDrums",
    "Woodblock",
    "TaikoDrum",
    "MelodicTom",
    "SynthDrum",
    "ReverseCymbal",
    "GuitarFretNoise",
    "BreathNoise",
    "Seashore",
    "BirdTweet",
    "TelephoneRing",
    "Helicopter",
    "Applause",
    "Gunshot"
};

void editTable::instrument2Id_init(){
    for(int i=0;i<128;++i){
        instrument2Id[instrumentName[i]] = i;
    }
}
int editTable::getInstrumentId(const std::string & n){
    auto it = instrument2Id.find(n);
    if(it==instrument2Id.end()){
        return 0;
    }else{
        return it->second;
    }
}
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
    
    std::set<int> iset;
    
    for (int track=0; track<tracks; track++) {
        //if (tracks > 1) cout << "\nTrack " << track << endl;
        
        //char infoBuf[128];
        //snprintf(infoBuf,sizeof(infoBuf),"track-%d",track);
        
        int instrumentId = 0;
        
        for (int event=0; event<midifile[track].size(); event++) {
            
            if (midifile[track][event].isNoteOn() && midifile[track][event].size()>=3){
                int position = midifile[track][event].tick;
                int delay = midifile[track][event].getTickDuration();
                int delayS = midifile[track][event].getDurationInSeconds();
                int tone = (int)midifile[track][event][1];
                int v = (int)midifile[track][event][2];
                addNote(position, tone, delay, v,instrumentName[instrumentId]);
                iset.insert(instrumentId);
            }else if(midifile[track][event].isTimbre()){
                instrumentId = midifile[track][event].getP1();
                if(instrumentId<0)
                    instrumentId = 0;
                else if(instrumentId>128)
                    instrumentId = 128;
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
        std::cout << "require instrument:" << instrumentName[it] << std::endl;
        EM_ASM({
            mgnr.requireInstrument($0);
        },it);
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
