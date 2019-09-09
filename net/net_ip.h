#ifndef __net_ip_H 
#define __net_ip_H 

#define MAXINTERFACES           16

char*   net_getIp(const char *net_dev,char *ip_buf);
char*   net_getGateway(const char *net_dev, char *gateway);
char*   net_getMask(const char *net_dev,char *mask);
char*   net_getMac(const char *net_dev, char *mac_buf) ;

int     net_setIp(const char *net_dev, char *ip);
int     net_setGateway(char *gateway);
int     net_setMask(const char *net_dev ,char *mask);


#endif
