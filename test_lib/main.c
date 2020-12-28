#include "IWebsocket.h"
#include "Datastructures.h"

struct IWebsocket iwebsocket;
void *onopen(ws_client *wsclient){
}
void *onclose(ws_client *wsclient){
}

/*
 * caz javascript can't do well with binary data;
 * so just send text, message_new() is new a text message.
 */
extern const  ws_list * ws_get_clients_list();
static int ws_send_text(YourWsClient *wsclient, char *text){
    ws_list *l = ws_get_clients_list();
    ws_connection_close   status;
    ws_message            *m = message_new();
    m->opcode[0] = '\x81'; 
    
    m->len = strlen(text);
    char *temp = malloc( sizeof(char)*(m->len+1) );
    if (temp == NULL) {
        return 0;
    }
    memset(temp, '\0', (m->len+1));
    memcpy(temp, text, m->len);
    m->msg = temp;
    temp = NULL;
    
    
    if ( (status = encodeMessage(m)) != CONTINUE) {
        message_free(m);
        free(m);
        return -1;
    }
    list_multicast_one_unsafe(l, wsclient, m);
    message_free(m);
    free(m); 
    return 0;
}
static int ws_send_text_all(char *text){
    ws_list *l = ws_get_clients_list();
    ws_connection_close   status;
    ws_message            *m = message_new();
    m->opcode[0] = '\x81'; 
    
    m->len = strlen(text);
    char *temp = malloc( sizeof(char)*(m->len+1) );
    if (temp == NULL) {
        return 0;
    }
    memset(temp, '\0', (m->len+1));
    memcpy(temp, text, m->len);
    m->msg = temp;
    temp = NULL;
    
    
    if ( (status = encodeMessage(m)) != CONTINUE) {
        message_free(m);
        free(m);
        return -1;
    }
    list_multicast_all_unsafe(l, m);
    message_free(m);
    free(m); 
    return 0;
}
/*
 * don't need to free message.It will free after onmessage
 */
void *onmessage(YourWsClient *wsclient, char *str_msg, uint64_t str_msg_len){
    printf("Received(%s): %s\n",wsclient->client_ip , str_msg);
    
    iwebsocket.send_text(wsclient, str_msg);
    
    iwebsocket.send_text_all(str_msg);
    
}

int main(int argc, char *argv[]){
    iwebsocket.port                     = 8000;
    iwebsocket.onopen                   = onopen;
    iwebsocket.onclose                  = onclose;
    iwebsocket.onmessage                = onmessage;
    iwebsocket.max_client               = 2;
    iwebsocket.bNeed_stdinput_for_test  = 1;
    
    iwebsocket.send_text_all            = ws_send_text_all;
    iwebsocket.send_text                = ws_send_text;
    
    start_websocket_server(&iwebsocket);
    return 0;
}