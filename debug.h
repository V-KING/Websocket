#ifndef _debug_H
#define _debug_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>


#if 0
#define print_dbg(fmt, arg...)
#define print_info(fmt, arg...)
#define print_err(fmt, arg...)
#define __print_errno
#define print_pAddr(p1, p2)
#define print_buf(buf, len)
#define __step
#define __pBegin
#define __pEnd
#define print_y(fmt, arg...)
#else
#define print_y(fmt, arg...)        printf("\033[33m[DBG ][%s: %s:%d]" fmt, __FILE__, __FUNCTION__, __LINE__, ##arg)
#define print_dbg(fmt, arg...)      printf("\033[37m[DBG ][%s: %s:%d]" fmt, __FILE__, __FUNCTION__, __LINE__, ##arg)
#define print_info(fmt, arg...)     printf("\033[32m[INFO][%s: %s:%d]" fmt, __FILE__, __FUNCTION__, __LINE__, ##arg)
#define print_err(fmt, arg...)      printf("\033[31m[ERR ][%s: %s:%d]" fmt, __FILE__, __FUNCTION__, __LINE__, ##arg)
#define __print_errno               print_err("[errno, err] = [%d, %s]\n", errno, strerror(errno));
#define print_pAddr(p1, p2)         printf("\033[37m[Addr compare][%s: %s:%d][0x%x :: 0x%x]\n", __FILE__, __FUNCTION__, __LINE__, p1,p2)
#define __step                      print_dbg("#\n");
#define __pBegin print_y("begin ..\n");
#define __pEnd   print_y("end   ..\n");
#define print_buf(buf, len)     do{         \
    int i = 0;                              \
    print_info("");                     \
    printf("buf data[%d] = {", len);                 \
    for(i = 0; i < len; i++)                \
        printf("0x%02x, ", *((unsigned char*)buf+i));           \
    printf("}\n");                          \
}while(0)
#endif

#ifdef INVG_RELEASE
#define print_dbg(fmt, arg...)
#define print_info(fmt, arg...)
#define print_err(fmt, arg...)          psys_err(fmt, arg...)
#define __print_errno
#define print_pAddr(p1, p2)
#define print_buf(buf, len)
#define __step
#define __pBegin
#define __pEnd
#define print_y(fmt, arg...)
#endif

#define STR(s)                          #s
#define err_nullptr_exe(ptr, expresion)     if(NULL == ptr) {expresion;}
#define err_nullptr(ptr)                    if(NULL == ptr) print_err("%s is null\n", STR(ptr))
#define err_int_return(ret)                 if(ret < 0) {print_err("%s\n", strerror(errno)); return ret;}



#endif
