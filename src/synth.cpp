#include "synth.h"
namespace mgnr{

synth::~synth(){
    clearTracks();
}

void synth::callJsNoteOn(const char * info,int tone,int vol){
    EM_ASM({
        try{
            var info=UTF8ToString($0);
            var tone=$1;
            var vol =$2;
            mgnr.noteOn(info, tone, vol);
        }catch(e){}
    }, info,tone,vol);
}
void synth::callJsNoteOff(const char * info,int tone){
    EM_ASM({
        try{
            var info=UTF8ToString($0);
            var tone=$1;
            mgnr.noteOff(info, tone);
        }catch(e){}
    }, info,tone);
}
void synth::onNoteOn(note * n){
    if(!n->info.empty()){
        if(n->info[0]=='@')
            return;
    }
    callJsNoteOn(n->info.c_str(), n->tone, n->volume);
}

void synth::onNoteOff(note * n){
    if(!n->info.empty()){
        if(n->info[0]=='@')
            return;
    }
    callJsNoteOff(n->info.c_str(), n->tone);
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
}

}
