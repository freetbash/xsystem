#include <xsystem/xsystem.h>
#include <iostream>
int main(){
    xsystem::net::FSocket f;
    f.Bind("127.0.0.1",3145);
    f.Listen(5);
    auto m =f.Accept();
    auto c = m.Recv();
    std::cout << c << std::endl;
    m.Send("HTTP/1.1 200 Ok\r\n\r\n<h1>ADS</h1>");
    return 0;
}