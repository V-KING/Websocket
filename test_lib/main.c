#include "IWebsocket.h"
void *onopen(ws_client *wsclient){
}
void *onclose(ws_client *wsclient){
}
/*
 * don't need to free message.It will free after onmessage
 */
void *onmessage(ws_client *wsclient, ws_message *message){
    /* here: ws_client->message == message */
    printf("Received(%s): %s\n",wsclient->client_ip , message->msg);
    ws_send_text(wsclient, message->msg);
    ws_send_text_all(message->msg);
}

int main(int argc, char *argv[]){
    struct IWebsocket iwebsocket;
    iwebsocket.port      = 8000;
    iwebsocket.onopen    = onopen;
    iwebsocket.onclose   = onclose;
    iwebsocket.onmessage = onmessage;
    iwebsocket.max_client= 2;
    iwebsocket.bNeed_stdinput_for_test = 1;
    
    start_websocket_server(&iwebsocket);
    return 0;
}