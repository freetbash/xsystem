#include <xsystem/xsystem.h>

void xsystem::cd(std::string path){
    #if __linux__
        if(chdir(path.c_str())<0){
            PRINT_ERROR
        }
    #elif _WIN32
        SetCurrentDirectory(SetCurrentDirectory(path.c_str()););
    #endif
}