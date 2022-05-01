#include "DirectoryTree.h"
#include "config.h"

#define debugout(x) std::cout<<#x<<": "<<x<<std::endl

//============================private=======================================
//递归打印working directory
void DirectoryTree::_recursivePwd(Node* curNode) const
{
    if(curNode->father)
        _recursivePwd(curNode->father);
    if(curNode->father)
        std::cout<<"/";
    std::cout<<curNode->name;
}
//在终端抛出异常
void DirectoryTree::_throwException(const std::string& op,const std::string& exception) const
{
#ifdef CLI
    std::cout<<op<<": "<<exception<<std::endl;
#endif
}
//递归保存树结构
void DirectoryTree::_recursiveSave(const Node* curNode,std::ofstream& ofs) const
{
    ofs<<curNode->name<<" "<<curNode->lastModified<<" ";
    if(curNode->isDir)
    {
        ofs<<"( ";
        for(auto&[key,value]:curNode->children)
        {
            ofs<<"- ";
            _recursiveSave(&value,ofs);
        }
        ofs<<") ";
    }
}
//递归读取树结构。读取到左括号进入递归，读取到未匹配的右括号视为结束。
DirNode DirectoryTree::_recursiveLoad(std::stringstream& ss)
{
    Node node;
    ss>>node.name>>node.lastModified;
    //如果是目录，递归
    if(node.name.back()!='*')
    {
        node.isDir=true;
        //首先会读入一个左括号
        std::string leftBrace,nextChild;
        ss>>leftBrace>>nextChild;
        assert(leftBrace=="(");
        while(nextChild=="-")
        {
            Node child=_recursiveLoad(ss);
            //不能在这里记录父节点！
            node.children.insert({child.name,std::move(child)});
            ss>>nextChild;
        }
    }
    else
        node.isDir=false;
    return node;
}
//递归为当前节点下所有子节点记录父节点
void DirectoryTree::_recursivePointerMaintain(Node& curNode)
{
    for(auto&[name,childNode]:curNode.children)
    {
        childNode.father=&curNode;
        if(childNode.isDir)
            _recursivePointerMaintain(childNode);
    }
}
//将路径字符串转换为路径上的节点
std::vector<std::string> DirectoryTree::_splitPath(const std::string& path) const
{
    std::stringstream ss(path);
    std::string read;
    std::vector<std::string> ret;
    while(getline(ss,read,'/'))
        ret.push_back(read);
    return ret;
}
//一旦出现找不到的直接返回
bool DirectoryTree::_getNodeFromRoot(const std::vector<std::string>& paths,Node*& ret)
{
    Node* curNode=&_root;
    for(int i=1;i<paths.size();i++)
    {
        auto nodeIter=curNode->children.find(paths[i]);
        //找不到，返回
        if(nodeIter==curNode->children.end())
            return false;
        Node* nextNode=&nodeIter->second;
        //找到文件，返回
        if(!nextNode->isDir)
            return false;
        curNode=nextNode;
    }
    ret=curNode;
    return true;
}
//递归保存当前节点路径
void DirectoryTree::_recursiveSaveWD(Node* curNode,std::ofstream& ofs) const
{
    if(curNode->father)
        _recursiveSaveWD(curNode->father,ofs);
    ofs<<curNode->name<<'/';
}


//============================public========================================
//打印工作目录
void DirectoryTree::pwd() const
{
    if(&_root==_curNode)
    {
        std::cout<<"/"<<std::endl;
        return;
    }
    _recursivePwd(_curNode);
    std::cout<<std::endl;
}
//输出working dir下的所有文件内容
void DirectoryTree::ls(const std::string& params) const
{
    bool l=false;
    for(char c:params)
        if(c=='l')
            l=true;
    for(auto&[key,value]:_curNode->children)
        if(!value.isDir)
        {
            std::cout<<key;
            if(l)
                std::cout<<" "<<value.lastModified<<" "<<value.permission;
            std::cout<<std::endl;
        }
    for(auto&[key,value]:_curNode->children)
        if(value.isDir)
        {
            std::cout<<key;
            if(l)
                std::cout<<" "<<value.lastModified<<" "<<value.permission;
            std::cout<<std::endl;
        }
}
//创建目录
void DirectoryTree::mkdir(const std::string& name)
{
    //要创建文件夹的上级目录
    Node* dstDir=nullptr;
    //要创建的目录名
    std::string newDirName;
    //绝对路径
    if(name[0]=='/')
    {
        std::vector<std::string> paths=_splitPath(name);
        newDirName=std::move(paths.back());
        paths.pop_back();
        if(!_getNodeFromRoot(paths,dstDir))
        {
            _throwException("rm "+name,"No such file or directory");
            return;
        }
    }
    //相对路径
    else
    {
        dstDir=_curNode;
        newDirName=name;
    }
    //如果已经存在，什么也不做
    if(dstDir->children.find(newDirName)!=dstDir->children.end())
        return;
    DirNode newNode;
    newNode.isDir=true;
    newNode.name=name;
    newNode.father=_curNode;
    dstDir->children[name]=std::move(newNode);
}
//创建文件，或者改变已经存在的文件的最后修改时间
void DirectoryTree::touch(const std::string& name)
{
    //要创建的文件的上级目录
    Node* dstDir=nullptr;
    //新文件的文件名
    std::string newFileName;
    //绝对路径
    if(name[0]=='/')
    {
        std::vector<std::string> paths=_splitPath(name);
        newFileName=std::move(paths.back())+"*";
        paths.pop_back();
        if(!_getNodeFromRoot(paths,dstDir)||!dstDir->isDir)
        {
            _throwException("touch "+name,"No such file or directory");
            return;
        }
    }
    //相对路径
    else
    {
        newFileName=name+"*";
        dstDir=_curNode;
    }
    assert(dstDir);
    auto nodeIter=dstDir->children.find(newFileName);
    //如果不存在这个文件，创建
    if(nodeIter==dstDir->children.end())
    {
        Node newNode;
        newNode.isDir=false;
        newNode.father=dstDir;
        newNode.name=newFileName;
        dstDir->children.insert({newFileName,newNode});
    }
    //如果存在这个文件，改变最后修改时间
    else
        nodeIter->second.lastModified=getTimeNow();
}
//删除文件/目录
void DirectoryTree::rm(const std::string& name)
{
    if(name=="")
    {
        _throwException("rm "+name,"Cannot delete root directory");
        return;
    }
    //绝对路径
    if(name[0]=='/')
    {
        std::vector<std::string> paths=_splitPath(name);
        Node* dstDir;
        if(!_getNodeFromRoot(paths,dstDir))
        {
            _throwException("rm "+name,"No such file or directory");
            return;
        }
        //删掉这个节点
        dstDir->father->children.erase(paths.back());
    }
    //相对路径
    auto nodeIterDir=_curNode->children.find(name);
    auto nodeIterFile=_curNode->children.find(name+"*");
    if(nodeIterDir==_curNode->children.end()&&nodeIterFile==_curNode->children.end())
    {
        _throwException("rm "+name,"No such file or directory");
        return;
    }
    if(nodeIterDir!=_curNode->children.end())
        _curNode->children.erase(nodeIterDir);
    else
        _curNode->children.erase(nodeIterFile);
}
//更改工作目录
void DirectoryTree::cd(const std::string& name)
{
    //返回根目录
    if(name=="")
    {
        _curNode=&_root;
        return;
    }
    //返回上级
    if(name=="..")
    {
        if(!_curNode->father)
        {
            _throwException("cd "+name,"Working on root directory");
            return;
        }
        _curNode=_curNode->father;
        return;
    }
    //绝对路径
    if(name[0]=='/')
    {
        std::vector<std::string> paths=_splitPath(name);
        Node* dstDir=nullptr;
        if(!_getNodeFromRoot(paths,dstDir))
        {
            _throwException("cd "+name,"No such directory");
            return;
        }
        _curNode=dstDir;
        return;
    }
    //相对路径
    auto nodeIter=_curNode->children.find(name);
    if(nodeIter==_curNode->children.end())
    {
        _throwException("cd "+name,"No such directory");
        return;
    }
    Node* nextNode=&nodeIter->second;
    if(!nextNode->isDir)
    {
        _throwException("cd "+name,"Not such directory");
        return;
    }
    _curNode=nextNode;
}
//保存目录结构
void DirectoryTree::save(const std::string& path) const
{
    std::ofstream ofs(path);
    _recursiveSaveWD(_curNode,ofs);
    ofs<<' ';
    if(!ofs.is_open())
    {
        _throwException("save "+path,"Failed to open file");
        return;
    }
    ofs<<"ROOT";
    _recursiveSave(&_root,ofs);
    ofs<<") ";
    ofs.close();
    _throwException("save "+path,"success");
}
//读取目录结构
void DirectoryTree::load(const std::string& path)
{
    _root.children.clear();
    std::ifstream ifs(path);
    if(!ifs.is_open())
    {
        _throwException("load "+path,"Failed to open file");
        return;
    }
    std::string dirStruct;
    getline(ifs,dirStruct);
    std::stringstream ss(dirStruct);
    std::string wd;
    ss>>wd;
    _root=_recursiveLoad(ss);
    _root.name="";
    //递归记录每个节点的父节点
    _recursivePointerMaintain(_root);
    cd(wd);
}