#include "synth.h"
namespace mgnr{

synth::~synth(){
    clearTracks();
}

void synth::callJsNoteOn(const char * info,int channel,int tone,int vol){
    EM_ASM({
        try{
            var info=UTF8ToString($0);
            var tone=$1;
            var vol =$2;
            var channel = $3;
            mgnr.noteOn(info,channel, tone, vol);
        }catch(e){}
    }, info,tone,vol,channel);
}
void synth::callJsNoteOff(const char * info,int channel,int tone){
    EM_ASM({
        try{
            var info=UTF8ToString($0);
            var tone=$1;
            var channel = $2;
            mgnr.noteOff(info,channel, tone);
        }catch(e){}
    }, info,tone,channel);
}
void synth::onNoteOn(note * n,int c){
    if(!n->info.empty()){
        if(n->info[0]=='@')
            return;
    }
    //printf("onNoteOn:%f %d\n",n->tone,c);
    callJsNoteOn(n->info.c_str(),c, n->tone, n->volume);
}

void synth::onNoteOff(note * n,int c){
    if(!n->info.empty()){
        if(n->info[0]=='@')
            return;
    }
    //printf("onNoteOff:%f %d\n",n->tone,c);
    callJsNoteOff(n->info.c_str(),c, n->tone);
}
void synth::onSetChannelIns(int c,int ins){
    EM_ASM({
        mgnr.setChannelInstrument($0,$1);
    },c,ins);
}
void synth::onUseInfo(const std::string & info){
    if(info.empty())
        return;
    if(info[0]=='@')
        return;
    EM_ASM({
        var info=UTF8ToString($0);
        mgnr.loadName(info);
    },info.c_str());
    loadInstrument(info);
}

}
