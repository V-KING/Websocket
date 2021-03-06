# API
## For linux pc: 
test on linux pc(Ubuntu12.04, Ubuntu16.04):
> make pc
> make run


generate --> libwebsocket.a
> make libwebsocket_pc


## For linux ARM
generate --> libwebsocket.a: 
> make libwebsocket_arm CROSS_COMPILE=/gcc_path/arm-linux-


## How to use libwebsocket.a
please see "test_lib/main.c"
```c
#include "IWebsocket.h"
void *onopen(ws_client *wsclient){
}
void *onclose(ws_client *wsclient){
}
/*
 * don't need to free message.It will free after onmessage
 */
#if 0
//!deprecated
void *onmessage(ws_client *wsclient, ws_message *message){
    /* here: ws_client->message == message */
    print_info("Received(%s): %s\n",wsclient->client_ip , message->msg);
    ws_send_text(wsclient, message->msg);
    ws_send_text_all(message->msg);
}
#endif
void *onmessage(YourWsClient *wsclient, char *str_msg, int str_msg_len){
    /* here: ws_client->message == message */
    print_info("Received(%s): %s\n",wsclient->client_ip , str_msg);
    
    iwebsocket.send_text(ws_client, str_msg);
    iwebsocket.send_text_all(str_msg)
}

int ws_send_text(YourWsClient *ws_client, char *str_msg, int str_msg_len){
    //TODO:  Implement your code 
}
int ws_send_text_all(char *str_msg){
    //TODO:  Implement your code 
}

int main(int argc, char *argv[]){
    struct IWebsocket iwebsocket;
    iwebsocket.port      = 8000;
    iwebsocket.onopen    = onopen;
    iwebsocket.onclose   = onclose;
    iwebsocket.onmessage = onmessage;
    iwebsocket.max_client= 2;
    iwebsocket.bNeed_stdinput_for_test = 1;
    iwebsocket.send_text_all            = ws_send_text_all;
    iwebsocket.send_text                = ws_send_text;

    
    start_websocket_server(&iwebsocket);
    return 0;
}
```
## TODO
- 客户端通过用户名密码调用服务端的接口来获取角色Token
- 链接websoket时带上此Token（wss://localhost:7896/?token=Token）
- websoket服务端建立连接时验证此Token是否有效，无效则连接失败
- 有效，通过Token中的数据来设置此连接的一些信息（比如：用户ID类似的）
- 以后客户端发送请求使用此连接


# Websocket Server in C 

The code in this repository is supposed to support the awesome websocket 
feature, that was introduced as part of the HTML5 standard. The idea with the
project was originally to learn the C language and furthermore understand the
basics of websockets. 


# Support

The websocket server is written in C and should be supported by the most UNIX 
systems. It does not have any dependencies to other libraries than the standard
libraries in UNIX. As of 09/04-2013 it has been compiled and tested on the 
following operating systems:

* Ubuntu 12.04, Ubuntu16.04, arm
* Arch Linux
* MAC OS x 10.8

BEWARE! It does not work in Windows!

# Conventions

As websockets is a pretty new feature, there has been a lot of different 
conventions on how to handle the communication between browser and server. This
websocket server should support the following conventions:

* [hixie-75](http://tools.ietf.org/html/draft-hixie-thewebsocketprotocol-75) 
which was supported by Chrome 4 and Safari 5.0.0 and forth. (NOT TESTED and/or 
IMPLEMENTED)
* [hixie-76](http://tools.ietf.org/html/draft-hixie-thewebsocketprotocol-76)
which was supported by Firefox 4, Chrome 6, Safari 5.0.1 and Opera 11 and forth.
* [hybi-07](http://tools.ietf.org/html/draft-ietf-hybi-thewebsocketprotocol-07)
which was supported by Firefox 6 and forth.
* [hybi-10](http://tools.ietf.org/html/draft-ietf-hybi-thewebsocketprotocol-10)
which was supported by Firefox 7, Chrome 14 and forth.
* [RFC6455](http://tools.ietf.org/html/rfc6455)
which was supported by IE 10, Firefox 11, Chrome 16, Safari 6 and Opera 12.10 
and forth.

The latter RFC6455 is the convention that is supposed to be the standard for
websockets according to the HTML5 standards.

# How to use
With the C code follows a makefile. This is used to compile and run the code.
What you do is simply open your terminal and navigate to the root folder of the
websocket server.

To compile the code, simply type:
`make`

To run the websocket server type:
`make run`

To run the websocket server with valgrind (requires that valgrind is installed) 
type:
`make valgrind`

The default port of the server is currently port 4567. If you wish to have 
another port you can simply type:
`make run PORT=1111`
which will make the server listen at port 1111.

When the server is up and running, it has a few commands that could be useful.
These commands can be displayed by typing `help`.

Last but not at least, it is up to the one running the server to decide which 
hosts and origins that is allowed. To choose these addresses, you can edit the 
2 files `Hosts.dat` and `Origins.dat`. The first line in the file indicates the
amount of addresses allowed, and the following lines is the actual addresses 
allowed.

`Hosts.dat` has the information:
<pre>
2
localhost
127.0.0.1
</pre>

`Origins.dat` has the information:
<pre>
2
http://localhost
http://127.0.0.1
</pre>

As some of the conventions does not require the client to set an origin, I have
choosen to implement it as follows. If the client supplies an origin, then we
check if the origin is listed in `Origins.dat`. If the origin was not supplied,
2 things can happen. In `Includes.h` a constant "ORIGIN\_REQUIRED" has been 
made, which defines which one of the two options to choose. If this constant
is 0, we accept the fact that we can't check the origin from the client and
just moves on. If it is not 0, then we close the connection to the client, as
he was not able to identify where he originated from.

# Future implementations

In the future, the server should be able to communicate with browsers, 
eventhough the browser is just trying to contact the server using a normal 
HTTP Request. The idea is to implement some kind of COMET server, such that 
the server is useful for old browsers as well.

Another thing that would be preferable is that the server is able to handle
SSL connections. Which includes being able to handle wss:// connections. This
implementation would probably require that "OPENSSL" is installed on the 
computer, as it would be too much work to implement my own version of SSL.

Finally me and my pal is currently developing a benchmark tool for a websocket 
server, such that we can find bugs in the server and benchmark how much it can
do. The project can be seen [here](https://github.com/hovmand/go-websocket-bench)
.
