#include <iostream>
#include <string>
#include <set>
#include <optional>
#include <ctime>
#include <sstream>
#include <vector>
#include <map>
#include <assert.h>
#include <fstream>
#include <stack>

#define debugout(x) std::cout<<#x<<": "<<x<<std::endl

//获取系统时间
static std::string getTimeNow()
{
    time_t rawTime;
    time(&rawTime);
    auto lt = localtime(&rawTime);
    std::stringstream ss;
    ss << lt->tm_year + 1900 << "-" << lt->tm_mon + 1 << "-" << lt->tm_mday
    << "&" << lt->tm_hour << ":" << lt->tm_min << ":" << lt->tm_sec;
    return ss.str();
}

struct DirNode
{
    //如果是目录，name为目录名；如果是文件，name是文件名后加*
    std::string name="";
    //是否是目录
    bool isDir=false;
    //上次修改时间
    std::string lastModified=getTimeNow();
    //权限
    std::string permission="rwx";
    //子目录
    std::map<std::string,DirNode> children;
    //父节点
    DirNode* father=nullptr;
};

class DirectoryTree
{
    using Node=DirNode;
private:
    Node _root;
    Node* _curNode=nullptr;

public:
    DirectoryTree()
    {
        _root.isDir=true;
        _curNode=&_root;
    }
    ~DirectoryTree(){};

private:
    //递归打印working directory
    void _recursivePwd(Node* curNode) const;
    //在终端抛出异常
    void _throwException(const std::string& op,const std::string& exception) const;
    //递归保存树结构
    void _recursiveSave(const Node* curNode,std::ofstream& ofs) const;
    //递归读取树结构。读取到左括号进入递归，读取到未匹配的右括号视为结束。
    Node _recursiveLoad(std::stringstream& ss);
    //递归为当前节点下所有子节点记录父节点
    void _recursivePointerMaintain(Node& curNode);
    //将路径字符串转换为路径上的节点
    std::vector<std::string> _splitPath(const std::string& path) const;
    //一旦出现找不到的直接返回
    bool _getNodeFromRoot(const std::vector<std::string>& paths,Node*& ret);
    //递归保存当前节点路径
    void _recursiveSaveWD(Node* curNode,std::ofstream& ofs) const;

public:
    //打印工作目录
    void pwd() const;
    //输出working dir下的所有文件内容
    void ls(const std::string& params="") const;
    //创建目录
    void mkdir(const std::string& name);
    //创建文件，或者改变已经存在的文件的最后修改时间
    void touch(const std::string& name);
    //删除文件/目录
    void rm(const std::string& name);
    //更改工作目录
    void cd(const std::string& name);
    //保存目录结构
    void save(const std::string& path) const;
    //读取目录结构
    void load(const std::string& path);
};