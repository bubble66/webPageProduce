#include <iostream>
#include <unistd.h>
#include <string>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <sstream>
#include <fstream>
#include <errno.h>
#include <stdlib.h>
#include <errno.h>
#include <dirent.h>
#include <fcntl.h>
#include <stack>
#include <map>
#include <vector>
#include <cctype>
#include <iconv.h> //用于编码转换
#include "newthreadpool.h"
#include "crawler.h"
#include "kmeansCluster.h"
#include <libxml/parser.h>
#include <libxml/tree.h>

#include "publicPart.h"
#include "getWebPageByURLClass.h"

#include "contentProcessClass.h"

using namespace std;
                    
void printMsg(struct lableMsg msg)
{
    cout<<"lableName: "<<msg.lableName<<"  " ;
    cout<<"lableFlag: " <<( (msg.lableFlag == 0) ? "error" : (msg.lableFlag == LEFTLABLE?  "leftLable" : "rightLable") )<<"  ";
    cout<<"beginIndex: "<<msg.beginIndex <<"  endIndex: " << msg.endIndex<<endl;
}


lableMsg getLableMsg(string &content,int findPostIndex)
{
    int rightIndex, spaceIndex;
    int leftIndex = content.find("<",findPostIndex);
    if(leftIndex == string::npos)
    {
        struct lableMsg msg;
        //return lableMsg("");
        return msg;
    }
    if(content[leftIndex+1] == '/')  //右标签
    {
        rightIndex = content.find(">",leftIndex+1);
        if(rightIndex == string::npos)
        {
            /*
             cout<<"------------------------------------\n";
             cout<<content.size()<<endl;
             cout<<leftIndex<<endl;
             cout<<content.substr(leftIndex-10)<<endl;
             cout<<"------------------------------------\n";
             */
            content.erase(content.begin() + leftIndex,content.end());
            struct lableMsg msg;
            return msg;
        }
        
        //  return lableMsg
        
        string lableName = content.substr(leftIndex+2,rightIndex-leftIndex-2);
        struct lableMsg msg(lableName,leftIndex,rightIndex,RIGHTLABLE);
        return msg;
    }
    else
    {
        rightIndex = content.find(">",leftIndex +1);
        if(rightIndex == string::npos)
        {
            /*
             cout<<"------------------------------------\n";
             cout<<content.size()<<endl;
             cout<<leftIndex<<endl;
             cout<<content.substr(leftIndex-10)<<endl;
             cout<<"------------------------------------\n";
             */
            content.erase(content.begin() + leftIndex,content.end());
            struct lableMsg msg;
            return msg;
        }
        if(content[rightIndex-1] == '/')   //单个不配对标签  eg：<input .....  />
        {
            content.erase(content.begin() + leftIndex,content.begin() + rightIndex + 1);
            return getLableMsg(content, findPostIndex);
        }
        string tmpcontent = content.substr(leftIndex+1,rightIndex - leftIndex-1);
        spaceIndex = tmpcontent.find(" ");
        string lableName;
        if(spaceIndex != string::npos)
            lableName = tmpcontent.substr(0, spaceIndex);
        else
            lableName = tmpcontent;
        struct lableMsg msg(lableName,leftIndex,rightIndex,LEFTLABLE);
        return msg;
    }
}


//判断标签内容是否为空
bool  inner_content_is_empty(string str)
{
    int len = str.size();
    if(len <= 1)
    {
        return true;
    }
    
    for(int i=0; i<len ; i++)
    {
        //if(str[i] == '\r')
        //  cout<<"change line\n";
        if(str[i] ==' ' || str[i] == '\t' || str[i] == '\r' || str[i] =='\n' || str[i] =='-' || str[i] == '|')
            continue;
        else
            return false;
    }
    return true;
}

string  get_destDirectory_by_time1()
{
    time_t  tm;
    time(&tm);
    string tmpstr = ctime(&tm);
    for(  int i=0;i<tmpstr.size();i++)
    {
        if(tmpstr[i] == ' ' || tmpstr[i] == ':')
        {
            tmpstr[i] = '_';
        }
        if(tmpstr[i] == '\n')
        {
            tmpstr.erase(tmpstr.begin()+i);
        }
    }
    return tmpstr;
}


int main1()
{
    crawler *myCrawler = new crawler(3,100);
    cout<<"begi３３n...\n";
    myCrawler->crawlerAddUrl("www.zhihu.com/question/36531933#answer-22595076");
    cout<<"end...\n";
    sleep(12);
    //string test("http://sports.sina.com.cn/nba/2015-04-24/12277585850.shtml");
    //int t = test.rfind(".");
    //cout<<test.substr(t+1)<<endl;
    delete myCrawler;
    return 0;
}
int xmlParser(string files){
    xmlDocPtr doc;
    xmlNodePtr curNode;
    xmlChar *key;
    doc = xmlReadFile(files.c_str(), "utf-8", 256);
    if(doc == NULL)
    {
        cout<<"xml open error!"<<endl;
        cout<<strerror(errno)<<endl;
    }
    else
        cout<<"xml open success!"<<endl;
    
    curNode = xmlDocGetRootElement(doc);
    if(curNode == NULL)
        cout<<"doc is empty"<<endl;
    else
    {
        cout<< curNode->name<<endl;
    }
    
    curNode = curNode->children;
    while(curNode != NULL)
    {
        key = xmlNodeGetContent(curNode);
        cout<<key<<endl;
        curNode = curNode->next;
        cout<<"---------------------------"<<endl;
        xmlFree(key);
    }
    cout<<"endl"<<endl;
    return 0;
}




//将标签特性转化为特征向量
vector<Feature> get_feature_vector(vector<struct lableFeature> lf)
{
    vector<Feature> ret;
    for(int i=0;i<lf.size();i++)
    {
        Feature tmpf;
        tmpf.push_back(lf[i].lableId);
        tmpf.push_back(lf[i].lableContentLength);
        tmpf.push_back(lf[i].lableLeftPartLength);
        tmpf.push_back(lf[i].lableRightPartLength);
        tmpf.push_back(lf[i].lablePunctNumber);
        tmpf.push_back(lf[i].lableLevelNumber);
        ret.push_back(tmpf);
    }
    return ret;
}

//打印标签特征
void print_lable_feature(struct lableFeature lf)
{
    cout<<"lableId:" << lf.lableId << "\n";
    cout<<"lableLevelNumber:" << lf.lableLevelNumber << "\n";
    cout<<"lablePunctNumber:"<<lf.lablePunctNumber<<endl;
    cout<<"lableName:"<<lf.lableName <<"\nlableContent:" << lf.lableContent ;
    cout<<"\nlableBeginIndex:"<< lf.lableBeginIndex;
    cout<<"\nlableEndIndex:"<< lf.lableEndIndex;
    cout << "\nlableLeftPartContent:" <<lf.lableLeftPartContent<<endl;
    cout<<"lableAttributeVector:";
    for(int i=0;i<lf.lableAttributeVector.size();i++)
    {
        cout<<lf.lableAttributeVector[i]<<"  ";
    }
    
    cout<<"\n ---------------------------------------\n";
}

int stringFindPunct(string str,string pat)
{
    int ret=0;
    int findPostIndex = 0;
    while(1)
    {
        int tmp = str.find(pat,findPostIndex);
        if(tmp == string::npos)
            return ret;
        ret++;
        findPostIndex = tmp+1;
    }
    return ret;
}

//获取标点符号数量
int get_punct_number(string content)
{
    if(content.size() <= 0)
        return 0;
    int ret = 0;
    //cout<<content<<endl;
    for(int i=0; i<content.size(); i++)
    {
        if(ispunct(content[i]))
        //if( (content[i][0] & 0xff) ==0xA1   &&   (content[i][1] & 0xff)==0xA3 )
        {
            //cout<<"content:"<<content[i]<< "====is punch"<<endl;
            ret++;
        }
        else{
            //cout<<"content:"<<content[i]<< "====not punch"<<endl;
        }
    }
    ret += stringFindPunct(content,"，");
    ret += stringFindPunct(content,"。");
    ret += stringFindPunct(content,"；");
    ret += stringFindPunct(content,"“");
    ret += stringFindPunct(content,"”");
    ret += stringFindPunct(content,"！");
    ret += stringFindPunct(content,"‘");
    ret += stringFindPunct(content,"’");
    ret += stringFindPunct(content,"（");
    ret += stringFindPunct(content,"）");
    ret += stringFindPunct(content,"《");
    ret += stringFindPunct(content,"》");
    ret += stringFindPunct(content,"、");
    //cout<< ret<<endl;
    return ret;
}


//获取左标签的属性值
vector<string> get_lable_attribute_vector(string content)
{
    vector<string> ret;
    int findPostIndex = 0;
    while(1)
    {
        int spaceIndex = content.find(' ',findPostIndex);
        if(spaceIndex != string::npos)
        {
            while(spaceIndex < content.size() &&  content[spaceIndex] == ' ')
                spaceIndex ++;
            if(spaceIndex >= content.size())
                return ret;
            int equalIndex = content.find('=',spaceIndex);
            if(equalIndex == string::npos)
            {
                return ret;
            }
            else
            {
                findPostIndex = equalIndex;
                ret.push_back(content.substr(spaceIndex,equalIndex - spaceIndex));
            }
        }
        else
        {
            return ret;
        }
 
    }

    return ret;
}

//获取标签特征
vector<struct lableFeature> get_content_lable_feature(string content){
    vector<struct lableFeature> ret;
    stack<struct lableMsg> lableStack;
    int findPostIndex = 0;
    int lableId = 0;
    lableMsg msg = getLableMsg(content, findPostIndex);
    while(msg.lableFlag == 2)
    {
        findPostIndex = msg.endIndex +1;
        msg = getLableMsg(content, findPostIndex);
    }
    lableStack.push(msg);
    findPostIndex = msg.endIndex+1;
    
    while(1)
    {
        msg = getLableMsg(content, findPostIndex);
        if(msg.lableFlag == 0)
        {
            cout<<"get content lable feature successful\n";
            break;
        }
        if(msg.lableFlag == LEFTLABLE) //左标签
        {
            if(lableStack.empty())
            {
                lableStack.push(msg);
            }
            else
            {
                lableStack.top().leafLableFlag = false;
                lableStack.push(msg);
            }
            findPostIndex = msg.endIndex +1;
        }
        else   //右标签
        {
            if(lableStack.empty())
            {
                findPostIndex = msg.endIndex +1;
                continue;
            }
            struct lableMsg topMsg = lableStack.top();
            if(topMsg.lableName == msg.lableName && topMsg.lableFlag == LEFTLABLE )
            {
                if(topMsg.leafLableFlag == false)
                {
                    findPostIndex = msg.endIndex + 1;
                    lableStack.pop();
                    continue;
                }
                string lableName = msg.lableName;
                /*
                if(lableName == "div")
                {
                    findPostIndex = msg.endIndex + 1;
                    lableStack.pop();
                    continue;
                }
                 */
                string lableContent = content.substr(topMsg.endIndex+1, msg.beginIndex -  topMsg.endIndex -1);
                if(inner_content_is_empty(lableContent))
                {
                    findPostIndex = msg.endIndex + 1;
                    lableStack.pop();
                }
                else
                {
                    string lableLeftPartContent = content.substr(topMsg.beginIndex,topMsg.endIndex - topMsg.beginIndex +1);
                    int lableBeginIndex = topMsg.endIndex + 1;
                    int lableEndIndex = msg.beginIndex -1;
                    // lableId ++;
                    int lableContentLength = lableContent.size();
                    int lableLeftPartLength = lableLeftPartContent.size();
                    int lableRightPartLength = msg.lableName.size() + 3;
                    int lablePunctNumber = get_punct_number(lableContent);
                    vector<string> lableAttributeVector = get_lable_attribute_vector(lableLeftPartContent);
                    int lableLevelNumber = lableStack.size();
                    struct lableFeature lf(lableName,lableContent,lableLeftPartContent,lableBeginIndex,lableEndIndex,
                                           lableId++,lableContentLength,lableLeftPartLength,lableRightPartLength,lablePunctNumber,lableAttributeVector,lableLevelNumber);
                    
                    
                    ret.push_back(lf);
                    findPostIndex = msg.endIndex + 1;
                    lableStack.pop();
                }
            }
            else
            {
                findPostIndex = msg.endIndex +1;
            }
        }
    }
    
    //for(int i=0;i<ret.size();i++)
        //print_lable_feature(ret[i]);
    
    return ret;
}


//获取标签特征-----加入包含标签
/*
如果完备标签去除时，其left标签和上一个标签中间有内容，将上一个标签改为叶子标签
如果完备标签去除时，其右标签和下一个标签之间有内容，则将上一个标签改为叶子标签
*/
vector<struct lableFeature> get_content_lable_feature_new(string content){
    vector<struct lableFeature> ret;
    stack<struct lableMsg> lableStack;
    int findPostIndex = 0;
    int lableId = 0;
    lableMsg msg = getLableMsg(content, findPostIndex);
    while(msg.lableFlag == 2)
    {
        findPostIndex = msg.endIndex +1;
        msg = getLableMsg(content, findPostIndex);
    }
    lableStack.push(msg);
    findPostIndex = msg.endIndex+1;
    
    while(1)
    {
        int contentsize = content.size();
        msg = getLableMsg(content, findPostIndex);
        if(msg.lableFlag == 0)
        {
            cout<<"get content lable feature successful\n";
            break;
        }
        if(msg.lableFlag == LEFTLABLE) //左标签
        {
            if(lableStack.empty())
            {
                lableStack.push(msg);
            }
            else
            {
                lableStack.top().leafLableFlag = false;
                lableStack.push(msg);
            }
            findPostIndex = msg.endIndex +1;
        }
        else   //右标签
        {
            if(lableStack.empty())
            {
                content.erase(0,msg.endIndex+1);
                findPostIndex = 0;
                continue;
            }
            struct lableMsg topMsg = lableStack.top();
            if(topMsg.lableName == msg.lableName && topMsg.lableFlag == LEFTLABLE )
            {
                if(topMsg.leafLableFlag == false)
                {
                    content.erase(topMsg.beginIndex +1, msg.endIndex - topMsg.beginIndex + 1);
                    findPostIndex = topMsg.beginIndex;
                    lableStack.pop();
                    continue;
                }
                string lableName = msg.lableName;
                string lableContent = content.substr(topMsg.endIndex+1, msg.beginIndex -  topMsg.endIndex -1);
                if(inner_content_is_empty(lableContent))
                {
                    content.erase(topMsg.beginIndex +1, msg.endIndex - topMsg.beginIndex + 1);
                    findPostIndex = topMsg.beginIndex + 1;
                    lableStack.pop();
                }
                else
                {
                    string lableLeftPartContent = content.substr(topMsg.beginIndex,topMsg.endIndex - topMsg.beginIndex +1);
                    int lableBeginIndex = topMsg.endIndex + 1;
                    int lableEndIndex = msg.beginIndex -1;
                    // lableId ++;
                    int lableContentLength = lableContent.size();
                    int lableLeftPartLength = lableLeftPartContent.size();
                    int lableRightPartLength = msg.lableName.size() + 3;
                    int lablePunctNumber = get_punct_number(lableContent);
                    vector<string> lableAttributeVector = get_lable_attribute_vector(lableLeftPartContent);
                    int lableLevelNumber = lableStack.size();
                    struct lableFeature lf(lableName,lableContent,lableLeftPartContent,lableBeginIndex,lableEndIndex,
                                           lableId,lableContentLength,lableLeftPartLength,lableRightPartLength,lablePunctNumber,lableAttributeVector,lableLevelNumber);
                    //ret.push_back(lf);
                    findPostIndex = topMsg.beginIndex;
                    lableStack.pop();
                    
                    
                    if(findPostIndex - lableStack.top().endIndex >= 2 )
                    {
                        struct lableMsg tmp_topMsg = lableStack.top();
                        string lableName = tmp_topMsg.lableName;
                        string lableContent = content.substr(tmp_topMsg.endIndex+1, findPostIndex  -  tmp_topMsg.endIndex -1);
                        if(inner_content_is_empty(lableContent))
                        {
                            ret.push_back(lf);
                            
                            //lableStack.pop();
                            
                            cout<<"-------11111-----"<<endl;
                            cout<<"lable: "<< lf.lableContent<<endl;
                            //cout<<"content: "<< content.substr(0,findPostIndex)<<endl;
                            cout<<"-------11111----"<<endl;
                            content.erase(topMsg.beginIndex,msg.endIndex - topMsg.beginIndex +1);
                            findPostIndex = topMsg.beginIndex;
                            lableId++;
                            //continue;
                            if(!lableStack.empty())
                            {
                                int tmp_end = content.find("<",findPostIndex);
                                if(tmp_end == string::npos)
                                    continue;
                                string  leftpart_string = content.substr(findPostIndex, tmp_end - findPostIndex);
                                if(!inner_content_is_empty(leftpart_string))
                                {
                                    lableStack.top().leafLableFlag = true;
                                }
                            }
                        }
                        else
                        {
                            string lableLeftPartContent = content.substr(tmp_topMsg.beginIndex,tmp_topMsg.endIndex - tmp_topMsg.beginIndex +1);
                            int lableBeginIndex = topMsg.endIndex + 1;
                            int lableEndIndex = findPostIndex - 1;
                            // lableId ++;
                            int lableContentLength = lableContent.size();
                            int lableLeftPartLength = lableLeftPartContent.size();
                            int lableRightPartLength = lableName.size() + 3;
                            int lablePunctNumber = get_punct_number(lableContent);
                            vector<string> lableAttributeVector = get_lable_attribute_vector(lableLeftPartContent);
                            int lableLevelNumber = lableStack.size();
                            struct lableFeature lf1(lableName,lableContent,lableLeftPartContent,lableBeginIndex,lableEndIndex,
                                                   lableId,lableContentLength,lableLeftPartLength,lableRightPartLength,lablePunctNumber,lableAttributeVector,lableLevelNumber);
                            ret.push_back(lf1);
                            lf.lableLevelNumber += 1;
                            lableId += 2;
                            ret.push_back(lf);
                            content.erase(tmp_topMsg.endIndex + 1 , msg.endIndex - (tmp_topMsg.endIndex +1) +1);
                            tmp_topMsg.leafLableFlag = true;
                            
                            findPostIndex = tmp_topMsg.endIndex;
                            
                            cout<<"-------22222-----"<<endl;
                            cout<<"lable1: "<< lf1.lableContent<<endl;
                            cout<<"lable2: "<< lf.lableContent<<endl;
                            //cout<<"content: "<< content.substr(0,200)<<endl;
                            cout<<"-------22222----"<<endl;
                            cout<<endl;
                        }
                    }
                    else
                    {
                        ret.push_back(lf);
                        content.erase(topMsg.beginIndex,msg.endIndex - topMsg.beginIndex +1);
                        findPostIndex = topMsg.beginIndex;
                        lableId++;
                        
                        if(!lableStack.empty())
                        {
                            int tmp_end = content.find("<",findPostIndex);
                            if(tmp_end == string::npos)
                                continue;
                            string  leftpart_string = content.substr(findPostIndex, tmp_end - findPostIndex);
                            if(!inner_content_is_empty(leftpart_string))
                            {
                                lableStack.top().leafLableFlag = true;
                            }
                        }
                    }
                    
                }
            }
            else
            {
                findPostIndex = msg.endIndex +1;
            }
        }
    }
    
    //for(int i=0;i<ret.size();i++)
        //print_lable_feature(ret[i]);
    
    return ret;
}


//获取文本实际长度

int get_content_real_length(string str)
{
    int ret = 0;
    for(int i=0;i<str.size();i++)
    {
        if(str[i] ==' ' || str[i] == '\t' || str[i] == '\r' || str[i] =='\n' || str[i] == '|')
            continue;
        else
            ret ++;
    }
    return ret;
}
//获取标签特征-----加入包含标签
/*
 1、如果是左标签，当stack不为空，且将左标签和top标签之间内容不为空，则将其内提出，左标签坐标前移
 */
vector<struct lableFeature> get_content_lable_feature_1124(string content){
    vector<struct lableFeature> ret;
    stack<struct lableMsg> lableStack;
    string init_content = content;
    int findPostIndex = 0;
    int lableId = 0;
    lableMsg msg = getLableMsg(content, findPostIndex);
    while(msg.lableFlag == 2)
    {
        findPostIndex = msg.endIndex +1;
        msg = getLableMsg(content, findPostIndex);
    }
    lableStack.push(msg);
    findPostIndex = msg.endIndex+1;
    
    while(1)
    {
        int contentsize = content.size();
        msg = getLableMsg(content, findPostIndex);
        if(msg.lableFlag == 0)
        {
            cout<<"get content lable feature successful\n";
            break;
        }
        if(msg.lableFlag == LEFTLABLE) //左标签
        {
            if(lableStack.empty())
            {
                lableStack.push(msg);
            }
            else
            {
                
                string content_between_topLable_and_leftLable = content.substr(lableStack.top().endIndex+1, msg.beginIndex -lableStack.top().endIndex-1 );
                if(inner_content_is_empty(content_between_topLable_and_leftLable))
                {
                    //lableStack.top().leafLableFlag = false;
                    lableStack.push(msg);
                    findPostIndex = msg.endIndex +1;
                }
                else
                {
                    struct lableMsg topMsg = lableStack.top();
                    string lableName = topMsg.lableName;
                    string lableContent = content_between_topLable_and_leftLable;
                    string lableLeftPartContent = content.substr(topMsg.beginIndex,topMsg.endIndex - topMsg.beginIndex +1);
                    int lableContentLength =get_content_real_length(lableContent);
                    int lableBeginIndex = init_content.find(lableContent);
                    int lableEndIndex = lableBeginIndex + lableContent.size() - 1;
                    int lableLeftPartLength = lableLeftPartContent.size();
                    int lableRightPartLength = msg.lableName.size() + 3;
                    int lablePunctNumber = get_punct_number(lableContent);
                    vector<string> lableAttributeVector = get_lable_attribute_vector(lableLeftPartContent);
                    int lableLevelNumber = lableStack.size();
                    struct lableFeature lf(lableName,lableContent,lableLeftPartContent,lableBeginIndex,lableEndIndex,
                                           lableId++,lableContentLength,lableLeftPartLength,lableRightPartLength,lablePunctNumber,lableAttributeVector,lableLevelNumber);
                    ret.push_back(lf);
                    
                    content.erase(content.begin() + topMsg.endIndex+1,content.begin() + msg.beginIndex);
                    msg.beginIndex -= lableContent.size();
                    msg.endIndex -= lableContent.size();
                    lableStack.push(msg);
                    findPostIndex = msg.endIndex +1;
                }
            }
        }
        else   //右标签
        {
            if(lableStack.empty())
            {
                content.erase(0,msg.endIndex+1);
                findPostIndex = 0;
                continue;
            }
            struct lableMsg topMsg = lableStack.top();
            if(topMsg.lableName == msg.lableName && topMsg.lableFlag == LEFTLABLE )
            {
                string lableName = msg.lableName;
                string lableContent = content.substr(topMsg.endIndex+1, msg.beginIndex -  topMsg.endIndex -1);
                if(inner_content_is_empty(lableContent))
                {
                    content.erase(topMsg.beginIndex, msg.endIndex - topMsg.beginIndex + 1);
                    findPostIndex = topMsg.beginIndex;
                    lableStack.pop();
                }
                else
                {
                    string lableLeftPartContent = content.substr(topMsg.beginIndex,topMsg.endIndex - topMsg.beginIndex +1);
                    int lableContentLength = get_content_real_length(lableContent);
                    int lableBeginIndex = init_content.find(lableContent);
                    int lableEndIndex = lableBeginIndex + lableContent.size() - 1;
                    int lableLeftPartLength = lableLeftPartContent.size();
                    int lableRightPartLength = msg.lableName.size() + 3;
                    int lablePunctNumber = get_punct_number(lableContent);
                    vector<string> lableAttributeVector = get_lable_attribute_vector(lableLeftPartContent);
                    int lableLevelNumber = lableStack.size();
                    struct lableFeature lf(lableName,lableContent,lableLeftPartContent,lableBeginIndex,lableEndIndex,
                                           lableId++,lableContentLength,lableLeftPartLength,lableRightPartLength,lablePunctNumber,lableAttributeVector,lableLevelNumber);
                    ret.push_back(lf);
                    content.erase(content.begin() + topMsg.beginIndex,content.begin() + msg.endIndex +1);
                    findPostIndex = topMsg.beginIndex;
                    lableStack.pop();
                }
            }
            else
            {
                findPostIndex = msg.endIndex +1;
            }
        }
    }
    
    //for(int i=0;i<ret.size();i++)
    //print_lable_feature(ret[i]);
    
    return ret;
}


//打印第k簇的内容
void print_content_by_kmeansCluster(vector<struct lableFeature>lf,vector< vector<int> > cluster,vector<Feature> kCenter,int k)
{
    cout<<"－－－－－－第 " << k<<" 聚类簇中心：(";
    for(int i=0;i<kCenter[k-1].size();i++)
        cout<< kCenter[k-1][i]<<", ";
    cout<<")"<<endl;
    for(int i=0;i<cluster[k-1].size();i++)
    {
        cout<<"LableId:"<<cluster[k-1][i]<< "--size:"<<lf[cluster[k-1][i]].lableContentLength;
        cout<<"--lableName:"<<lf[cluster[k-1][i]].lableName<<"  --->"<< lf[cluster[k-1][i]].lableContent<<endl;
    }
    cout<<"－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－"<<endl;
}


//打印正文标签信息,并返回正文
string  print_page_content_by_id(vector<struct lableFeature>lf,vector<int> id)
{
    string page_text("");
    for(int i=0;i<id.size(); i++)
    {
        cout<<"LableId:"<<id[i]<< "--size:"<<lf[id[i]].lableContentLength;
        cout<<"--lableName:"<<lf[id[i]].lableName<<"  --->"<< lf[id[i]].lableContent<<endl;
        
        //cout<< lf[id[i]].lableContent[0] << "=="<<lf[id[i]].lableContent[1]<<endl;
        //cout<<"content2:"<<lf[id[i]].lableContent.substr(0,2)<<endl;
        //cout<<"content4:"<<lf[id[i]].lableContent.substr(0,4)<<endl;
        //cout<<"content6:"<<lf[id[i]].lableContent.substr(0,6)<<endl;
        if(lf[id[i]].lableContent.substr(0,2) == "\343\200")
        {
            //cout<<"yest"<<endl;
            page_text += "\n";
        }
        page_text += lf[id[i]].lableContent;
    }
    return page_text;
}


//调整正文簇内lableID
//1、同层次最多
vector<int> get_page_text_cluster_id_after_produce(vector<struct lableFeature>lf,vector< vector<int> > cluster,int k)
{
    vector<int> ic = cluster[k-1];
    vector<int> ret;
    //if(ic.size() <= 1)
      //  return ret;
    //查询文本簇中出现次数最多的标签名,定次数最多的标签名为正文标签名
    map<string,vector<int> > lable_map;
    //map<string,int> lable_level_map;
    string lableName = lf[ic[0]].lableName;
    for(int i=0;i<ic.size();i++)
    {
        string lableName_tmp = lf[ic[i]].lableName;
        lable_map[lableName_tmp].push_back(lf[ic[i]].lableLevelNumber);
        if(lable_map[lableName].size() < lable_map[lableName_tmp].size())
            lableName = lableName_tmp;
        //lable_level_map[lableName] = lf[ic[i]].lableLevelNumber;
    }
    /*
    //确定次数最多的标签名为正文标签名
    map<string,vector<int> >::iterator iter = lable_map.begin();
    //string lableName =iter->first;
    for(;iter!=lable_map.end();iter++)
    {
        if((iter->second).size() > lable_map[lableName].size())
        {
            lableName = iter->first;
        }
    }
    */
    int min_id=-1,max_id =-1;
    bool flag_min = true;
    //用于确定正文在第几层
    vector<int> lableName_level_vect = lable_map[lableName];
    int text_lable_level = 0;
    int text_lable_level_num = 0;
    //正文标签每个层次对应的数量map
    map<int,int> lableName_level_time_map;
    for(int i=0;i<lableName_level_vect.size();i++)
    {
        lableName_level_time_map[lableName_level_vect[i]] ++;
        if(lableName_level_time_map[lableName_level_vect[i]]  > text_lable_level_num)
        {
            text_lable_level = lableName_level_vect[i];
            text_lable_level_num = lableName_level_time_map[lableName_level_vect[i]] ;
        }
    }
    
    
    map<int,int>  lableLevelMap;
    for(int i=0;i<ic.size();i++)
    {
        string lable = lf[ic[i]].lableName;
        int lableLevel = lf[ic[i]].lableLevelNumber;
        if(lable == lableName && lableLevel == text_lable_level)
        {
            if(flag_min)
            {
                min_id = i;
                flag_min = false;
            }
            ///lableLevelMap[lableLevel] ++;
            max_id = i;
        }
    }
    cout << "init min_id: "<<min_id<< "    max_id:"<<max_id<<endl;
    /*

    map<int,int>::iterator iter_level = lableLevelMap.begin();
    text_lable_level = iter_level->first;
    
    for(;iter_level!= lableLevelMap.end(); iter_level++)
    {
        if(iter_level->second > lableLevelMap[text_lable_level])
            text_lable_level = iter_level->first;
    }
    
    */
    
    int beginID = ic[min_id];
    int endID = ic[ max_id ];
    
    cout << "init beginID: "<<beginID<< "    endID:"<<endID<<endl;
    //正文标签层次
    //int lable_level = lable_level_map[lableName];
    
    for(int i=beginID-1;i>=0; i--)
    {
        cout<<"lfi name:"<<lf[i].lableName<<endl;
        if(lf[i].lableName == lableName)
        {
            beginID = i;
            continue;
        }
        if(lf[i].lableName == "h1" || lf[i].lableName == "h2" || lf[i].lableName == "h3"||
           lf[i].lableName == "h4" ||lf[i].lableName == "h5" || lf[i].lableName == "h6" ||
           lf[i].lableName == "h")
        {
            beginID = i+1;
            break;
        }
        if(lf[i].lableLevelNumber < text_lable_level || lf[i].lableLevelNumber > text_lable_level +1)
        {
            beginID = i+1;
            break;
        }
        if(lf[i].lableName != lableName && lf[i].lableName != "strong")
        {
            beginID = i+1;
            break;
        }
    }
    
    for(int i= endID+1; i<lf.size();i++)
    {
        if(lf[i].lableName == lableName)
        {
            endID = i;
            continue;
        }
        
        if(lf[i].lableName == "h1" || lf[i].lableName == "h2" || lf[i].lableName == "h3"||
           lf[i].lableName == "h4" ||lf[i].lableName == "h5" || lf[i].lableName == "h6" ||
           lf[i].lableName == "h")
        {
            endID = i-1;
            break;
        }

        
        if(lf[i].lableLevelNumber < text_lable_level)
        {
            endID = i-1;
            break;
        }
        if(lf[i].lableName != lableName && lf[i].lableName != "strong")
        {
            endID = i-1;
            break;
        }
    }
    
    for(int i=beginID;i<=endID;i++)
        ret.push_back(i);
    
    return ret;
}

int save_content2file(string file_path_name,string content)
{
    fstream cfile;
    cfile.open(file_path_name.c_str(),ios_base::out|ios::out );
    if((char*)&cfile == NULL)
    {
        cout<< "open clear file error: "<< strerror(errno)<<endl;
        return 0;
    }
    cfile << content;
    cfile.close();
    return 1;
    
}

int main_loop_url(string url,int kcluster = 3)
{
    //请求爬去html网页
    getWebPageClass gwp;
    string utf_8_content = gwp.get_html_content_by_url(url);
    
    //string utf_8_content = gwp.get_web_page_by_read_XML_file("/Users/pc/qqfile.html");
    //string utf_8_content = gwp.get_web_page_by_read_XML_file("/Users/pc/sohufile.html");
    string web_page_title = gwp.get_web_page_title();
    
    if(utf_8_content.size() <= 0)
    {
        cout<<"get url content error\n";
        return 0;
    }
    
    //网页html预处理
    contentProcess cp;
    cp.parse_content_test(utf_8_content);
    
    cout<<"content size after process : "<<utf_8_content.size()<<endl;
    
    int ret = save_content2file("/Users/pc/get_clear_page.html", utf_8_content);
    if(ret == 1)
        cout<< "save clear  file ok"<<endl;
    else
        cout<< "save clear  file error"<<endl;
    
    //vector<struct lableFeature> lable_feature = get_content_lable_feature(utf_8_content);
    
    
    vector<struct lableFeature> lable_feature = get_content_lable_feature_1124(utf_8_content);
    for(int i=0;i<lable_feature.size();i++)
        print_lable_feature(lable_feature[i]);
    
    vector<Feature> feature_vector = get_feature_vector(lable_feature);
    
    int KC =kcluster;
    try
    {
        kmeansCluster kmeans(feature_vector,KC);
        kmeans.kmeans_function();
        for(int i=1;i<=KC;i++)
            kmeans.print_kmeans_cluster(i);
        
        vector< vector<int> > lableId_cluster = kmeans.get_lableId_from_cluster();
        vector<Feature> kCenter = kmeans.get_kCenter();
        vector< vector<Feature> > cluster = kmeans.get_cluster();
        
        for(int i=1;i<=KC; i++)
        {
            print_content_by_kmeansCluster(lable_feature, lableId_cluster, kCenter,i);
        }
        
        int page_text_cluster_k = kmeans.get_page_text_cluster_k();
        
        vector<int> page_text_id_vector =  get_page_text_cluster_id_after_produce(lable_feature, lableId_cluster, page_text_cluster_k);
        
        if(page_text_id_vector.size() == 0)
        {
            cout<<" 这个网页非新闻文本"<<endl;
        }
        else
        {
            for(int i=0;i<page_text_id_vector.size();i++)
            {
                cout<<page_text_id_vector[i]<<" ";
            }
            cout<<endl;
            cout<<"\n－－－－－－－－－－－－正文标签信息开始－－－－－－－－－－－－－－－－－－－－－－－－－－－\n";
            cout<<"标题："<<web_page_title<<endl;
            string page_text = print_page_content_by_id(lable_feature, page_text_id_vector);
            cout<<"－－－－－－－－－－－－正文标签信息结束－－－－－－－－－－－－－－－－－－－－－－－－－－－\n";
            
            cout<<"\n\n－－－－－－－－－－－－网页正文－－－－－－－－－－－－－－－－－－－－－－－－－－－\n";
            cout<<"              新闻标题： "<<web_page_title<<endl;
            cout<<"\n" << page_text<<endl;
            cout<<"\n\n－－－－－－－－－－－－网页正文－－－－－－－－－－－－－－－－－－－－－－－－－－－\n\n";
            
            string tmp("              新闻标题： ");
            page_text = tmp + web_page_title + page_text;
            ret = save_content2file("/Users/pc/get_clear_page.html", page_text);
            if(ret == 1)
                cout<< "save page text  file ok"<<endl;
            else
                cout<< "save page text  file error"<<endl;
        }
    }
    catch(int e)
    {
        if(e == 1)
            cout<<"K must big than 0  or  featureVector  is empty"<<endl;
        else if(e==2)
        {
            cout<<"No need to cluter,because K is small than featureCluster number "<<endl;
            
        }
        else if(e == 3)
            cout << "this page only has less two lables,maybe it's not a new page\n"<<endl;
    }
    
    return 1;
}

int main_loop_file(string filepath,int kcluster = 3)
{
    //请求爬去html网页
    getWebPageClass gwp;
    string utf_8_content = gwp.get_web_page_by_read_XML_file(filepath);
    //string utf_8_content = gwp.get_web_page_by_read_XML_file("/Users/pc/sohufile.html");
    string web_page_title = gwp.get_web_page_title();
    
    if(utf_8_content.size() <= 0)
    {
        cout<<"get url content error\n";
        return 0;
    }
    
    //网页html预处理
    contentProcess cp;
    cp.parse_content_test(utf_8_content);
    
    cout<<"content size after process : "<<utf_8_content.size()<<endl;
    
    int ret = save_content2file("/Users/pc/get_clear_page.html", utf_8_content);
    if(ret == 1)
        cout<< "save clear  file ok"<<endl;
    else
        cout<< "save clear  file error"<<endl;
    
    //vector<struct lableFeature> lable_feature = get_content_lable_feature(utf_8_content);
    
    
    vector<struct lableFeature> lable_feature = get_content_lable_feature_1124(utf_8_content);
    for(int i=0;i<lable_feature.size();i++)
        print_lable_feature(lable_feature[i]);
    
    vector<Feature> feature_vector = get_feature_vector(lable_feature);
    
    int KC =kcluster;
    try
    {
        kmeansCluster kmeans(feature_vector,KC);
        kmeans.kmeans_function();
        for(int i=1;i<=KC;i++)
            kmeans.print_kmeans_cluster(i);
        
        vector< vector<int> > lableId_cluster = kmeans.get_lableId_from_cluster();
        vector<Feature> kCenter = kmeans.get_kCenter();
        vector< vector<Feature> > cluster = kmeans.get_cluster();
        
        for(int i=1;i<=KC; i++)
        {
            print_content_by_kmeansCluster(lable_feature, lableId_cluster, kCenter,i);
        }
        
        int page_text_cluster_k = kmeans.get_page_text_cluster_k();
        
        vector<int> page_text_id_vector =  get_page_text_cluster_id_after_produce(lable_feature, lableId_cluster, page_text_cluster_k);
        
        if(page_text_id_vector.size() == 0)
        {
            cout<<" 这个网页非新闻文本"<<endl;
        }
        else
        {
            for(int i=0;i<page_text_id_vector.size();i++)
            {
                cout<<page_text_id_vector[i]<<" ";
            }
            cout<<endl;
            cout<<"\n－－－－－－－－－－－－正文标签信息开始－－－－－－－－－－－－－－－－－－－－－－－－－－－\n";
            cout<<"标题："<<web_page_title<<endl;
            string page_text = print_page_content_by_id(lable_feature, page_text_id_vector);
            cout<<"－－－－－－－－－－－－正文标签信息结束－－－－－－－－－－－－－－－－－－－－－－－－－－－\n";
            
            cout<<"\n\n－－－－－－－－－－－－网页正文－－－－－－－－－－－－－－－－－－－－－－－－－－－\n";
            cout<<"              新闻标题： "<<web_page_title<<endl;
            cout<<"\n" << page_text<<endl;
            cout<<"\n\n－－－－－－－－－－－－网页正文－－－－－－－－－－－－－－－－－－－－－－－－－－－\n\n";
            
            string tmp("              新闻标题： ");
            page_text = tmp + web_page_title +"\n\n" + page_text;
            ret = save_content2file("/Users/pc/get_clear_page.html", page_text);
            if(ret == 1)
                cout<< "save page text  file ok"<<endl;
            else
                cout<< "save page text  file error"<<endl;
        }
    }
    catch(int e)
    {
        if(e == 1)
            cout<<"K must big than 0  or  featureVector  is empty"<<endl;
        else if(e==2)
        {
            cout<<"No need to cluter,because K is small than featureCluster number "<<endl;
            
        }
        else if(e == 3)
            cout << "this page only has less two lables,maybe it's not a new page\n"<<endl;
    }
    
    return 1;
}


vector<string> get_test_url_vect()
{
    vector<string> urlVect;
    //新浪网
    string url0("http://sports.sina.com.cn/nba/2015-04-24/12277585850.shtml");
    //string url00("http://sports.sina.com.cn/basketball/nba/2015-11-23/doc-ifxkxfvn8964499.shtml");
    string url1("http://sports.sina.com.cn/basketball/nba/2015-11-13/doc-ifxksqis4781087.shtml");
    string url2("http://finance.sina.com.cn/chanjing/gsnews/20151030/010123623334.shtml?cre=financepagepc&mod=f&loc=3&r=h&rfunc=6");  //两桶油
    urlVect.push_back(url0);
    urlVect.push_back(url1);
    urlVect.push_back(url2);
    //百度新闻
    string url3("http://www.baidu.com/s?ie=utf-8&f=8&rsv_bp=1&tn=ubuntuu_cb&wd=%E7%BD%91%E7%BB%9C%E7%88%AC%E8%99%AB%20%20%E6%96%87%E6%9C%AC%E4%B9%B1%E7%A0%81%20C%2B%2B&rsv_pq=f7b9acd800000ed2&rsv_t=e7254jg%2BHKV2oGILj6QU1qUUY8RmgxC18gdzcLQ5%2F%2F4MrxgSJYq5FZqJzFVCrTTvOA&rsv_enter=1&rsv_sug3=27&rsv_sug1=5&rsv_sug2=0&inputT=9319&rsv_sug4=10773");
    urlVect.push_back(url3);
    //新华网
    string url4("http://news.xinhuanet.com/politics/2015-11/09/c_1117086503.htm");
    urlVect.push_back(url4);
    //cn博客
    string url5("http://www.cnblogs.com/hoys/archive/2011/08/11/2134653.html");
    urlVect.push_back(url5);
    //博客网博客网页
    string url6("view-source:http://www.cnblogs.com/daniagger/archive/2012/06/08/2541506.html");
    urlVect.push_back(url6);
    //直播吧新闻
    string url7("http://news.zhibo8.cc/zuqiu/2015-11-12/564489ba44a33.htm");
    urlVect.push_back(url7);
    //搜狐新闻  完全正确
    string url8("http://mt.sohu.com/20151111/n426103878.shtml");
    string url9("http://mil.sohu.com/20151113/n426330845_1.shtml");
    urlVect.push_back(url8);
    urlVect.push_back(url9);
    //网易新闻
    string url10("http://news.163.com/15/1113/09/B89TC4RP0001124J.html");
    string url11("http://news.163.com/15/1117/14/B8KNNLR8000146BE.html");
    urlVect.push_back(url10);
    urlVect.push_back(url11);
    //电子科大校网
    string url12("http://www.new1.uestc.edu.cn/?n=UestcNews.Front.Document.ArticlePage&Id=51659");
    urlVect.push_back(url12);
    //腾讯
    string url13("http://news.qq.com/a/20151123/047720.htm");
    urlVect.push_back(url13);
    //12
    //qqfile、
    //直播吧  正文长短句差距很明显，容易把短句子和长句子分开     ＝＝＝》 人为动态调控：连续
    //搜狐还行
    
    //微信
    string url14("http://mp.weixin.qq.com/s?__biz=MzI5MDA2MjgxNg==&mid=400896281&idx=1&sn=bffd0ea81bf14235e2e1c844a45c629c&scene=0&key=d72a47206eca0ea917efb4f6e73c3f35fc7f7a3c49448b58ce1d23298faf202cb35a7912d42b193c69e1ed2e00c795db&ascene=0&uin=MjE4NTYwODM2Mg%3D%3D&devicetype=iMac+MacBookAir5%2C2+OSX+OSX+10.10.5+build(14F27)&version=11020201&pass_ticket=810kuNxs9pVZVGTy5vAw6NnM2JhyeXrg6c%2BaMvnMIr0%2F9g68s9wrZmVhLvRIQ4XG");
    //七月算法网站
    string url15("http://ask.julyedu.com/question/787");
    urlVect.push_back(url15);
    //凤凰
    string url16("http://news.ifeng.com/a/20151123/46355849_0.shtml");
    urlVect.push_back(url16);
    //央视网
    string url17("http://news.cntv.cn/2015/11/23/ARTI1448281298076767.shtml");
    urlVect.push_back(url17);
    //网信网
    string url18("http://www.cac.gov.cn/2015-11/16/c_1117152499.htm");
    urlVect.push_back(url18);
    //环球网
    string url19("http://world.huanqiu.com/exclusive/2015-11/8022073.html");
    urlVect.push_back(url19);
    //中华军事
    string url20("http://military.china.com/important/11132797/20151123/20804396.html");
    urlVect.push_back(url20);
    //米尔军事
    string url21("http://club.miercn.com/201511/thread_588085_1.html");
    urlVect.push_back(url21);
    //铁血军事
    string url22("http://bbs.tiexue.net/post2_10361709_1.html");
    urlVect.push_back(url22);
    //强国网
    string url23("http://www.cnqiang.com/junshi/junqing/201511/01256156.html");
    string url24("http://www.cnqiang.com/junshi/zhanlue/201511/01256154.html");
    urlVect.push_back(url23);
    urlVect.push_back(url24);
    return urlVect;
}
int main()
{
    vector<string> urlVect = get_test_url_vect();
    string loop_url("url");
    int loop_time = 0;
    while(loop_time < urlVect.size())
    {
        main_loop_url(urlVect[loop_time]);
        loop_time ++;
    }
    main_loop_url(urlVect[11]);
    main_loop_file("/Users/pc/qqfile.html");
    
    //xmlParser("/Users/pc/get_clear_page.html");
    return 1;
}


//question1: 页面抓取不完

#define OUTLEN 255

