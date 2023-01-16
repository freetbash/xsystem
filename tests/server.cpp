#include "../xsystem.hpp"
using xsystem::net::Socket;
#include <iostream>
using std::cin,std::cout;

int main(){
  Socket::Init();
  
  Socket s(AF_INET, SOCK_STREAM, 0);
  s.Bind("192.168.0.14", 3147);
  s.Listen(5);
  
  auto c = s.Accept();
  
  char buf[1024]="Hello!";
  c.Send(buf,1024,0);
  c.Recv(buf,1024,0);
  cout << buf << endl;
  
  c.Close();
  s.Close();
  Socket::Exit();
  return 0;
}
