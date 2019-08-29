#ifndef _IWebsocket_H
#define _IWebsocket_H
#include "Datastructures.h"

struct IWebsocket{
    int  port;
    int  max_client;
    int  bNeed_stdinput_for_test;
    void *(*onopen)(ws_client *wsclient);
    void *(*onclose)(ws_client *wsclient);
    void *(*onmessage)(ws_client *, ws_message *);
};

/*
 * caz javascript can't do well with binary data;
 * so just send text, message_new() is new a text message.
 */
extern int ws_send_text(ws_client *wsclient, char *text);
extern int ws_send_text_all_client(char *text);

#endif