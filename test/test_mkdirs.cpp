#include <xsystem/xsystem.h>
int main(int argc, char const *argv[])
{
    #if __linux__
        xsystem::mkdirs("./linux");
    #elif _WIN32
        xsystem::mkdirs("./windows");
    #endif
    return 0;
}
