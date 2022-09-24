#include <xsystem/xsystem.h>

std::string xsystem::get_os(){
    #if __linux__
        return "Linux";
    #elif _WIN32
        return "Windows";
    #endif
}