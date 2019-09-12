#ifndef MGNR_SYNTH
#define MGNR_SYNTH
#include "player.h"
#include <set>
#include <list>
namespace mgnr{
    class synth:public player{
        public:
            void onNoteOn(note * n);
            void onNoteOff(note * n);
    };
}
#endif