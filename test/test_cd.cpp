#include <xsystem/xsystem.h>
#include <iostream>
int main(int argc, char const *argv[])
{
    xsystem::cd("..");
    #if __linux__
        auto c= xsystem::shell("ls");
    #elif _WIN32
        auto c= xsystem::shell("dir");
    #endif
    std::cout << c << std::endl;
    return 0;
}
