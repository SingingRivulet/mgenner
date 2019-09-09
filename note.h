#ifndef MGNR_MIDI_NOTE
#define MGNR_MIDI_NOTE
#include <string>
namespace mgnr{
    class note{
        public:
            float begin;
            float tone;
            float delay;
            int volume;
            
            std::string info;
            
            bool selected;
            
            inline void construct(){
                selected = false;
            }
            
            void * indexer;
            
            note * next;//内存池变量，不应该被修改
    };
}
#endif