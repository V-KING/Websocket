
#include "net_ip.h"
#include "cJSON.h"
#include "s2j.h"
#include "AEI_S1/CpsDeviceMessage.h"


// #if 1
// 不要使用cJSON的动态库，可能存在如下问题：
// 1. 动态库全局变量s2jHook的问题
S2jHook s2jHook = {
        .malloc_fn = malloc,
        .free_fn = free,
};
/*****json to struct********************/
typedef struct {
    char name[16];
} Hometown;

typedef struct {
    uint8_t id;
    double weight;
    uint8_t score[8];
    char name[10];
    Hometown hometown;
} Student;
static void *json_to_struct(cJSON* json_obj) {
    /* create Student structure object */
    s2j_create_struct_obj(struct_student, Student);

    /* deserialize data to Student structure object. */
    s2j_struct_get_basic_element(struct_student, json_obj, int, id);
    s2j_struct_get_array_element(struct_student, json_obj, int, score);
    s2j_struct_get_basic_element(struct_student, json_obj, string, name);
    s2j_struct_get_basic_element(struct_student, json_obj, double, weight);

    /* deserialize data to Student.Hometown structure object. */
    s2j_struct_get_struct_element(struct_hometown, struct_student, json_hometown, json_obj, Hometown, hometown);
    s2j_struct_get_basic_element(struct_hometown, json_hometown, string, name);

    /* return Student structure object pointer */
    return struct_student;
}
/*****json to struct********************/

struct key_value{
    char key[100];
    char value[100];
    char *_tmp;
};

#define MAX_NUM_KV_STR      20
struct key_value a_kv[MAX_NUM_KV_STR];



/*
 * ParamStr=/vk?name=lbl&age=29&addr&high=176
 * TODO:
 * {
 *      vk: {
 *              "name" : "lbl",
 *              "age"  : 29,
 *              "addr" : "ShenZhen",
 *              "high" : 176,
 *      }
 * }
 * output: a_kv
 */
static void parseCmdParamStr2KeyValue(char *token, char buf_path[], struct key_value *a_kv){
    memset(a_kv, 0, sizeof (a_kv));
    int i = 0, j = 0;
    char *p = NULL;
    token = strtok(token, "/?");
    print_dbg("#\n");
    print_dbg("cmd: %s\n", token);
    strcpy(buf_path, token);
    print_dbg("#\n");
    if(token != NULL){
        print_dbg("#\n");
        if((token = strtok(NULL, "?")) != NULL){
            print_dbg("#\n");
            //params: name=lbl&age=29&addr&high=176
            p = strtok(token, "&");
            print_dbg("#\n");
            while(i<MAX_NUM_KV_STR && p != NULL){
                //print_dbg("p = %s\n", p);
                a_kv[i++]._tmp= p;
                p = strtok(NULL, "&");
            }
            print_dbg("#\n");
            while(j<MAX_NUM_KV_STR && a_kv[j]._tmp != NULL){
                if((a_kv[j]._tmp = strtok(a_kv[j]._tmp, "=")) != NULL){
                    strcpy(a_kv[j].key, a_kv[j]._tmp);
                }
                
                if((a_kv[j]._tmp = strtok(NULL, "=")) != NULL){
                    strcpy(a_kv[j].value, a_kv[j]._tmp);
                }
                j++;
            }
        }
    }
}

void parseHttpHeadersGetStr2KeyValues(const char *headers_get, char buf_path[], struct key_value *a_kv){
    memset(a_kv, 0, sizeof (a_kv));
    char *s = strdup(headers_get);
    char *token = strtok(s, " ");
    
    if(token != NULL){
        token = strtok(NULL, " ");
        if(token!=NULL){
            //token = /vk?name=lbl&age=29&addr&high=176
            parseCmdParamStr2KeyValue(token, buf_path, a_kv);
        }
    }
    free(s);
}

void *onopen(ws_client *wsclient){
    __pBegin
    print_err("open\n");
    ws_client *n = wsclient;
    /* route path */
    char buf_cmd[1000] = {0};
    if(wsclient == NULL){
        print_err("wsclient is null\n");
        return;
    }
    if(wsclient->headers == NULL){
        print_err("wsclient->headers is null\n");
        return;
    }
    print_dbg("ip = %s\n", wsclient->client_ip);
    
//     print_dbg("n->headers->get = %s\n", n->headers->get);
    parseHttpHeadersGetStr2KeyValues(n->headers->get, buf_cmd ,a_kv);
    print_dbg("cmd = %s\n", buf_cmd); //session
    for(int j = 0; a_kv[j].key[0]!=0; j++){
        print_dbg("a_kv[%d]: {%s: %s}\n", j,a_kv[j].key, a_kv[j].value);
    }

    __pEnd
}
void *onclose(ws_client *wsclient){
    __pBegin
    print_err("close\n");
    __pEnd
}
/*
 * don't need to free message.It will free after onmessage
 */
// void *onmessage(YourWsClient *n, ws_message *message){
void *onmessage(YourWsClient *n, char *str_msg, uint64_t str_msg_len){
    __pBegin
    
    /* here: ws_client->message == message */
    print_info("Received(%s): %s\n",n->client_ip , str_msg);
    //ws_send_text(n, str_msg);
    print_dbg("str_msg = %s\n", str_msg);
    print_dbg("str_msg_len= %d\n", str_msg_len);
    //TODO: here put a call_back funtion
    print_buf(str_msg, str_msg_len);
    if(str_msg_len == 0){
       print_err("msg is null\n");
       return;
    }
 
    /* ws://127.0.0.1:8000/vk?name=lbl&age=29&addr&high=176 */
    char buf_cmd[10] = {0};
    char json[4096] = {0};
    #if 0
    /* route path */
    parseHttpHeadersGetStr2KeyValues(n->headers->get, buf_cmd ,a_kv);
    print_dbg("cmd = %s\n", buf_cmd);
    for(int j = 0; a_kv[j].key[0]!=0; j++){
        print_dbg("a_kv[%d]: {%s: %s}\n", j,a_kv[j].key, a_kv[j].value);
    }
    #endif
    
    /* if the message is json text 
    {
        "id":   24,
        "weight":       71.2,
        "score":        [1, 2, 3, 4, 5, 6, 7, 8],
        "name": "armink",
        "hometown": {
            "name": "China"
        }
    }
    */
    {
        cJSON *json_student = cJSON_Parse(str_msg);
        print_dbg("#\n");
        if(json_student){
            print_dbg("#\n");
            printf("%s\n", cJSON_Print(json_student));
            Student *converted_student_obj = json_to_struct(json_student);
            print_dbg("id:     %d\n", converted_student_obj->id);
            print_dbg("weight: %f\n", converted_student_obj->weight);
            print_dbg("score:  ");
            printf("[");
            for(int i=0; i < 8; i++){
                printf("%d, ", converted_student_obj->score[i]);
            }
            printf("]\n");
            print_dbg("hometown.name: %s\n", converted_student_obj->hometown.name);
            s2j_delete_json_obj(json_student);
            s2j_delete_struct_obj(converted_student_obj);
        }else{
            print_dbg("#\n");
            /*msg: wsPerson?name=lbl&age=29&addr&high=176 */
            struct key_value a_kv[MAX_NUM_KV_STR];
            print_dbg("#\n");
            char buf_cmd[100] = {0};
            print_dbg("#\n");
            parseCmdParamStr2KeyValue(str_msg, buf_cmd,a_kv);
            print_dbg("#\n");
            print_dbg("cmd = %s\n", buf_cmd);
            for(int j = 0; a_kv[j].key[0]!=0; j++){
                print_dbg("a_kv[%d]: {%s: %s}\n", j,a_kv[j].key, a_kv[j].value);
            }
            if( 0 == strcmp(buf_cmd, "ws_ip") ) {
                if( strcmp(a_kv[0].key, "get") == 0){
                    /* '/ws_ip?get' */
                    char ip[20]      = {0};
                    char mask[20]    = {0};
                    char gateway[20] = {0};
                    net_getIp("enp0s3",ip); net_getMask("enp0s3",mask); net_getGateway("enp0s3",gateway);
                    print_dbg("ip : %s\n", ip);
                    sprintf(json, "{\"ip\": \"%s\", \"mask\": \"%s\", \"gateway\": \"%s\"}", ip, mask, gateway);
                    ws_send_text(n, json);
                }else if(strcmp(a_kv[0].key, "set") == 0){
                    /* TODO */
                    print_err("set .....\n");
                }
            }else if( 0 == strcmp(buf_cmd, "ws_power") ) {
                
            }else if( 0 == strcmp(buf_cmd, "ws_antenna") ) {
                
            }else{
                print_err("Unkonw cmd: %s\n", buf_cmd);
                ws_send_text(n, buf_cmd);
            }
        }
    }
    
    __pEnd
}

DeviceMessage deviceMessage = {
    .deviceInfo.mag1Resistance = 1726,
    .deviceInfo.mag2Resistance = 1726,
    .deviceInfo.mag3Resistance = 1726,
    .deviceInfo.mag4Resistance = 1726,
    .deviceInfo.rfStatus= 1,
    .deviceInfo.antennaConnection = 1,
    .deviceInfo.deviceDateTime= "2019/09/12 15:12:01",
    .cpsSettings.cpsConnectionStatus = 1,
};

void *thread_loop_send(void *args){
    __pBegin
    const ws_list *l = NULL ; 
    char buf[4096] = {0};
    struct IWebsocket *pIwebsocket = (struct IWebsocket*) args;

    while(1){
        l = ws_get_clients_list();
        if(l != NULL){
//             print_info("waiting ...\n");
            pthread_mutex_lock(&l->lock);
//             print_info("waited lock .....\n");
            if(l->len > 0){
//                 usleep(100*1000);
                sleep(1);
                //print_dbg("Number of websocket client: %d, ip[0]: %s\n", l->len, l->first->client_ip);
                memset(buf, 0, 100);
                deviceMessage.deviceInfo.mag1Resistance = rand()/1000000;
                deviceMessage.deviceInfo.mag2Resistance = rand()/1000000;
                deviceMessage.deviceInfo.mag3Resistance = rand()/1000000;
                deviceMessage.deviceInfo.mag4Resistance = rand()/10000000;
                
                cJSON *json_deviceMessage = DeviceMessage_to_json(&deviceMessage);
                char *tmp= cJSON_Print(json_deviceMessage);
                //printf("%s\n", tmp);
                //ws_send_text_all(l,tmp);
                pIwebsocket->send_text_all(tmp);
                
                pthread_mutex_unlock(&l->lock);
//                 print_info("unlock .....\n");

                s2j_delete_json_obj(json_deviceMessage);
                free(tmp);
            }else{
                pthread_mutex_unlock(&l->lock);
//                 print_info("unlock .....\n");
                sleep(1);
                print_dbg("no client\n");
            }
        }
    }
    __pEnd
    pthread_exit(NULL);
}
void *thread_loop_send1(void *args){
    __pBegin
    const ws_list *l = NULL ; 
    char buf[4096] = {0};
    uint64_t msgid = 0;
    int      value = 0;
    struct IWebsocket *pIwebsocket = (struct IWebsocket *) args;

    while(1){
        l = ws_get_clients_list();
        if(l != NULL){
            pthread_mutex_lock(&l->lock);
            if(l->len > 0){
                memset(buf, 0, 100);
                srand((unsigned)time(NULL));
                
                char testJSONContent[] ="{"
                "\"type\":\"deviceStatus\","
                "\"deviceInfo\":{\"mag1Resistance\":1, \"mag2Resistance\":1, \"mag3Resistance\":1, \"mag4Resistance\":1, \"rfStatus\":1, \"antennaConnection\":1,\"rfModule\":1, \"deviceDateTime\":\"2019-11-11 00:00:01\"},"
                "\"cpsSettings\":{\"cpsConnectionStatus\":1},"
                "\"upsSettings\":{\"upsStatus\":1}"
                "}";

                value = rand() % 1000;
                sprintf(buf, "{\"type\": \"sensorData\", \"msgid\":%d, \"strainValue\":%d}", msgid++, value);
                print_dbg("buf_len = %d\n", strlen(buf));
          
                //ws_send_text_all(l, testJSONContent);
                pIwebsocket->send_text_all(testJSONContent);
                pthread_mutex_unlock(&l->lock);

            }else{
                pthread_mutex_unlock(&l->lock);
                print_dbg("no client\n");
            }
            sleep(1);
        }
    }
    __pEnd
    pthread_exit(NULL);
}

void * thread_start_websocket_server(void *arg){
    struct IWebsocket *piwebsocket = (struct IWebsocket*)arg;
    start_websocket_server(piwebsocket);
}

/*
 *see also another websocket: https://github.com/payden/libwebsock
 */
int main(int argc, char *argv[]){
    struct IWebsocket iwebsocket;
    iwebsocket.port                     = 8000;
    iwebsocket.onopen                   = onopen;
    iwebsocket.onclose                  = onclose;
    iwebsocket.onmessage                = onmessage;
    iwebsocket.max_client               = 2;
    iwebsocket.bNeed_stdinput_for_test  = 1;
    
    iwebsocket.send_text_all            = ws_send_text_all;
    iwebsocket.send_text                = ws_send_text;
    
    
    pthread_t _pthread_id0;
    pthread_t _pthread_id;
    pthread_create(&_pthread_id0, NULL, thread_start_websocket_server, &iwebsocket);
//     pthread_create(&_pthread_id, NULL, thread_loop_send, NULL);
    pthread_create(&_pthread_id, NULL, thread_loop_send1, &iwebsocket);
    pthread_join(_pthread_id, NULL);
    pthread_join(_pthread_id0, NULL);
    
    return 0;
}
