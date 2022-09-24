#include <xsystem/xsystem.h>
#if __linux__
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#elif _WIN32
#include <direct.h>
#endif

void xsystem::mkdirs(std::string path){
    #if __linux__
        int ret=0;
        DIR * mydir = NULL;
        if ((mydir = opendir(path.c_str())) == NULL){
            ret = mkdir(path.c_str(), 0755);
            if (ret != 0){PRINT_ERROR};
        }else{
            printf("%s exist!/n", path.c_str());
        }
    #elif _WIN32
        
        _mkdir(path.c_str());
    #endif
}