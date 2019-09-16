#ifndef MGNR_SYNTH
#define MGNR_SYNTH
#include "player.h"
#include <set>
#include <list>
#include <vector>
#include <SDL/SDL_audio.h>
#include <AL/al.h>
#include <AL/alc.h>
namespace mgnr{
    struct voiceWord{
        int length;//时长
        int tone;//音高（-1为休止符）
        int volume;
        std::string word;//词汇
        voiceWord(int l,int t,int v,const std::string & w){
            length  = l;
            tone    = t;
            word    = w;
            volume  = v;
        }
        voiceWord(const voiceWord & i){
            length  = i.length;
            tone    = i.tone;
            word    = i.word;
            volume  = i.volume;
        }
    };
    struct voiceTrack{
        std::list<voiceWord> words;
        int length;
        void pushPause(int len);
        void pushNote(int len,const std::string & info,int tone,int v);
        voiceTrack(){
            length=0;
            words.clear();
        }
    };
    class synth:public player{
        public:
            void onNoteOn(note * n);
            void onNoteOff(note * n);
            void onUseInfo(const std::string & info);
            
            void synthOutput();//链接外部引擎合成
            
            std::vector<voiceTrack*> tracks;
            
            ~synth();
            
        private:
            void callJsNoteOn(const char * info,int tone,int vol);
            void callJsNoteOff(const char * info,int tone);
            
            void splitTracks();//将midimap分解成音轨
            void clearTracks();
            
            void synthOutput_init();
            void synthOutput_addChannel(int);
            void synthOutput_addIntoChannel(int channel,const voiceTrack&);
            void synthOutput_free();
    };
}
#endif