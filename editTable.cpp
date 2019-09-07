#include "editTable.h"
namespace mgnr{

editTable::editTable(){
    displayBuffer.showing = false;
}

editTable::~editTable(){
    
}

void editTable::render(){
    drawNote_begin();
    findNote();
    drawDisplay();
    drawNote_end();
}

void editTable::drawDisplay(){
    if(displayBuffer.showing){
        drawNoteAbs(
            displayBuffer.begin , 
            displayBuffer.tone , 
            defaultDelay , 
            defaultVolume , 
            defaultInfo ,
            false ,
            true
        );
    }
}

HBB::vec editTable::screenToAbs(int x,int y){
    //由于使用了三个私有变量，此操作必须在render后进行，否则结果将不准确
    float absy = ((float)(windowHeight - y))/noteHeight + realLookAtY;//先上下对称处理，然后除以高度，再加上左上角坐标
    float absx = ((float)x)/noteLength + lookAtX;//除以长度，加上左上角坐标
    HBB::vec res;
    res.X=absx;
    res.Y=absy;
    return res;
}

void editTable::automatic(float & x,float & y){
    if(automaticX){
        if(maticBlock>0){
            int num    = x/maticBlock;
            float real = num*maticBlock;
            x=real;
        }
    }
    if(automaticY){
        y=(int)y;
    }
}

note * editTable::clickToAdd(int x,int y){
    auto p=screenToAbs(x,y);
    automatic(p.X , p.Y);
    return addNote(p.X , p.Y , defaultDelay , defaultVolume , defaultInfo);
}
void editTable::clearSelected(){
    for(auto it:selected){
        it->selected=false;
    }
    selected.clear();
}
void editTable::removeSelected(){
    for(auto it:selected){
        removeNote(it);
    }
    selected.clear();
}
void editTable::clickToSelect(int x,int y){
    auto p=screenToAbs(x,y);
    
    find(p,[](note * n , void * arg){//调用HBB搜索
        auto self = (editTable*)arg;
        if(!n->selected){//未选择就加上选择
            self->selected.insert(n);
            n->selected=true;
        }else{
            self->selected.erase(n);//第二次点击取消选择
            n->selected=false;
        }
    },this);
    
}

void editTable::clickToDisplay(int x,int y){
    auto p=screenToAbs(x,y);
    automatic(p.X , p.Y);
    displayBuffer.begin=p.X;
    displayBuffer.tone=p.Y;
    displayBuffer.showing=true;
}

void editTable::clickToDisplay_close(){
    displayBuffer.showing=false;
}

void editTable::addDisplaied(){
    if(displayBuffer.showing){
        addNote(displayBuffer.begin , displayBuffer.tone , defaultDelay , defaultVolume , defaultInfo);
        displayBuffer.showing=false;
    }
}

void editTable::findNote(){
    noteAreaHeight=((float)windowHeight)/((float)noteHeight);//音符区域高度=窗口高度/音符图片高度
    noteAreaWidth =((float)windowWidth)/noteLength;//音符区域宽度=窗口宽度/音符长度比例
    
    realLookAtY = lookAtY-noteAreaHeight/2;//左上角Y位置=中心Y位置-音符区域高度/2
    //realLookAtX=lookAtX
    
    HBB::vec from;
    from.set(lookAtX,realLookAtY);
    
    HBB::vec to=from;
    to.X+=noteAreaWidth;
    to.Y+=noteAreaHeight;
    
    find(from,to,[](note * n , void * arg){//调用HBB搜索
        auto self = (editTable*)arg;
        self->drawNoteAbs(n);
    },this);
}

void editTable::drawNoteAbs(note * n){
    drawNoteAbs(n->begin,n->tone,n->delay,n->volume , n->info , n->selected);
}

void editTable::drawNoteAbs(float begin,float tone,float delay,float volume,const std::string & info,bool selected,bool onlydisplay){
    float relX = begin - lookAtX;//相对坐标
    float relY = tone  - realLookAtY;
    
    int scrX = relX * noteLength;
    int scrY = relY * noteHeight;
    
    #define vPosi(n,max) \
        if(n<0) \
            n=0; \
        else \
        if(n>max) \
            n=max;
    
    int scrYto = windowHeight - scrY;//y坐标上下翻转，因为屏幕坐标系和midi坐标系上下相反
    
    scrY = scrYto - noteHeight;//翻转过，当然要减
    
    int scrXto = scrX + delay * noteLength;
    
    vPosi(scrX   ,windowWidth);//验证
    vPosi(scrXto ,windowWidth);
    vPosi(scrY   ,windowHeight);
    vPosi(scrYto ,windowHeight);
    
    if(scrX==scrXto && (scrX==0 || scrX==windowWidth))
        return;//长度为0并且在边缘，不予显示（因为显示不出来）
    
    if(scrY==scrYto && (scrY==0 || scrY==windowHeight))
        return;//长度为0并且在边缘，不予显示（因为显示不出来）
    
    drawNote(scrX , scrY , scrXto , scrYto , volume , info , selected , onlydisplay);
}

}