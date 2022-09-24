#include <xsystem/xsystem.h>

std::string xsystem::get_hostname(){
    #if __linux__
        char hostname[128];
	    gethostname(hostname, sizeof(hostname));
	    return hostname;
    #elif _WIN32
        WSAData w;
        WSAStartup(MAKEWORD(2,2),&w);
        char hostname[128];
	    gethostname(hostname, sizeof(hostname));
        WSACleanup();
	    return hostname;
    #endif
    
}