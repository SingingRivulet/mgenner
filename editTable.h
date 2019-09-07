#ifndef MGNR_EDIT_TABLE
#define MGNR_EDIT_TABLE
#include "midiMap.h"
namespace mgnr{
    class editTable:midiMap{
        public:
            editTable();
            ~editTable();
            
            void render();
            void drawDisplay();
            virtual void drawNote_begin()=0;
            virtual void drawNote(int fx,int fy,int tx,int ty, int volume,const std::string & info,bool selected,bool onlydisplay=false)=0;
            virtual void drawNote_end()=0;
            
            HBB::vec screenToAbs(int x,int y);//屏幕坐标转midi绝对坐标
            
            note * clickToAdd(int x,int y);
            void addDisplaied();
            void clickToSelect(int x,int y);
            void clearSelected();
            void removeSelected();
            void automatic(float & x,float & y);
            void clickToDisplay(int x,int y);
            void clickToDisplay_close();
            
            void drawNoteAbs(note*);//画音符绝对坐标
            void drawNoteAbs(float begin,float tone,float delay,float volume,const std::string & info,bool selected,bool onlydisplay=false);
            void findNote();//根据参数找到搜索矩形，利用HBB找到音符
            
            float lookAtX;//瞄准位置（左边缘中心点）
            float lookAtY;
            
            int noteHeight;//音符图片高度
            float noteLength;//音符长度比例
            
            int windowHeight;//窗口高度
            int windowWidth;//窗口宽度
            
            bool  automaticX;//自动吸附模式（起始）
            bool  automaticY;//自动吸附模式（音高）
            float maticBlock;//起始时间吸附到这个的整数倍
            float defaultDelay;//持续时间
            int   defaultVolume;//音量
            std::string defaultInfo;//音量
            
            std::set<note*> selected;
            
        private:
            float noteAreaHeight;
            float noteAreaWidth;
            float realLookAtY;
            
            struct{
                float begin;
                float tone;
                bool  showing;
            }displayBuffer;
    };
}
#endif