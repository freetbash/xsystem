#include <xsystem/xsystem.h>
#if __linux__
#include <unistd.h>  //close 和 shutdown
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> //inet_ntop
#include <string.h>
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

net::FSocket::address::address(){
	memset(&_socket_addr, 0, sizeof(_socket_addr));
};

net::FSocket::address::address(std::string const& str_ip, uint16_t port) {
	set(str_ip, port);
};

void net::FSocket::address::set(std::string const& str_ip, uint16_t port) {
	_socket_addr.sin_family = AF_INET;  //TCP ipv4
	SET_IP(_socket_addr, str_ip);  //点分十进制ip转32位
	_socket_addr.sin_port = htons(port); //转成网络字节序
}
sockaddr const* net::FSocket::address::socket_addr(){
	return (sockaddr*)(&_socket_addr);
}

size_t net::FSocket::address::size(){
	return sizeof(sockaddr);
}

net::FSocket::FSocket(){ this->sfd = socket(AF_INET, SOCK_STREAM, 0); if(this->sfd < 0){PRINT_ERROR} };
net::FSocket::FSocket(std::string const& str_ip, uint16_t port) :addr(str_ip, port) { 
	this->sfd = socket(AF_INET, SOCK_STREAM, 0); 
	if(this->sfd<0){
		PRINT_ERROR
	}
	assert(::bind(this->sfd, addr.socket_addr(), addr.size()) != SOCKET_ERROR);
};
net::FSocket::~FSocket() { this->close(); };

void net::FSocket::bind(std::string const& str_ip, uint16_t port) {
	addr.set(str_ip, port);
	if(::bind(this->sfd, addr.socket_addr(), addr.size()) <0){
		PRINT_ERROR
	}
}

void net::FSocket::connect(std::string const& str_ip, uint16_t port) {
	address server_addr(str_ip, port);
	if(::connect(this->sfd, server_addr.socket_addr(), server_addr.size())){
		PRINT_ERROR
	}
}

void net::FSocket::send(const char *msg) {
	if(::send(this->sfd, msg, strlen(msg) + 1, 0)<0){
		PRINT_ERROR
	}
}

char *net::FSocket::recv(size_t buffer_size) {
	char *buffer = new char[buffer_size]; //8KB
	if (int t =::recv(this->sfd, buffer, buffer_size, 0) < 0) {
		PRINT_ERROR
	}
	return buffer;
}

void net::FSocket::listen(size_t num) {
	if(::listen(this->sfd, num) <0){
		PRINT_ERROR
	}
}
net::FSocket net::FSocket::accept() {
	//后面两个参数可以获得客户端的信息。这里置空不去管。
	net::FSocket client;
	
	#if __linux__
		socklen_t t = sizeof(sockaddr_in);
	#elif _WIN32
		int t = sizeof(sockaddr_in);
	#endif
	client.sfd = ::accept(this->sfd, (sockaddr *)&client.addr._socket_addr, &t);  
	printf("%d\n",client.sfd);
	if(client.sfd <0){
		PRINT_ERROR
	}
	return client;
}

void net::FSocket::close() {
	CLOSE_SOCKET(this->sfd);
}
