#ifndef MGNR_LCS
#define MGNR_LCS
#include <vector>
#include <functional>
namespace mgnr{
    template<typename T> //比较两个数组，callback返回相同的位置终点，(A位置，B位置，长度)
    void LCS(const std::vector<T> & A,const std::vector<T> & B,std::function<void (int,int,int)> const & callback){
        if(A.empty() || B.empty())
            return;
        std::vector<int> buf1,buf2;
        std::vector<int> * thisline=NULL  ,  * lastline=NULL;
        buf1.resize(A.size());
        buf2.resize(A.size());
        
        thisline = &buf1;
        //此时不存在上一行
        
        int al=A.size(),bl=B.size();
        //虚构一个矩阵 m[al][bl]
        for(int j=0;j<bl;++j){
            const T & elmb = B.at(j);
            for(int i=0;i<al;++i){
                const T & elma = A.at(i);
                if(elma == elmb){
                    if(lastline){
                        if(i>0)
                            thisline->at(i)=lastline->at(i-1)+1;
                        else
                            thisline->at(i)=1;
                    }else
                        thisline->at(i)=1;
                }else{
                    thisline->at(i)=0;
                }
            }
            if(lastline){
                for(int i=0;i<al;++i){
                    if(i==al-1 && lastline->at(i)>0){//最后一个位置
                        callback(i,j-1,lastline->at(i));
                    }else
                    if(lastline->at(i)>0 && thisline->at(i+1)==0){
                        callback(i,j-1,lastline->at(i));
                    }
                }
            }
            if(j==bl-1 && thisline){//清算没处理完的
                for(int i=0;i<al;++i){
                    if(thisline->at(i)>0){
                        callback(i,j,thisline->at(i));
                    }
                }
            }
            if(lastline==NULL){//是第一行
                lastline=thisline;
                thisline=&buf2;
            }else{
                auto tmp = thisline;//交换两行
                thisline = lastline;
                lastline = tmp;
            }
        }
    };
}
#endif
