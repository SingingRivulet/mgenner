#include "synth.h"
namespace mgnr{
void synth::callJsNoteOn(const char * info,int tone,int vol){
    EM_ASM({
        var info=UTF8ToString($0);
        var tone=$1;
        var vol =$2;
        MIDI.noteOn(0, tone, vol, 0);
    }, info,tone,vol);
}
void synth::callJsNoteOff(const char * info,int tone){
    EM_ASM({
        var info=UTF8ToString($0);
        var tone=$1;
        MIDI.noteOff(0, tone, 0);
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
    
}

}