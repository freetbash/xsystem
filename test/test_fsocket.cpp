#include <xsystem/xsystem.h>
#include <iostream>
int main(){
    net::FSocket f("127.0.0.1",3145);
    f.listen(5);
    auto m =f.accept();
    auto c = m.recv();
    std::cout << c << std::endl;
    m.send("HTTP/1.1 200 Ok\r\n\r\n<h1>ADS</h1>");
    return 0;
}