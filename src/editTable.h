#ifndef MGNR_EDIT_TABLE
#define MGNR_EDIT_TABLE
#include "midiMap.h"
#include <emscripten.h>
#include <sstream>
#include <list>
#include <stdio.h>
#include <memory>
namespace mgnr{
    struct noteInfo{
        //音符数据（如果是删除的话）
        float position;
        float tone;
        float delay;
        int volume;
        std::string info;
        noteInfo(note * p){
            position = p->begin;
            tone     = p->tone;
            delay    = p->delay;
            volume   = p->volume;
            info     = p->info;
        }
    };
    struct history{
        enum{
            H_NOTE_ADD,
            H_NOTE_DEL,
            H_TEMPO_ADD,
            H_TEMPO_DEL
        }method;
        std::list<int> noteIds;//音符，如果是添加的话，将会存在
        int begin;//起始时间
        double tempo;//添加删除速度时使用
        std::list<std::unique_ptr<noteInfo> > notes;
    };
    class editTable:public midiMap{
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
            int  clickToSelect(int x,int y);
            void clickToSetTempo(int x,int y);
            void clickToRemoveTempo(int x,int y);
            int  selAll();
            void clearSelected();
            void removeSelected();
            void selectedToRelative(std::string & out);
            void clearNotes();
            void renameSelected(const std::string & n);
            void resizeSelected(int delta);
            void resizeSelected_apply();
            void automatic(float & x,float & y);
            void clickToDisplay(int x,int y);
            void clickToDisplay_close();
            
            int selectByArea(int x,int y,int len);
            int selectByArea_unique(int x,int y,int len);
            
            void drawNoteAbs(note*);//画音符绝对坐标
            void drawNoteAbs(float begin,float tone,float delay,float volume,const std::string & info,bool selected,bool onlydisplay=false);
            void findNote();//根据参数找到搜索矩形，利用HBB找到音符
            void drawTableRaws();
            bool drawToneRaw(int t);
            void drawTableColumns();
            void drawSectionLine();
            void drawTempoLine();
            
            virtual void drawTableRaw(int from,int to,int t)=0;
            virtual void drawTimeCol(float p)=0;
            virtual void drawSectionCol(float p,int n)=0;
            virtual void drawTempo(float p,double t)=0;
            virtual void drawTempoPadd()=0;
            
            void toString(std::string & str);
            void loadString(const std::string & str);
            void loadMidi(const std::string & str);
            void exportMidi(const std::string & str);
            
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
            std::string defaultInfo;//信息
            
            std::set<note*> selected;

            void addChord(float position,
                const std::string & root , const std::string & name , const char * format, float length , int root_base = 0,int v = 70,const std::string & info = "default",bool useTPQ = true);
            void addChord(float position,const std::string & name, float length , int root_base = 0,int v = 70,const std::string & info = "default", const std::string & format="",bool useTPQ = true);
            
            int getInstrumentId(const std::string & n);
            void loadInstrument(int id);
            inline void loadInstrument(const std::string & n){
                loadInstrument(getInstrumentId(n));
            }
            
        private:
            float noteAreaHeight;
            float noteAreaWidth;
            float realLookAtY;
            std::unordered_map<std::string,int> instrument2Id;
            void instrument2Id_init();
            bool instrumentLoaded[128];
            
        public:
            struct{
                float begin;
                float tone;
                bool  showing;
            }displayBuffer;
            void undo();
        private:
            std::list<std::unique_ptr<history> > histories;
    };
}
#endif
