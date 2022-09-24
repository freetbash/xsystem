#include <xsystem/xsystem.h>
#if __linux__
#include <net/if.h>
#include <sys/ioctl.h>
#elif _WIN32
#include <Iphlpapi.h>
#include <time.h>
void byte2Hex(unsigned char bData,unsigned char hex[]){
    int high=bData/16,low =bData %16;
    hex[0] = (high <10)?('0'+high):('A'+high-10);
    hex[1] = (low <10)?('0'+low):('A'+low-10);
}
#endif
std::string xsystem::get_mac(){
    #if __linux__
        char mac[30];
        struct ifreq tmp;
        int sock_mac;
        char mac_addr[30];
        sock_mac = socket(AF_INET, SOCK_STREAM, 0);
        if( sock_mac == -1){PRINT_ERROR}
        memset(&tmp,0,sizeof(tmp));
        strncpy(tmp.ifr_name,"eth0",sizeof(tmp.ifr_name)-1 );
        if( (ioctl( sock_mac, SIOCGIFHWADDR, &tmp)) < 0 ){PRINT_ERROR}
        sprintf(mac_addr, "%02x:%02x:%02x:%02x:%02x:%02x",
            (unsigned char)tmp.ifr_hwaddr.sa_data[0],
            (unsigned char)tmp.ifr_hwaddr.sa_data[1],
            (unsigned char)tmp.ifr_hwaddr.sa_data[2],
            (unsigned char)tmp.ifr_hwaddr.sa_data[3],
            (unsigned char)tmp.ifr_hwaddr.sa_data[4],
            (unsigned char)tmp.ifr_hwaddr.sa_data[5]
        );
        close(sock_mac);
        memcpy(mac,mac_addr,strlen(mac_addr));
        return mac;
    #elif _WIN32
        
        unsigned char mac[1024];
        ULONG ulSize=0;
        PIP_ADAPTER_INFO pInfo=NULL;
        int temp=0;
        temp = GetAdaptersInfo(pInfo,&ulSize);//第一处调用，获取缓冲区大小
        pInfo=(PIP_ADAPTER_INFO)malloc(ulSize);
        temp = GetAdaptersInfo(pInfo,&ulSize);
        int iCount=0;
        while(pInfo){//遍历每一张网卡{//  pInfo->Address MAC址
            for(int i=0;i<(int)pInfo->AddressLength;i++){
                byte2Hex(pInfo->Address[i],&mac[iCount]);
                iCount+=2;
                if(i<(int)pInfo->AddressLength-1){
                    mac[iCount++] = ':';
                }else{
                    mac[iCount++] = '#';
                }
            }
            pInfo = pInfo->Next;
        }

        return (char *)mac;

    
    #endif
}