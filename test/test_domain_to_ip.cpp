#include <xsystem/xsystem.h>

int main(int argc, char const *argv[])
{
    std::cout << xsystem::net::domain_to_ip("www.baidu.com") << std::endl;
    /* code */
    return 0;
}
