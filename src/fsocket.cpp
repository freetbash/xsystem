#include <xsystem/xsystem.h>
#if __linux__
#include <unistd.h>  //close 和 shutdown
#include <sys/types.h>
#include <sys/socket.h>
typedef uint32_t in_addr_t;
typedef u_int sa_family_t;
#include <arpa/inet.h> //inet_ntop
#include <string.h>
#include <netdb.h>
#define SOCKET_ERROR (-1)
#define CLOSE_SOCKET(x) ::close(x);
#define SET_IP(socket_addr,str_ip) \
socket_addr.sin_addr.s_addr = inet_addr(str_ip.c_str());

#elif _WIN32
//win64下也有_win32宏为1

#define CLOSE_SOCKET(x) closesocket(x);
#define SET_IP(_socket_addr,str_ip) \
_socket_addr.sin_addr.S_un.S_addr = inet_addr(str_ip.c_str());

class win_socket_dll {
public:
	win_socket_dll() {
		uint16_t w_req = MAKEWORD(2, 2);//版本号
		WSADATA wsadata;
		if (WSAStartup(w_req, &wsadata) != 0) exit(1);
		if (LOBYTE(wsadata.wVersion) != 2 || HIBYTE(wsadata.wHighVersion) != 2) {
			WSACleanup();
			exit(2);
		}
	}
	~win_socket_dll() {
		WSACleanup();
	}
};
static win_socket_dll dll;

#if _WIN64
#pragma warning (disable:4267)
#pragma warning (disable:4244)
#endif

#endif

#include <string>
#include <cassert>

xsystem::net::FSocket::address::address(){
	memset(&_socket_addr, 0, sizeof(_socket_addr));
};

xsystem::net::FSocket::address::address(std::string const& str_ip, uint16_t port) {
	set(str_ip, port);
};

void xsystem::net::FSocket::address::set(std::string const& str_ip, uint16_t port) {
	_socket_addr.sin_family = AF_INET;  //TCP ipv4
	SET_IP(_socket_addr, str_ip);  //点分十进制ip转32位
	_socket_addr.sin_port = htons(port); //转成网络字节序
}

void xsystem::net::FSocket::address::set_by_domain(std::string domain, uint16_t port) {
	_socket_addr.sin_family = AF_INET;  //TCP ipv4
	_socket_addr.sin_port = htons(port); //转成网络字节序
	#if __linux__
		struct hostent* h;
		if ( (h = gethostbyname(domain.c_str())) == 0 )   // 指定服务端的ip地址。
		{
			PRINT_ERROR
		}
		struct sockaddr_in servaddr;
		memcpy(&_socket_addr.sin_addr,h->h_addr,h->h_length);
	#elif _WIN32
		struct hostent *hptr=NULL;
		hptr = (struct hostent *)gethostbyname(domain.c_str());

	//无效指针则结束
		if (hptr == NULL || hptr->h_addr == NULL) {
			PRINT_ERROR
		}
		CopyMemory(&_socket_addr.sin_addr.S_un.S_addr, hptr->h_addr_list[0], hptr->h_length);
	#endif
	
}


size_t xsystem::net::FSocket::address::size(){
	return sizeof(sockaddr);
}

xsystem::net::FSocket::FSocket(){ this->sfd = socket(AF_INET, SOCK_STREAM, 0); if(this->sfd < 0){PRINT_ERROR} };
xsystem::net::FSocket::FSocket(std::string const& str_ip, uint16_t port) :addr(str_ip, port) { 
	this->sfd = socket(AF_INET, SOCK_STREAM, 0); 
	if(this->sfd<0){
		PRINT_ERROR
	}
	#if __linux__
		assert(xsystem::bind(this->sfd, (sockaddr *)&addr._socket_addr, addr.size()) != SOCKET_ERROR);
	#elif _WIN32
		assert(::bind(this->sfd, (sockaddr *)&addr._socket_addr, addr.size()) != SOCKET_ERROR);
	#endif
};
xsystem::net::FSocket::~FSocket() { this->close(); };

void xsystem::net::FSocket::bind(std::string const& str_ip, uint16_t port) {
	addr.set(str_ip, port);
	#if __linux__
		if(xsystem::bind(this->sfd, (sockaddr *)&addr._socket_addr, addr.size()) <0){PRINT_ERROR}
	#elif _WIN32
		if(::bind(this->sfd, (sockaddr *)&addr._socket_addr, addr.size()) <0){PRINT_ERROR}
	#endif
}

void xsystem::net::FSocket::connect(std::string const& str_ip, uint16_t port) {
	address server_addr(str_ip, port);
	#if __linux__
		if(xsystem::connect(this->sfd, (sockaddr *)&server_addr._socket_addr, server_addr.size())){PRINT_ERROR}
	#elif _WIN32
		if(::connect(this->sfd, (sockaddr *)&server_addr._socket_addr, server_addr.size())){PRINT_ERROR}
	#endif
}

void xsystem::net::FSocket::send(const char *msg) {
	#if __linux__
		if(xsystem::send(this->sfd, msg, strlen(msg) + 1, 0)<0){PRINT_ERROR}
	#elif _WIN32
		if(::send(this->sfd, msg, strlen(msg) + 1, 0)<0){PRINT_ERROR}
	#endif
	
}
void xsystem::net::FSocket::send(const char *msg,long len) {
	#if __linux__
		if(xsystem::send(this->sfd, msg, len, 0)<0){PRINT_ERROR}
	#elif _WIN32
		if(::send(this->sfd, msg, len, 0)<0){PRINT_ERROR}
	#endif
}

char *xsystem::net::FSocket::recv(size_t buffer_size) {
	char *buffer = new char[buffer_size]; //8KB
	#if __linux__
		if (int t =xsystem::recv(this->sfd, buffer, buffer_size, 0) < 0) {PRINT_ERROR}
	#elif _WIN32
		if (int t =::recv(this->sfd, buffer, buffer_size, 0) < 0) {PRINT_ERROR}
	#endif
	return buffer;
}

void xsystem::net::FSocket::listen(size_t num) {
	#if __linux__
		if(xsystem::listen(this->sfd, num) <0){PRINT_ERROR}
	#elif _WIN32
		if(::listen(this->sfd, num) <0){PRINT_ERROR}
	#endif

}
xsystem::net::FSocket xsystem::net::FSocket::accept() {
	//后面两个参数可以获得客户端的信息。这里置空不去管。
	xsystem::net::FSocket client;
	
	#if __linux__
		socklen_t t = sizeof(sockaddr_in);
		client.sfd = xsystem::accept(this->sfd, (sockaddr *)&client.addr._socket_addr, &t);  
	#elif _WIN32
		int t = sizeof(sockaddr_in);
		client.sfd = ::accept(this->sfd, (sockaddr *)&client.addr._socket_addr, &t);  
	#endif
	
	printf("%d\n",client.sfd);
	if(client.sfd <0){
		PRINT_ERROR
	}
	return client;
}

void xsystem::net::FSocket::close() {
	CLOSE_SOCKET(this->sfd);
}
