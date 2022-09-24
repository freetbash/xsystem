#include <xsystem/xsystem.h>


std::string xsystem::pwd(){
    #if __linux__
        return getcwd(NULL,0);
    #elif _WIN32
        char path[512];
        GetCurrentDirectory(sizeof(path),path);
        return path;
    #endif
}