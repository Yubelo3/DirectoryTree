## RUN
mkdir build  
cd build  
cmake ..  
make  

## TEST
在config.h中打开#define SCRIPT并关闭#define CLI，将测试文件夹data放在工程根目录，重新build并运行。  
