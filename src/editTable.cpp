#include "editTable.h"
namespace mgnr{

editTable::editTable(){
    displayBuffer.showing = false;
    automaticX=true;
    automaticY=true;
    lookAtX=0;
    lookAtY=64;
    noteHeight=20;
    noteLength=0.6;
    defaultDelay=120;
    maticBlock=120;
    setSection(4);
    defaultVolume=50;
    defaultInfo="default";
    selected.clear();
}

editTable::~editTable(){
    
}

void editTable::render(){
    drawNote_begin();
    findNote();
    drawDisplay();
    drawTempoLine();
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
            int num    = floor(x/maticBlock);
            float real = num*maticBlock;
            x=real;
        }
    }
    if(automaticY){
        y=(int)floor(y);
    }
}
void editTable::clickToRemoveTempo(int x,int y){
    auto p=screenToAbs(x,y);
    automatic(p.X , p.Y);
    if(EM_ASM_INT({return confirm("确定删除速度控制吗？")?1:0;})==1){
        auto res = removeTempoBeforePos(p.X);
        if(std::get<0>(res)==true){
            auto & tick = std::get<1>(res);
            auto & tempo = std::get<2>(res);
            printf("remove tempo:%d=>%f\n",tick,tempo);
            std::unique_ptr<history> hisptr (new history);//插入历史记录
            hisptr->method = history::H_TEMPO_DEL;
            hisptr->tempo = tempo;
            hisptr->begin = tick;
            histories.push_back(std::move(hisptr));
        }
    }
}
void editTable::clickToSetTempo(int x,int y){
    auto p=screenToAbs(x,y);
    automatic(p.X , p.Y);
    double res = EM_ASM_DOUBLE({
        var jsString = prompt("设置速度（BPM）");
        if(!jsString)
            return -1;
        var r = Number(jsString);
        return r;
    });
    if(res>0){
        if(addTempo(p.X,res)){
            std::unique_ptr<history> hisptr (new history);//插入历史记录
            hisptr->method = history::H_TEMPO_ADD;
            hisptr->tempo = res;
            hisptr->begin = p.X;
            histories.push_back(std::move(hisptr));
        }
    }
}

note * editTable::clickToAdd(int x,int y){
    auto p=screenToAbs(x,y);
    automatic(p.X , p.Y);
    auto ptr = addNote(p.X , p.Y , defaultDelay , defaultVolume , defaultInfo);
    
    printf("add %d %d\n",x,y);
    std::unique_ptr<history> hisptr (new history);//插入历史记录
    hisptr->method = history::H_NOTE_ADD;
    hisptr->noteIds.push_back(ptr->id);
    histories.push_back(std::move(hisptr));
    
    return ptr;
}
void editTable::clearSelected(){
    for(auto it:selected){
        it->selected=false;
    }
    selected.clear();
}
void editTable::clearNotes(){
    clear();
    selected.clear();
}
void editTable::removeSelected(){
    if(selected.empty())
        return;
    
    std::unique_ptr<history> hisptr (new history);//插入历史记录
    hisptr->method = history::H_NOTE_DEL;
    
    for(auto it:selected){
        std::unique_ptr<noteInfo> np(new noteInfo(it));
        hisptr->notes.push_back(std::move(np));
        removeNote(it);
    }
    
    printf("delete notes\n");
    selected.clear();
    
    histories.push_back(std::move(hisptr));
}
void editTable::resizeSelected(int delta){
    auto rd = (float)delta/((float)noteLength*256);
    for(auto it:selected){
        it->delay+=rd;
        if(it->delay<10)
            it->delay=10;
    }
}
void editTable::resizeSelected_apply(){
    for(auto it:selected){
        resizeNote(it);
    }
}
void editTable::renameSelected(const std::string & n){
    onUseInfo(n);
    for(auto it:selected){
        if(!it->info.empty()){
            if(it->info[0]=='@'){
                removeControl(it->begin,it->info);
            }
        }
        it->info = n;
        if(!n.empty()){
            if(n[0]=='@'){
                addControl(it->begin,n);
            }
        }
    }
}
int editTable::selAll(){
    int num;
    if(infoFilter.empty()){//无过滤
    
        for(auto it:notes){
            it->selected=true;
            selected.insert(it);
            ++num;
        }
    
    }else{
        
        for(auto it:notes){
            if(it->info==infoFilter){
                it->selected=true;
                selected.insert(it);
                ++num;
            }
        }
        
    }
    return num;
}
int editTable::clickToSelect(int x,int y){
    auto p=screenToAbs(x,y);
    
    return find(p,[](note * n , void * arg){//调用HBB搜索
        auto self = (editTable*)arg;
        if(!n->selected){//未选择就加上选择
            self->selected.insert(n);
            printf("select:%f %f %s delay:%f volume:%d\n",n->begin,n->tone,n->info.c_str(),n->delay,n->volume);
            n->selected=true;
            
            if(n->info != self->defaultInfo){
                printf("use note name:%s\n",n->info.c_str());
                self->defaultInfo = n->info;
            }
            
        }else{
            self->selected.erase(n);//第二次点击取消选择
            printf("unselect:%f %f\n",n->begin,n->tone);
            n->selected=false;
        }
    },this);
    
}
int editTable::selectByArea_unique(int x,int y,int len){
    auto f = HBB::vec(x,0);
    auto t = HBB::vec(x+len,128);
    
    int selNum = 0;
    
    find(f,t,[](note * n , void * arg){//调用HBB搜索
        int * num = (int*)arg;
        if(n->selected){
            ++(*num);
        }
    },&selNum);
    
    if(selNum>0)
        return 0;
    
    f.Y = y;
    t.Y = y+0.9;
    
    return find(f,t,[](note * n , void * arg){
        auto self = (editTable*)arg;
        if(!n->selected){//未选择就加上选择
            self->selected.insert(n);
            n->selected=true;
        }
    },this);
    
}
int editTable::selectByArea(int x,int y,int len){
    
    auto f = HBB::vec(x,y);
    auto t = HBB::vec(x+len,y+0.9);
    
    return find(f,t,[](note * n , void * arg){//调用HBB搜索
        auto self = (editTable*)arg;
        if(!n->selected){//未选择就加上选择
            self->selected.insert(n);
            n->selected=true;
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
        
        printf("add %f %f\n",displayBuffer.begin , displayBuffer.tone);
        std::unique_ptr<history> hisptr (new history);//插入历史记录
        hisptr->method = history::H_NOTE_ADD;
        hisptr->noteIds.push_back(addNote(displayBuffer.begin , displayBuffer.tone , defaultDelay , defaultVolume , defaultInfo)->id);
        histories.push_back(std::move(hisptr));
        
        displayBuffer.showing=false;
    }
}

void editTable::findNote(){
    noteAreaHeight=((float)windowHeight)/((float)noteHeight);//音符区域高度=窗口高度/音符图片高度
    noteAreaWidth =((float)windowWidth)/noteLength;//音符区域宽度=窗口宽度/音符长度比例
    
    realLookAtY = lookAtY-noteAreaHeight/2;//左上角Y位置=中心Y位置-音符区域高度/2
    //realLookAtX=lookAtX
    
    drawTableRaws();
    
    drawTableColumns();
    drawSectionLine();
    
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

void editTable::drawTableColumns(){
    float p;
    float r;
    float delta  =maticBlock*noteLength;
    if(delta<5)
        return;
    int   befn   =lookAtX/maticBlock;
    float start  =(befn+1)*maticBlock;
    r=start;
    
    //printf("%f %f\n",p,r);
    
    while(1){
        
        p=(r-lookAtX)*noteLength;
        
        if(p>=windowWidth)
            break;
        
        if(p>30 && r>0)
            drawTimeCol(p);
        
        r+=maticBlock;
        
    }
}
void editTable::drawTempoLine(){
    drawTempoPadd();
    getTempo(lookAtX,[&](int tick,double tempo){
        auto p=(tick-lookAtX)*noteLength;
        if(p<0)
            p = 0;
        if(p>windowWidth){
            return false;
        }else{
            drawTempo(p,tempo);
        }
        return true;
    });
}
void editTable::drawSectionLine(){
    float p;
    float r;
    float delta  =sectionLen*noteLength;
    if(delta<5)
        return;
    int   befn   =lookAtX/sectionLen;
    float start  =(befn+1)*sectionLen;
    r=start;
    
    while(1){
        
        ++befn;
        
        p=(r-lookAtX)*noteLength;
        
        if(p>=windowWidth)
            break;
        
        if(p>30 && r>0)
            drawSectionCol(p,befn+1);
        
        r+=sectionLen;
        
    }
    if(lookAtX<0){
        float posi = (-lookAtX)*noteLength;
        if(posi>30 && posi<windowWidth)
            drawSectionCol(posi,1);
    }
}
void editTable::drawTableRaws(){
    int p;
    int ilookAtY=lookAtY;
    if(ilookAtY>=0 && ilookAtY<128){
        p=ilookAtY;
        while(1){
            if(p>=128 || p<0)
                break;
            if(!drawToneRaw(p))
                break;
            --p;
        }
        p=ilookAtY+1;
        while(1){
            if(p>=128 || p<0)
                break;
            if(!drawToneRaw(p))
                break;
            ++p;
        }
    }else
    if(ilookAtY<0){
        p=0;
        while(1){
            if(p>=128 || p<0)
                break;
            if(!drawToneRaw(p))
                break;
            ++p;
        }
    }else{
        p=127;
        while(1){
            if(p>=128 || p<0)
                break;
            if(!drawToneRaw(p))
                break;
            --p;
        }
    }
}

#define vPosi(n,max) \
    if(n<0) \
        n=0; \
    else \
    if(n>max) \
        n=max;
    

bool editTable::drawToneRaw(int t){
    float relY = t  - realLookAtY;
    int scrY = relY * noteHeight;
    int scrYto = windowHeight - scrY;
    scrY = scrYto - noteHeight;
    
    //vPosi(scrY   ,windowHeight);
    //vPosi(scrYto ,windowHeight);
    
    //if(scrYto - scrY != noteHeight)
    //    printf("%d %d\n",scrY,scrYto);
    
    if((scrYto>windowHeight && scrY>windowHeight) || (scrYto<0 && scrY<0))
        return false;
    
    drawTableRaw(scrY,scrYto,t);
    
    
    return true;
}

void editTable::drawNoteAbs(float begin,float tone,float delay,float volume,const std::string & info,bool selected,bool onlydisplay){
    float relX = begin - lookAtX;//相对坐标
    float relY = tone  - realLookAtY;
    
    int scrX = relX * noteLength;
    int scrY = relY * noteHeight;
    
    
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

void editTable::toString(std::string & str){
    str="MGNR V1.0\n";
    char tbuf[1024];
    snprintf(tbuf,sizeof(tbuf),"!T%d\n",TPQ);
    str+=tbuf;
    for(auto it:notes){
        snprintf(tbuf,sizeof(tbuf),"+%s %f %f %f %d\n",it->info.c_str(),it->begin,it->tone,it->delay,it->volume);
        str+=tbuf;
    }
    for(auto it:timeMap){
        snprintf(tbuf,sizeof(tbuf),"!B%d %lf\n",it.first,it.second);
        str+=tbuf;
    }
}
void editTable::loadString(const std::string & str){
    std::istringstream iss(str);
    char buf[1024];
    while(!iss.eof()){
        bzero(buf,1024);
        iss.getline(buf,1024);
        if(buf[0]=='+'){
            if(strlen(buf)>2){
                
                std::istringstream ts(buf+1);
            
                std::string info;
                float position;
                float tone;
                float delay;
                int v;
            
                ts>>info;
                ts>>position;
                ts>>tone;
                ts>>delay;
                ts>>v;
            
                addNote(position,tone,delay,v,info);
                
            }
        }else
        if(buf[0]=='!'){
            if(strlen(buf)>=3){
                if(buf[1]=='T'){
                    int t=atoi(buf+2);
                    TPQ=t;
                    rebuildNoteLen();
                }else if(buf[1]=='B'){
                    int tick = 0;
                    double tpo = 120;
                    if(sscanf(buf+2 , "%d %lf",&tick,&tpo)>=2){
                        addTempo(tick,tpo);
                    }
                }
            }
        }
    }
}
void editTable::selectedToRelative(std::string & out){
    std::map<int,std::set<note*> > ns;
    for(auto it:selected){
        ns[it->begin].insert(it);
    }
    std::list<int> single;
    for(auto it:ns){
        auto topn =it.second.begin();
        if(topn != it.second.end()){
            single.push_back((*topn)->tone);
        }
    }
    char buf[64];
    out="0 ";
    if(!single.empty()){
        auto it=single.begin();
        int last = *it;
        ++it;
        for(;it!=single.end();++it){
            int delta = *it - last;
            last = *it;
            snprintf(buf,64,"%d ",delta);
            out+=buf;
        }
    }
}

void editTable::undo(){
    auto it = histories.rbegin();
    if(it!=histories.rend()){
        if((*it)->method==history::H_NOTE_ADD){
            for(auto id:(*it)->noteIds){
                removeNoteById(id);
            }
        }else if((*it)->method==history::H_NOTE_DEL){
            for(auto & itn:(*it)->notes){
                addNote(itn->position,itn->tone,itn->delay,itn->volume,itn->info);
            }
        }else if((*it)->method==history::H_TEMPO_ADD){
            timeMap.erase((*it)->begin);
        }else if((*it)->method==history::H_TEMPO_DEL){
            addTempo((*it)->begin,(*it)->tempo);
        }
        histories.pop_back();
    }
}
void editTable::addChord(float position,const std::string & root , const std::string & name , const char * format, float length , int root_base,int v,const std::string & info,bool useTPQ){
    auto cmnit = chord_map_note.find(root);
    auto cmit = chord_map.find(name);
    if(cmnit==chord_map_note.end() || cmit==chord_map.end() || length<=0)
        return;
    
    int lform = strlen(format);

    int root_note = cmnit->second + root_base*12;
    const std::vector<int> & chord = cmit->second;
    float tm  = length/lform;
    float pos = position;
    if(useTPQ){
        tm*=TPQ;
        pos*=TPQ*length;
    }
    pos += XShift*TPQ;
    
    std::unique_ptr<history> hisptr (new history);//插入历史记录
    hisptr->method = history::H_NOTE_ADD;
    for(int i=0;i<lform;++i){
        int dis = format[i]-'0';
        try{
            if(dis>=0 && dis<notes.size()){
                int note = root_note + chord.at(dis) +baseTone;
                hisptr->noteIds.push_back(addNote(pos , note , tm , v , info)->id);
            }
        }catch(...){}
        pos += tm;
    }
    histories.push_back(std::move(hisptr));
}
void editTable::addChord(float position,const std::string & name, float length,int root_base,int v,const std::string & info,const std::string & format,bool useTPQ){
    char buf[128];
    snprintf(buf,128,"%s",name.c_str());
    char * bufp = buf;
    while(*bufp){
        if(*bufp=='-')*bufp = ' ';
        ++bufp;
    }
    std::string sbuf;
    std::istringstream iss(buf);
    int last = -1;
    std::vector<int> notes;
    while(1){
        sbuf = "";
        iss>>sbuf;
        auto it = note_number_map.find(sbuf);
        if(it==note_number_map.end()){
            break;
        }
        int note = it->second;
        while(note<last){
            note+=12;
        }
        last = note;
        notes.push_back(note);
        //printf("%d\n",note);
    }
    float pos = position;
    float len = length;
    if(useTPQ){
        len*=TPQ;
        pos*=TPQ*length;
    }
    pos += XShift*TPQ;
    
    std::unique_ptr<history> hisptr (new history);//插入历史记录
    hisptr->method = history::H_NOTE_ADD;
    if(format.empty()){
        for(auto it:notes){
            int note = root_base*12 + it + 60;
            hisptr->noteIds.push_back(addNote(pos , note , len , v , info)->id);
        }
    }else{
        auto fstr = format.c_str();
        int  flen = strlen(fstr);
        len /= flen;
        for(int i=0;i<flen;++i){
            int dis = fstr[i]-'0';
            try{
                if(dis>=0 && dis<notes.size()){
                    int note = root_base*12 + notes.at(dis) + 60;
                    hisptr->noteIds.push_back(addNote(pos , note , len , v , info)->id);
                }
            }catch(...){}
            pos += len;
        }
    }
    histories.push_back(std::move(hisptr));
}

}
