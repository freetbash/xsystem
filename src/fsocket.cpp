#include <xsystem/xsystem.h>

void xsystem::net::FSocket::Address::set(std::string ip, uint16_t port){
	this->addr.sin_family=AF_INET;
	this->addr.sin_port=htons(port);
	#if __linux__	
		if(inet_pton(AF_INET, ip.c_str(), &this->addr.sin_addr) <= 0){PRINT_ERROR}
	#elif _WIN32
		this->addr.sin_addr.S_un.S_addr=inet_addr(ip.c_str());
	#endif

}
std::string xsystem::net::FSocket::Address::get_ip(){
	return std::string(inet_ntoa(this->addr.sin_addr));
}

uint16_t xsystem::net::FSocket::Address::get_port(){
	return ntohs(this->addr.sin_port);
}

xsystem::net::FSocket::FSocket(){
	#if __linux__
		this->nfd = socket(AF_INET,SOCK_STREAM,0);
		if(this->nfd<0){PRINT_ERROR};

	#elif _WIN32
		WSADATA wsaData;
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0){PRINT_ERROR}
		this->nfd = socket(AF_INET,SOCK_STREAM,0);
		if(this->nfd==INVALID_SOCKET){PRINT_ERROR}
	#endif
}

xsystem::net::FSocket::~FSocket(){
	#if __linux__
		close(this->nfd);
	#elif _WIN32
		WSACleanup();
		closesocket(this->nfd);
	#endif
}

void xsystem::net::FSocket::Bind(std::string ip, uint16_t port){
	this->address.set(ip,port);
	#if __linux__
		if(bind(this->nfd,(struct sockaddr *)&(this->address.addr),sizeof(struct sockaddr))==-1){PRINT_ERROR}
	#elif _WIN32
		if(bind(this->nfd,(SOCKADDR*)&(this->address.addr),sizeof(SOCKADDR))==SOCKET_ERROR){PRINT_ERROR}
	#endif
}

void xsystem::net::FSocket::Connect(std::string ip, uint16_t port){
	xsystem::net::FSocket::Address dst;
	dst.set(ip,port);
	#if __linux__
		if(connect(this->nfd,(struct sockaddr *)&(dst.addr),sizeof(struct sockaddr))<0){PRINT_ERROR}
	#elif _WIN32
		if(connect(this->nfd,(SOCKADDR *)&(dst.addr),sizeof(SOCKADDR))==SOCKET_ERROR){PRINT_ERROR}
	#endif
}

void xsystem::net::FSocket::Listen(size_t num){
	if(listen(this->nfd,num)==-1){PRINT_ERROR}
}

void xsystem::net::FSocket::Send(const char *msg){
	if(send(this->nfd,msg,strlen(msg),0)<0){PRINT_ERROR};
}

void xsystem::net::FSocket::Send(const char *msg, long len){
	if(send(this->nfd,msg,len,0)<0){PRINT_ERROR};

}

xsystem::net::FSocket xsystem::net::FSocket::Accept(){
	xsystem::net::FSocket client;
	
	#if __linux__
		unsigned int len=sizeof(sockaddr);
		uint32_t cfd = accept(this->nfd,(struct sockaddr *)&(client.address.addr),&len);
		if(cfd==-1){PRINT_ERROR};
	#elif _WIN32
		unsigned int len=sizeof(SOCKADDR);
		SOCKET cfd = accept(this->nfd,(SOCKADDR *)&(client.address.addr),&len);
		if(cfd==SOCKET_ERROR){PRINT_ERROR};
	#endif
	client.nfd=cfd;
	return client;
}

char *xsystem::net::FSocket::Recv(size_t buffer_size){
	char *temp=(char *)malloc(sizeof(char)*buffer_size);
	memset(temp,0,sizeof(char)*buffer_size);
	int len;
	len = recv(this->nfd,temp,buffer_size,0);
	if(len<0){PRINT_ERROR}
	return temp;
}

void xsystem::net::FSocket::Close(){
	#if __linux__
		close(this->nfd);
	#elif _WIN32
		closesocket(this->nfd);
	#endif
}