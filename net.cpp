#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <linux/if.h>
#include <sys/ioctl.h>
#include "net.h"
#include "debug.h"

void ipaddr( const char* device, const char* ip ){
    ip = GetMyIpAddr( device );
} 

const char* GetMyIpAddr(const char* device_name) 
{
    int s = socket(AF_INET, SOCK_STREAM, 0);

    struct ifreq ifr;
    ifr.ifr_addr.sa_family = AF_INET;
    strcpy(ifr.ifr_name, device_name);
    ioctl(s, SIOCGIFADDR, &ifr);
    close(s);

    struct sockaddr_in addr;
    memcpy( &addr, &ifr.ifr_ifru.ifru_addr, sizeof(struct sockaddr_in) );

    return inet_ntoa(addr.sin_addr);
}