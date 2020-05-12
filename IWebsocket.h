#ifndef _IWebsocket_H
#define _IWebsocket_H
#include "Datastructures.h"
#include "Communicate.h"
#include "Handshake.h"

// TODO: change to you date 
typedef ws_client  YourWsClient;//can be void*

struct IWebsocket{
    int  port;
    int  max_client;
    int  bNeed_stdinput_for_test;
//     void *(*onopen)(ws_client *wsclient);
//     void *(*onclose)(ws_client *wsclient);
//     void *(*onmessage)(ws_client *, ws_message *);
    
    void *(*onopen)(YourWsClient *wsclient);//wsclient can be private data
    void *(*onclose)(YourWsClient *wsclient);//wsclient can be private data
    void *(*onmessage)(YourWsClient *wsclient,char *msg, uint64_t len);//it's a text
    
    int (*send_text)(YourWsClient *wsclient, char *);
    int (*send_text_all)(char *);
    void *private_data;//store ws_list*
};

/*
 * caz javascript can't do well with binary data;
 * so just send text, message_new() is new a text message.
 */
// extern int ws_send_text(YourWsClient *wsclient, char *text);
// extern int ws_send_text_all(ws_list *l,char *text);
extern int start_websocket_server(struct IWebsocket *iwebsocket);
// extern const ws_list *  ws_get_clients_list();


#endif