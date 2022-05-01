#include "config.h"
#include "DirectoryTree.h"

DirectoryTree t;

static bool parseCmd_CLI(std::stringstream ss)
{
    std::string cmd,params;
    ss>>cmd>>params;
    if(cmd=="quit")
        return false;
    if(cmd=="pwd")
        t.pwd();
    else if(cmd=="ls")
        t.ls(params);
    else if(cmd=="mkdir")
        t.mkdir(params);
    else if(cmd=="touch")
        t.touch(params);
    else if(cmd=="rm")
        t.rm(params);
    else if(cmd=="cd")
        t.cd(params);
    else if(cmd=="save")
        t.save(params);
    else if(cmd=="load")
        t.load(params);
    return true;
}

static bool parseCmd_Script(std::stringstream ss)
{
    std::string cmd,params;
    ss>>cmd>>params;
    // std::cout<<cmd<<std::endl;
    if(cmd=="quit")
        return false;
    else if(cmd=="dir")
        t.ls();
    else if(cmd=="cd")
    {
        if(params.empty())
            t.pwd();
        else
            t.cd(params);
    }
    else if(cmd=="mkdir")
        t.mkdir(params);
    else if(cmd=="mkfile")
        t.touch(params);
    else if(cmd=="delete")
        t.rm(params);
    else if(cmd=="save")
        t.save(params);
    else if(cmd=="load")
        t.load(params);
    return true;
}

const int script_index=7;

int main()
{
    std::string cmd;

//命令行交互
#ifdef CLI
    std::cout<<"> ";
    getline(std::cin,cmd);
    while(parseCmd_CLI(std::stringstream(cmd)))
    {
        std::cout<<"> ";
        getline(std::cin,cmd);
    }
#endif

//文件脚本
#ifdef SCRIPT
    std::stringstream ssIn,ssOut;
    ssIn<<"../data/input/in"<<script_index<<".txt";
    std::ifstream ifs(ssIn.str());
    ssOut<<"../data/myoutput/out"<<script_index<<".txt";
    freopen(ssOut.str().c_str(),"w",stdout);
    assert(ifs.is_open());
    getline(ifs,cmd);
    while(parseCmd_Script(std::stringstream(cmd)))
    {
        getline(ifs,cmd);
        // std::cout<<cmd<<std::endl;
    }
#endif

    return 0;
}