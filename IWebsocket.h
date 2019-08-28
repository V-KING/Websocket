#ifndef _IWebsocket_H
#define _IWebsocket_H
#include "Datastructures.h"

struct IWebsocket{
    int  port;
    int  bNeed_stdinput_for_test;
    void *(*onopen)(ws_client *wsclient);
    void *(*onclose)(ws_client *wsclient);
    void *(*onmessage)(ws_client *, ws_message *);
};

#endif