#include <xsystem/xsystem.h>

void xsystem::rm(std::string path){
    #if __linux__
        xsystem::shell("rm -rf "+path);
    #elif _WIN32
        xsystem::shell("del /F /Q "+path);
    #endif
}