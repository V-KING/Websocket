#include "debug.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <syslog.h>

void  psys_dbg(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsyslog(LOG_DEBUG, fmt, ap);
    va_end(ap);
    return;
}
void  psys_err(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsyslog(LOG_ERR, fmt, ap);
    va_end(ap);
    return;
}
void  psys_info(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsyslog(LOG_INFO, fmt, ap);
    va_end(ap);
    return;
}
void  print_syslog(int level,const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsyslog(level, fmt, ap);
    va_end(ap);
    return;
}
