#include "debug.h"
#include "net_ip.h"
#include <net/if.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <ctype.h>
 
 

//设置IP地址
/*
 * 函数名称 ： int setip(char *ip)
 * 函数功能 ： 设置系统IP地址
 * 参    数 ： 
 *char *ip ：设置的IP地址，以点分十进制的字符串方式表示，如“192.168.0.5” 
 * 返 回 值 ： 0 : 成功 ；  -1 :  失败 
 */
int net_setIp(const char *net_dev, char *ip)
{
    struct ifreq temp;
    struct sockaddr_in *addr;
    int fd = 0;
    int ret = -1;
    strcpy(temp.ifr_name, net_dev);
    if((fd=socket(AF_INET, SOCK_STREAM, 0))<0)
    {
      return -1;
    }
    addr = (struct sockaddr_in *)&(temp.ifr_addr);
    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = inet_addr(ip);
    ret = ioctl(fd, SIOCSIFADDR, &temp);
    close(fd);
    if(ret < 0)
       return -1;
    return 0;
}

//获取IP地址
/*
 * 函数名称 ： char * getip(char *ip_buf)
 * 函数功能 ： 获取系統IP地址
 * 参    数 ： 
 *char *ip_buf ：用来存放IP地址的内存空间
 * 返 回 值 ： ip_buf ： 存放IP地址的内存地址
 */
char* net_getIp(const char *net_dev, char *ip_buf)
{
    struct ifreq temp;
    struct sockaddr_in *myaddr;
    int fd = 0;
    int ret = -1;
    strcpy(temp.ifr_name, net_dev);
    if((fd=socket(AF_INET, SOCK_STREAM, 0))<0)
    {
        return-1;
    }
    ret = ioctl(fd, SIOCGIFADDR, &temp);
    close(fd);
    if(ret < 0)
        return NULL;
    myaddr = (struct sockaddr_in *)&(temp.ifr_addr);
    strcpy(ip_buf, inet_ntoa(myaddr->sin_addr));
    return ip_buf;
}
//end setip.c


int net_setMask(const char *net_dev,char *ip)
{
    struct ifreq temp;
    struct sockaddr_in *addr;
    int fd = 0;
    int ret = -1;
    strcpy(temp.ifr_name, net_dev);
    if((fd=socket(AF_INET, SOCK_STREAM, 0))<0)
    {
      return -1;
    }
    addr = (struct sockaddr_in *)&(temp.ifr_addr);
    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = inet_addr(ip);
    ret = ioctl(fd, SIOCSIFNETMASK, &temp);
    close(fd);
    if(ret < 0)
       return -1;
    return 0;
}


char* net_getMask(const char *net_dev, char *ip_buf)
{
    struct ifreq temp;
    struct sockaddr_in *myaddr;
    int fd = 0;
    int ret = -1;
    strcpy(temp.ifr_name, net_dev);
    if((fd=socket(AF_INET, SOCK_STREAM, 0))<0)
    {
        return-1;
    }
    ret = ioctl(fd, SIOCGIFNETMASK, &temp);
    close(fd);
    if(ret < 0)
        return NULL;
    myaddr = (struct sockaddr_in *)&(temp.ifr_addr);
    strcpy(ip_buf, inet_ntoa(myaddr->sin_addr));
    return ip_buf;
}


int net_setGateway(char* gateway)
{
    /*  设置默认网关  */
    int ret;
    char str[256];
    if( NULL == gateway ){
        print_err("gateway = null\n");
        return -1;
    }
    sprintf(str, "/sbin/route add default gw %s ", gateway);
    ret = system(str);
    if(ret < 0){
        return -1;
    }
    return 0;

}

char *net_getGateway(const char *net_dev, char *gateway)
{
    FILE *fp;    
    char buf[1024];  
    char iface[16];    
    unsigned char tmp[100]={'\0'};
    unsigned int dest_addr=0, gate_addr=0;
    
    err_nullptr_exe(gateway, return NULL);
    
    fp = fopen("/proc/net/route", "r");    
    err_nullptr_exe(fp, return NULL);
    
    fgets(buf, sizeof(buf), fp);    
    while(fgets(buf, sizeof(buf), fp)) 
    {    
        if((sscanf(buf, "%s\t%X\t%X", iface, &dest_addr, &gate_addr) == 3) 
            && (memcmp(net_dev, iface, strlen(net_dev)) == 0)
            && gate_addr != 0) 
        {
                memcpy(tmp, (unsigned char *)&gate_addr, 4);
                sprintf(gateway, "%d.%d.%d.%d", (unsigned char)*tmp, (unsigned char)*(tmp+1), (unsigned char)*(tmp+2), (unsigned char)*(tmp+3));
                break; 
        }
    }    
      
    fclose(fp);
    return gateway;
}


char *net_getMac(const char *net_dev,char *mac_buf) 
{
    int sock_info, err;
    struct ifreq ifr_netinfo;
    unsigned int ptemp[8];
    unsigned char mac_addr[30];
    err = -1;
    //printf("getMacInfo..\n");
    sock_info = socket(AF_INET, SOCK_STREAM, 0);
    if(sock_info == -1) {
        print_err("getMacInfo:get socket failed.\n");
        return err;
    }
    memset(&ifr_netinfo, 0, sizeof(ifr_netinfo));
    strncpy(ifr_netinfo.ifr_name, net_dev, sizeof(ifr_netinfo.ifr_name) - 1);
    if(ioctl(sock_info, SIOCGIFHWADDR, &ifr_netinfo) < 0) {
        print_err( "getnetworkInfo:ioctl MAC failed.\n");
        return err;
    }
    sprintf(mac_addr, "%02x%02x%02x%02x%02x%02x",
            (unsigned char)ifr_netinfo.ifr_hwaddr.sa_data[0],
            (unsigned char)ifr_netinfo.ifr_hwaddr.sa_data[1],
            (unsigned char)ifr_netinfo.ifr_hwaddr.sa_data[2],
            (unsigned char)ifr_netinfo.ifr_hwaddr.sa_data[3],
            (unsigned char)ifr_netinfo.ifr_hwaddr.sa_data[4],
            (unsigned char)ifr_netinfo.ifr_hwaddr.sa_data[5]);
    
    memcpy(mac_buf, mac_addr, strlen(mac_addr)+1);
    //sscanf(mac_addr, "%02x%02x%02x%02x%02x%02x", &ptemp[0], &ptemp[1], &ptemp[2], &ptemp[3], &ptemp[4], &ptemp[5]);
    close(sock_info);
    err = 0;
    return err;
}

/*
 * something wrong: can't be connected(tcp)
 */
static char* net_getGateway_old(char *gateway_addr)
{
    FILE *fp;
    char cmd[128];
    char buf[512];
    char *tmp;
    int err;
    unsigned int ptemp[8];
    strcpy(cmd,"ip route");
    fp=popen(cmd,"r");
    if(NULL==fp)
    {   
        print_err("getGateway:popen cmd failed.\n");
        return -1;
    }
    while(fgets(buf,sizeof(buf),fp)!=NULL)
    {
        tmp=buf;
        while(*tmp && isspace(*tmp))
          ++tmp;
         if(strncpy(tmp,"default",strlen("default"))==0)
             break;
    }
    /* buf = default via 192.168.7.1 dev eth0 */
    sscanf(buf,"%*s%*s%s",gateway_addr);
     
    /* is gateway legal formate? */
    if(inet_addr(gateway_addr) == INADDR_NONE){
        memset(gateway_addr, 0, 10);
        pclose(fp);
        return NULL;
    }
    
    pclose(fp);
  
    return gateway_addr;
}

