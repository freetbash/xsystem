#ifndef CROSS_SYSTEM
#define CROSS_SYSTEM
// -lws2_32 -liphlpapi -lgdi32
#include <string>
#include <string.h>
#include <stdio.h>
#include <iostream>
#if __linux__
#include <unistd.h>

#elif _WIN32
#include <Windows.h>
#include <Winsock2.h>
#pragma comment(lib, "ws2_32.lib")

#endif
#define PRINT_ERROR printf("%s:%d, %s\n", __FILE__, __LINE__, strerror(errno));exit(-1);

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



        // net
    #if __linux__
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #elif _WIN32
    #include <winsock.h>
    #pragma comment(lib, "ws2_32.lib")
    #endif
    namespace net
    {
        class FSocket{
        public:
            class Address{
            public:
                #if __linux__
                    sockaddr_in addr;
                #elif _WIN32
                    SOCKADDR_IN addr;
                #endif
                void set(std::string ip, uint16_t port);
                std::string get_ip();
                uint16_t get_port();
            };

            uint32_t nfd;
            Address address;
            FSocket();
            ~FSocket();
            void Bind(std::string ip, uint16_t port);
            void Listen(size_t num = 512);
            void Connect(std::string ip, uint16_t port);
            FSocket Accept();
            void Send(const char *msg);
            void Send(const char *msg,long len);
            char *Recv(size_t buffer_size = 2048);
            void Close();
        };
        std::string domain_to_ip(std::string domain);
        
    };
    namespace tools{
        void screenshot(char *savepath);
    };

};

#endif