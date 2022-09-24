#ifndef CROSS_SYSTEM
#define CROSS_SYSTEM
// -lws2_32 -liphlpapi
#include <string>
#include <string.h>
#include <stdio.h>
#include <iostream>
#if __linux__
#include <unistd.h>


#elif _WIN32
#include <Windows.h>
#include <Winsock2.h>
#pragma comment(lib,"ws2_32.lib")

#endif
#define PRINT_ERROR printf("%s:%d, %s\n",__FILE__,__LINE__,strerror(errno));







// system
namespace xsystem{
    std::string shell(std::string cmd);
    void cd(std::string path);
    std::string pwd();
    std::string get_hostname();
    std::string get_os();
    std::string get_mac();
    void mkdirs(std::string path);
    // rm
    // isdir
    // file_exist
    // dir_exist

};














// net
#if __linux__
#include <sys/socket.h>
#include <netinet/in.h>
#elif _WIN32
#include <winsock.h>
#pragma comment(lib,"ws2_32.lib")
#endif
namespace net{
    class FSocket {
    public:
        class address{
            public:
                sockaddr_in _socket_addr;
                address();
                address(std::string const& str_ip, uint16_t port);
                void set(std::string const& str_ip, uint16_t port);
                sockaddr const* socket_addr();
                size_t size();
        };

        uint32_t sfd; 
        address addr;
        FSocket();
        FSocket(std::string const& str_ip, uint16_t port);
        ~FSocket();
        void bind(std::string const& str_ip, uint16_t port);
        void connect(std::string const& str_ip, uint16_t port);
        void send(const char *msg);
        char *recv(size_t buffer_size = 8192);
        void listen(size_t num = 10);
        FSocket accept();
        void close();
    };
};




#endif