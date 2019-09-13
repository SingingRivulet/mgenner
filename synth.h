#ifndef MGNR_SYNTH
#define MGNR_SYNTH
#include "player.h"
#include <set>
#include <list>
#include <SDL/SDL_audio.h>
#include <AL/al.h>
#include <AL/alc.h>
namespace mgnr{
    class synth:public player{
        public:
            void onNoteOn(note * n);
            void onNoteOff(note * n);
            void onUseInfo(const std::string & info);
        private:
            void callJsNoteOn(const char * info,int tone,int vol);
            void callJsNoteOff(const char * info,int tone);
    };
}
#endif