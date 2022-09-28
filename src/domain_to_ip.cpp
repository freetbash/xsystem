#include <xsystem/xsystem.h>
#if __linux__
#include <stdlib.h>
typedef unsigned int sa_family_t;
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>  // gethostbyname
#elif _WIN32
#include<WinSock2.h>
#pragma comment(lib,"ws2_32.lib")
#endif
#include <iostream>
std::string xsystem::net::domain_to_ip(std::string domain){
	char* ip;
	#if __linux__
		struct hostent* phost = gethostbyname(domain.c_str());
		if (phost== NULL){
		
		}
		ip = inet_ntoa(*(struct in_addr*)phost->h_addr_list[0]);
	#elif _WIN32
		WSAData wsa;
		WSAStartup(MAKEWORD(2,2),&wsa);
		hostent *phst=(hostent *)gethostbyname("www.baidu.com");
		in_addr * iddr=(in_addr*)phst->h_addr;
		unsigned long IPUL=iddr->s_addr;
		ip=inet_ntoa(*iddr);
        
	#endif
	return ip;
}
