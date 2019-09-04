/******************************************************************************
  Copyright (c) 2013 Morten Houmøller Nygaard - www.mortz.dk - admin@mortz.dk
 
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
******************************************************************************/

#include "Handshake.h"
#include "Communicate.h"
#include "Errors.h"
#include "debug.h"
#include "IWebsocket.h"

#define PORT 4567

ws_list *l;
int port;
int count_client = 0;

/*
 * caz javascript can't do well with binary data;
 * so just send text, message_new() is new a text message.
 */
int ws_send_text(ws_client *wsclient, char *text){
    __pBegin
    ws_connection_close   status;
    ws_message            *m = message_new();
    m->opcode[0] = '\x81'; 
    
    m->len = strlen(text);
    char *temp = malloc( sizeof(char)*(m->len+1) );
    if (temp == NULL) {
        print_err("malloc err\n");
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
    list_multicast_one(l, wsclient, m);
    message_free(m);
    free(m); 
    __pEnd
    return 0;
}

/*
 * caz javascript can't do well with binary data;
 * so just send text
 */
int ws_send_text_all(char *text){
    __pBegin
    ws_connection_close   status;
    ws_message            *m = message_new();
    m->opcode[0] = '\x81'; 
    
    m->len = strlen(text);
    char *temp = malloc( sizeof(char)*(m->len+1) );
    if (temp == NULL) {
        print_err("malloc err\n");
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
    list_multicast_all(l, m);
    message_free(m);
    free(m); 
    __pEnd
    return 0;
}
/**
 * Handler to call when CTRL+C is typed. This function shuts down the server
 * in a safe way.
 */
void sigint_handler(int sig) {
	if (sig == SIGINT || sig == SIGSEGV) {
		if (l != NULL) {
			list_free(l);
			l = NULL;
		}
		(void) signal(sig, SIG_DFL);
		exit(0);
	} else if (sig == SIGPIPE) {
		(void) signal(sig, SIG_IGN);
	}
}

/**
 * Shuts down a client in a safe way. This is only used for Hybi-00.
 */
void cleanup_client(void *args) {
	ws_client *n = args;
	if (n != NULL) {
		print_info("Shutting client down..\n\n> ");
		fflush(stdout);
		list_remove(l, n);
	}
}

/**
 * This function listens for input from STDIN and tries to match it to a 
 * pattern that will trigger different actions.
 */
void *cmdline(void *arg) {
	pthread_detach(pthread_self());
	(void) arg; char buffer[1024];
	
	while (1) {
		memset(buffer, '\0', 1024);
		printf("> \n");
		fflush(stdout);
		fgets(buffer, 1024, stdin);
		
		if (strncasecmp(buffer, "users", 5) == 0 || 
				strncasecmp(buffer, "online", 6) == 0 ||
				strncasecmp(buffer, "clients", 7) == 0) {
			list_print(l);
			continue;
		} else if (strncasecmp(buffer, "exit", 4) == 0 || 
				strncasecmp(buffer, "quit", 4) == 0) {
			raise(SIGINT);
			break;
		} else if ( strncasecmp(buffer, "help", 4) == 0 ) {
			printf("------------------------ HELP ------------------------\n");
			printf("|   To display information about the online users,   |\n");
			printf("|   type: 'users', 'online', or 'clients'.           |\n");
			printf("|                                                    |\n");
			printf("|   To send a message to a specific user from the    |\n");
			printf("|   server type: 'send <IP> <SOCKET> <MESSAGE>' or   |\n");
			printf("|   'write <IP> <SOCKET> <MESSAGE>'.                 |\n");
			printf("|                                                    |\n");
			printf("|   To send a message to all users from the server   |\n");
			printf("|   type: 'sendall <MESSAGE>' or 'writeall           |\n");
			printf("|   <MESSAGE>'.                                      |\n");
			printf("|                                                    |\n");
 			printf("|   To kick a user from the server and close the     |\n");
			printf("|   socket connection type: 'kick <IP> <SOCKET>'     |\n");
			printf("|   or 'close <IP> <SOCKET>'.                        |\n");
			printf("|                                                    |\n"); 
 			printf("|   To kick all users from the server and close      |\n");
			printf("|   all socket connections type: 'kickall' or        |\n");
			printf("|   'closeall'.                                      |\n");
			printf("|                                                    |\n");
			printf("|   To quit the server type: 'quit' or 'exit'.       |\n");
			printf("------------------------------------------------------\n");
			fflush(stdout);
			continue;
		} else if ( strncasecmp(buffer, "kickall", 7) == 0 ||
				strncasecmp(buffer, "closeall", 8) == 0) {
			list_remove_all(l);	
		} else if ( strncasecmp(buffer, "kick", 4) == 0 ||
				strncasecmp(buffer, "close", 5) == 0) {
			char *token = strtok(buffer, " "), *addr, *sock;

			if (token != NULL) {
				token = strtok(NULL, " ");

				if (token == NULL) {
					printf("The command was executed without parameters. Type "
						   "'help' to see how to execute the command properly."
						   "\n");
					fflush(stdout);
					continue;
				} else {
					addr = token;	
				}

				token = strtok(NULL, "");

				if (token == NULL) {
					printf("The command was executed with too few parameters. Type "
						   "'help' to see how to execute the command properly."
						   "\n");
					fflush(stdout);
					continue;
				} else {
					sock = token;	
				}

				ws_client *n = list_get(l, addr, 
						strtol(sock, (char **) NULL, 10));

				if (n == NULL) {
					printf("The client that was supposed to receive the "
						   "message, was not found in the userlist.\n");
					fflush(stdout);
					continue;
				}

				ws_closeframe(n, CLOSE_SHUTDOWN);
			}
		} else if ( strncasecmp(buffer, "sendall", 7) == 0 ||
			   strncasecmp(buffer, "writeall", 8) == 0) {
			char *token = strtok(buffer, " ");
			ws_connection_close status;

			if (token != NULL) {
				token = strtok(NULL, "");

				if (token == NULL) {
					printf("The command was executed without parameters. Type "
						   "'help' to see how to execute the command properly."
						   "\n");
					fflush(stdout);
					continue;
				} else {
					ws_message *m = message_new();
					m->len = strlen(token);
					
					char *temp = malloc( sizeof(char)*(m->len+1) );
					if (temp == NULL) {
						raise(SIGINT);		
						break;
					}
					memset(temp, '\0', (m->len+1));
					memcpy(temp, token, m->len);
					m->msg = temp;
					temp = NULL;

					if ( (status = encodeMessage(m)) != CONTINUE) {
						message_free(m);
						free(m);
						raise(SIGINT);
						break;;
					}

					list_multicast_all(l, m);
					message_free(m);
					free(m);
				}
			}
		} else if ( strncasecmp(buffer, "send", 4) == 0 ||
				strncasecmp(buffer, "write", 5) == 0) {
			char *token = strtok(buffer, " "), *addr, *sock, *msg;
			ws_connection_close status;

			if (token != NULL) {
				token = strtok(NULL, " ");

				if (token == NULL) {
					printf("The command was executed without parameters. Type "
						   "'help' to see how to execute the command properly."
						   "\n");
					fflush(stdout);
					continue;
				} else {
					addr = token;	
				}

				token = strtok(NULL, " ");

				if (token == NULL) {
					printf("The command was executed with too few parameters. Type "
						   "'help' to see how to execute the command properly."
						   "\n");
					fflush(stdout);
					continue;
				} else {
					sock = token;	
				}

				token = strtok(NULL, "");
				
				if (token == NULL) {
					printf("The command was executed with too few parameters. Type "
						   "'help' to see how to execute the command properly."
						   "\n");
					fflush(stdout);
					continue;
				} else {
					msg = token;	
				}

				ws_client *n = list_get(l, addr, 
						strtol(sock, (char **) NULL, 10));

				if (n == NULL) {
					printf("The client that was supposed to receive the "
						   "message, was not found in the userlist.\n");
					fflush(stdout);
					continue;
				}

				ws_message *m = message_new();
				m->len = strlen(msg);
				
				char *temp = malloc( sizeof(char)*(m->len+1) );
				if (temp == NULL) {
					raise(SIGINT);		
					break;
				}
				memset(temp, '\0', (m->len+1));
				memcpy(temp, msg, m->len);
				m->msg = temp;
				temp = NULL;

				if ( (status = encodeMessage(m)) != CONTINUE) {
					message_free(m);
					free(m);
					raise(SIGINT);
					break;;
				}

				list_multicast_one(l, n, m);
				message_free(m);
				free(m);
			}
		} else {
			printf("To see functions available type: 'help'.\n");
			fflush(stdout);
			continue;
		}
	}

	pthread_exit((void *) EXIT_SUCCESS);
}

#if 0
void *handleClient(void *args) {
	pthread_detach(pthread_self());
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
	pthread_cleanup_push(&cleanup_client, args);

	int buffer_length = 0, string_length = 1, reads = 1;
        /*
         * TODO: onopen()
         */
	ws_client *n = args;
	n->thread_id = pthread_self();

	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

	char buffer[BUFFERSIZE];
	n->string = (char *) malloc(sizeof(char));

	if (n->string == NULL) {
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
		handshake_error("Couldn't allocate memory.", ERROR_INTERNAL, n);
		pthread_exit((void *) EXIT_FAILURE);
	}

	printf("Client connected with the following information:\n"
		   "\tSocket: %d\n"
		   "\tAddress: %s\n\n", n->socket_id, (char *) n->client_ip);
	printf("Checking whether client is valid ...\n\n");
	fflush(stdout);

	/**
	 * Getting headers and doing reallocation if headers is bigger than our
	 * allocated memory.
	 */
	do {
		memset(buffer, '\0', BUFFERSIZE);
		if ((buffer_length = recv(n->socket_id, buffer, BUFFERSIZE, 0)) <= 0){
			pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
			handshake_error("Didn't receive any headers from the client.", 
					ERROR_BAD, n);
			pthread_exit((void *) EXIT_FAILURE);
		}

		if (reads == 1 && strlen(buffer) < 14) {
			handshake_error("SSL request is not supported yet.", 
					ERROR_NOT_IMPL, n);
			pthread_exit((void *) EXIT_FAILURE);
		}

		string_length += buffer_length;

		char *tmp = realloc(n->string, string_length);
		if (tmp == NULL) {
			pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
			handshake_error("Couldn't reallocate memory.", ERROR_INTERNAL, n);
			pthread_exit((void *) EXIT_FAILURE);
		}
		n->string = tmp;
		tmp = NULL;

		memset(n->string + (string_length-buffer_length-1), '\0', 
				buffer_length+1);
		memcpy(n->string + (string_length-buffer_length-1), buffer, 
				buffer_length);
		reads++;
	} while( strncmp("\r\n\r\n", n->string + (string_length-5), 4) != 0 
			&& strncmp("\n\n", n->string + (string_length-3), 2) != 0
			&& strncmp("\r\n\r\n", n->string + (string_length-8-5), 4) != 0
			&& strncmp("\n\n", n->string + (string_length-8-3), 2) != 0 );
	
	printf("User connected with the following headers:\n%s\n\n", n->string);
	fflush(stdout);

	ws_header *h = header_new();

	if (h == NULL) {
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
		handshake_error("Couldn't allocate memory.", ERROR_INTERNAL, n);
		pthread_exit((void *) EXIT_FAILURE);
	}

	n->headers = h;

	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
	if ( parseHeaders(n->string, n, port) < 0 ) {
		pthread_exit((void *) EXIT_FAILURE);
	}

	if ( sendHandshake(n) < 0 && n->headers->type != UNKNOWN ) {
		pthread_exit((void *) EXIT_FAILURE);	
	}	

	list_add(l, n);
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

	printf("Client has been validated and is now connected\n\n");
	printf("> ");
	fflush(stdout);

	uint64_t next_len = 0;
	char next[BUFFERSIZE];
	memset(next, '\0', BUFFERSIZE);

	while (1) {
		if ( communicate(n, next, next_len) != CONTINUE) {
			break;
		}

		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
		if (n->headers->protocol == CHAT) {
			list_multicast(l, n);
		} else if (n->headers->protocol == ECHO) {
			list_multicast_one(l, n, n->message);
                        print_dbg("protocol = ECHO(%d)\n", ECHO);
		} else {
                        /* 
                         * TODO: void onmessage(ws_message*)
                         */
                        print_dbg("ws_client = \n");
                        print_dbg("          client_ip: %s:\n", n->client_ip);
                        print_dbg("          message.enc: \n");
                        print_buf(n->message->enc, n->message->enc_len);
                        print_dbg("          message->opcode: 0x%x\n", n->message->opcode[0]&0xf);
                        print_dbg("          message->len   : %d\n", n->message->len);
                        print_buf(n->message->msg, n->message->len);
                        list_multicast_one(l, n, n->message);
		}
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

		if (n->message != NULL) {
			memset(next, '\0', BUFFERSIZE);
			memcpy(next, n->message->next, n->message->next_len);
			next_len = n->message->next_len;
			message_free(n->message);
			free(n->message);
			n->message = NULL;	
		}
	}
	
        /*TODO: onclose() */
	print_info("Shutting client down..\n\n");
	printf("> ");
	fflush(stdout);

	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
	list_remove(l, n);
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

	pthread_cleanup_pop(0);
	pthread_exit((void *) EXIT_SUCCESS);
}
int main(int argc, char *argv[]) {
	int server_socket, client_socket, on = 1;
	
	struct sockaddr_in server_addr, client_addr;
	socklen_t client_length;
	pthread_t pthread_id;
	pthread_attr_t pthread_attr;

	/**
	 * Creating new lists, l is supposed to contain the connected users.
	 */
	l = list_new();

	/**
	 * Listens for CTRL-C and Segmentation faults.
	 */ 
	(void) signal(SIGINT, &sigint_handler);
	(void) signal(SIGSEGV, &sigint_handler);
	(void) signal(SIGPIPE, &sigint_handler);


	printf("Server: \t\tStarted\n");
	fflush(stdout);

	/**
	 * Assigning port value.
	 */
	if (argc == 2) {
		port = strtol(argv[1], (char **) NULL, 10);
		
		if (port <= 1024 || port >= 65565) {
			port = PORT;
		}

	} else {
		port = PORT;	
	}

	printf("Port: \t\t\t%d\n", port);
	fflush(stdout);

	/**
	 * Opening server socket.
	 */
	if ( (server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
		server_error(strerror(errno), server_socket, l);
	}

	printf("Socket: \t\tInitialized\n");
	fflush(stdout);

	/**
	 * Allow reuse of address, when the server shuts down.
	 */
	if ( (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &on, 
					sizeof(on))) < 0 ){
		server_error(strerror(errno), server_socket, l);
	}

	printf("Reuse Port %d: \tEnabled\n", port);
	fflush(stdout);

	memset((char *) &server_addr, '\0', sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(port);

	printf("Ip Address: \t\t%s\n", inet_ntoa(server_addr.sin_addr));
	fflush(stdout);

	/**
	 * Bind address.
	 */
	if ( (bind(server_socket, (struct sockaddr *) &server_addr, 
			sizeof(server_addr))) < 0 ) {
		server_error(strerror(errno), server_socket, l);
	}

	printf("Binding: \t\tSuccess\n");
	fflush(stdout);

	/**
	 * Listen on the server socket for connections
	 */
	if ( (listen(server_socket, 10)) < 0) {
		server_error(strerror(errno), server_socket, l);
	}

	printf("Listen: \t\tSuccess\n\n");
	fflush(stdout);

	/**
	 * Attributes for the threads we will create when a new client connects.
	 */
	pthread_attr_init(&pthread_attr);
	pthread_attr_setdetachstate(&pthread_attr, PTHREAD_CREATE_DETACHED);
	pthread_attr_setstacksize(&pthread_attr, 524288);

	printf("Server is now waiting for clients to connect ...\n\n");
	fflush(stdout);

	/**
	 * Create commandline, such that we can do simple commands on the server.
	 */
	if ( (pthread_create(&pthread_id, &pthread_attr, cmdline, NULL)) < 0 ){
		server_error(strerror(errno), server_socket, l);
	}

	/**
	 * Do not wait for the thread to terminate.
	 */
	pthread_detach(pthread_id);

	while (1) {
		client_length = sizeof(client_addr);
		
		/**
		 * If a client connects, we observe it here.
		 */
		if ( (client_socket = accept(server_socket, 
				(struct sockaddr *) &client_addr,
				&client_length)) < 0) {
			server_error(strerror(errno), server_socket, l);
		}

		/**
		 * Save some information about the client, which we will
		 * later use to identify him with.
		 */
		char *temp = (char *) inet_ntoa(client_addr.sin_addr);
		char *addr = (char *) malloc( sizeof(char)*(strlen(temp)+1) );
		if (addr == NULL) {
			server_error(strerror(errno), server_socket, l);
			break;
		}
		memset(addr, '\0', strlen(temp)+1);
	    memcpy(addr, temp, strlen(temp));	

		ws_client *n = client_new(client_socket, addr);

		/**
		 * Create client thread, which will take care of handshake and all
		 * communication with the client.
		 */
		if ( (pthread_create(&pthread_id, &pthread_attr, handleClient, 
						(void *) n)) < 0 ){
			server_error(strerror(errno), server_socket, l);
		}

		pthread_detach(pthread_id);
	}

	list_free(l);
	l = NULL;
	close(server_socket);
	pthread_attr_destroy(&pthread_attr);
	return EXIT_SUCCESS;
}
#endif

void *handleClient_2(void *args) {
        pthread_detach(pthread_self());
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
        pthread_cleanup_push(&cleanup_client, args);

        int buffer_length = 0, string_length = 1, reads = 1;

        /*
         * TODO: onopen()
         */
        struct IWebsocket *iwebsocket;
        struct argsthread{
            ws_client         *wsclient;
            struct IWebsocket *iwebsocket;
        } *pargsthread;
        pargsthread  = args;
        iwebsocket   = pargsthread->iwebsocket;
        ws_client *n = pargsthread->wsclient;
        /* add interface function */
        iwebsocket->onopen(n);
        
  
        n->thread_id = pthread_self();

        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

        char buffer[BUFFERSIZE];
        n->string = (char *) malloc(sizeof(char));

        if (n->string == NULL) {
                pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
                handshake_error("Couldn't allocate memory.", ERROR_INTERNAL, n);
                pthread_exit((void *) EXIT_FAILURE);
        }
#ifdef INVG_RELEASE
        printf("Client connected with the following information:\n"
                   "\tSocket: %d\n"
                   "\tAddress: %s\n\n", n->socket_id, (char *) n->client_ip);
        printf("Checking whether client is valid ...\n\n");
#endif
        fflush(stdout);

        /**
         * Getting headers and doing reallocation if headers is bigger than our
         * allocated memory.
         */
        do {
                memset(buffer, '\0', BUFFERSIZE);
                if ((buffer_length = recv(n->socket_id, buffer, BUFFERSIZE, 0)) <= 0){
                        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
                        handshake_error("Didn't receive any headers from the client.", 
                                        ERROR_BAD, n);
                        pthread_exit((void *) EXIT_FAILURE);
                }

                if (reads == 1 && strlen(buffer) < 14) {
                        handshake_error("SSL request is not supported yet.", 
                                        ERROR_NOT_IMPL, n);
                        pthread_exit((void *) EXIT_FAILURE);
                }

                string_length += buffer_length;

                char *tmp = realloc(n->string, string_length);
                if (tmp == NULL) {
                        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
                        handshake_error("Couldn't reallocate memory.", ERROR_INTERNAL, n);
                        pthread_exit((void *) EXIT_FAILURE);
                }
                n->string = tmp;
                tmp = NULL;

                memset(n->string + (string_length-buffer_length-1), '\0', 
                                buffer_length+1);
                memcpy(n->string + (string_length-buffer_length-1), buffer, 
                                buffer_length);
                reads++;
        } while( strncmp("\r\n\r\n", n->string + (string_length-5), 4) != 0 
                        && strncmp("\n\n", n->string + (string_length-3), 2) != 0
                        && strncmp("\r\n\r\n", n->string + (string_length-8-5), 4) != 0
                        && strncmp("\n\n", n->string + (string_length-8-3), 2) != 0 );
#ifdef INVG_RELEASE
        printf("User connected with the following headers:\n%s\n\n", n->string);
#endif
        fflush(stdout);

        ws_header *h = header_new();

        if (h == NULL) {
                pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
                handshake_error("Couldn't allocate memory.", ERROR_INTERNAL, n);
                pthread_exit((void *) EXIT_FAILURE);
        }

        n->headers = h;

        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
        if ( parseHeaders(n->string, n, port) < 0 ) {
                pthread_exit((void *) EXIT_FAILURE);
        }

        if ( sendHandshake(n) < 0 && n->headers->type != UNKNOWN ) {
                pthread_exit((void *) EXIT_FAILURE);    
        }       

        list_add(l, n);
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

        printf("Client has been validated and is now connected\n\n");
        printf("> \n");
        
        if(iwebsocket->max_client > 0){
            print_dbg("max_client= %d, count_client=%d\n", iwebsocket->max_client, count_client);
            if(count_client > iwebsocket->max_client){
                print_err("max_client= %d, count_client=%d\n", iwebsocket->max_client, count_client);
                ws_send_text(n, "Too many client. Server break this connection");
                goto _exit_handleclient;
            }
        }
        
        fflush(stdout);

        uint64_t next_len = 0;
        char next[BUFFERSIZE];
        memset(next, '\0', BUFFERSIZE);

        while (1) {
                if ( communicate(n, next, next_len) != CONTINUE) {
                        goto _exit_handleclient;
                }

                pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
                if (n->headers->protocol == CHAT) {
                        list_multicast(l, n);
                } else if (n->headers->protocol == ECHO) {
                        list_multicast_one(l, n, n->message);
                        print_dbg("protocol = ECHO(%d)\n", ECHO);
                } else {
                        /* 
                         * TODO: void onmessage(ws_message*)
                         */
                        iwebsocket->onmessage(n, n->message);
                        //list_multicast_one(l, n, n->message);
                }

                if (n->message != NULL) {
                        memset(next, '\0', BUFFERSIZE);
                        memcpy(next, n->message->next, n->message->next_len);
                        next_len = n->message->next_len;
                        message_free(n->message);
                        free(n->message);
                        n->message = NULL;      
                }
                pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        }
        
_exit_handleclient:
        /*
         * TODO: onclose() 
         */
        iwebsocket->onclose(n);
        print_dbg("Shutting client down..\n\n");
        printf("> \n");
        fflush(stdout);

        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
        list_remove(l, n);
        count_client--;
        print_dbg("count_client -- \n\n");
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

        pthread_cleanup_pop(0);
        pthread_exit((void *) EXIT_SUCCESS);
}
int start_websocket_server(struct IWebsocket *iwebsocket) {
        if(iwebsocket == NULL){
            print_err("is NULL\n");
            return -1;
        }
        int server_socket, client_socket, on = 1;
        
        struct sockaddr_in server_addr, client_addr;
        socklen_t client_length;
        pthread_t pthread_id;
        pthread_attr_t pthread_attr;

        /**
         * Creating new lists, l is supposed to contain the connected users.
         */
        l = list_new();

        /**
         * Listens for CTRL-C and Segmentation faults.
         */ 
        (void) signal(SIGINT, &sigint_handler);
        (void) signal(SIGSEGV, &sigint_handler);
        (void) signal(SIGPIPE, &sigint_handler);


        print_dbg("Server: \t\t\tStarted\n");
        fflush(stdout);

        /**
         * Assigning port value.
         */
        port = iwebsocket->port;
        if (port <= 1024 || port >= 65565) {
            port = PORT;
        }

        print_dbg("Port: \t\t\t%d\n", port);
        fflush(stdout);

        /**
         * Opening server socket.
         */
        if ( (server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
                server_error(strerror(errno), server_socket, l);
        }

        print_dbg("Socket: \t\t\tInitialized\n");
        fflush(stdout);

        /**
         * Allow reuse of address, when the server shuts down.
         */
        if ( (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &on, 
                                        sizeof(on))) < 0 ){
                server_error(strerror(errno), server_socket, l);
        }

        print_dbg("Reuse Port %d: \tEnabled\n", port);
        fflush(stdout);

        memset((char *) &server_addr, '\0', sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        server_addr.sin_port = htons(port);

        print_dbg("Ip Address: \t\t%s\n", inet_ntoa(server_addr.sin_addr));
        fflush(stdout);

        /**
         * Bind address.
         */
        if ( (bind(server_socket, (struct sockaddr *) &server_addr, 
                        sizeof(server_addr))) < 0 ) {
                server_error(strerror(errno), server_socket, l);
        }

        print_dbg("Binding: \t\tSuccess\n");
        fflush(stdout);

        /**
         * Listen on the server socket for connections
         */
        if ( (listen(server_socket, 10)) < 0) {
                server_error(strerror(errno), server_socket, l);
        }

        print_dbg("Listen: \t\t\tSuccess\n\n");
        fflush(stdout);

        /**
         * Attributes for the threads we will create when a new client connects.
         */
        pthread_attr_init(&pthread_attr);
        pthread_attr_setdetachstate(&pthread_attr, PTHREAD_CREATE_DETACHED);
        pthread_attr_setstacksize(&pthread_attr, 524288);

        print_dbg("Server is now waiting for clients to connect ...\n\n");
        fflush(stdout);

        /**
         * Create commandline, such that we can do simple commands on the server.
         */
        if(iwebsocket->bNeed_stdinput_for_test == 1){
            if ( (pthread_create(&pthread_id, &pthread_attr, cmdline, NULL)) < 0 ){
                server_error(strerror(errno), server_socket, l);
            }
            /**
             * Do not wait for the thread to terminate.
             */
            pthread_detach(pthread_id);
        }


        while (1) {
                client_length = sizeof(client_addr);
                
                /**
                 * If a client connects, we observe it here.
                 */
                if ( (client_socket = accept(server_socket, (struct sockaddr *) &client_addr, &client_length)) < 0) {
                        server_error(strerror(errno), server_socket, l);
                }

                count_client++;

                /**
                 * Save some information about the client, which we will
                 * later use to identify him with.
                 */
                char *temp = (char *) inet_ntoa(client_addr.sin_addr);
                char *addr = (char *) malloc( sizeof(char)*(strlen(temp)+1) );
                if (addr == NULL) {
                        server_error(strerror(errno), server_socket, l);
                        break;
                }
                memset(addr, '\0', strlen(temp)+1);
                memcpy(addr, temp, strlen(temp));   
                
                ws_client *n = client_new(client_socket, addr);

                /**
                 * Create client thread, which will take care of handshake and all
                 * communication with the client.
                 */
                struct argsthread{
                    ws_client         *wsclient;
                    struct IWebsocket *iwebsocket;
                } _argsthread;
                _argsthread.wsclient   = n;
                _argsthread.iwebsocket = iwebsocket;
                if ( (pthread_create(&pthread_id, &pthread_attr, handleClient_2, (void *) &_argsthread)) < 0 ){
                        server_error(strerror(errno), server_socket, l);
                }

                pthread_detach(pthread_id);
        }

        list_free(l);
        l = NULL;
        close(server_socket);
        pthread_attr_destroy(&pthread_attr);
        return EXIT_SUCCESS;
}

// #ifdef TEST_MAIN_PC
#if 1
struct key_value{
    char key[20];
    char value[20];
    char *_tmp;
};

#define MAX_NUM_KV_STR      5
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
static void parseCmdParamStr2KeyValue(char *token, struct key_value *a_kv){
    int i = 0, j = 0;
    char *p = NULL;
    token = strtok(token, "/?");
    print_dbg("cmd: %s\n", token);
    if(token != NULL){
        if((token = strtok(NULL, "?")) != NULL){
            print_dbg("params: %s\n", token);
            p = strtok(token, "&");
            while(i<MAX_NUM_KV_STR && p != NULL){
                //print_dbg("p = %s\n", p);
                a_kv[i++]._tmp= p;
                p = strtok(NULL, "&");
            }
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

void parseHttpHeadersGetStr2KeyValues(char *headers_get,struct key_value *a_kv){
    memset(a_kv, 0, sizeof (a_kv)/sizeof(a_kv[0]));
    char *s = strdup(headers_get);
    char *token = strtok(s, " ");
    
    if(token != NULL){
        token = strtok(NULL, " ");
        if(token!=NULL){
            print_dbg("token = %s\n", token);
            parseCmdParamStr2KeyValue(token, a_kv);
        }
    }
    free(s);
}

void *onopen(ws_client *wsclient){
    __pBegin
    __pEnd
}
void *onclose(ws_client *wsclient){
    __pBegin
    __pEnd
}
/*
 * don't need to free message.It will free after onmessage
 */
void *onmessage(ws_client *n, ws_message *message){
    __pBegin
    /* here: ws_client->message == message */
    print_info("Received(%s): %s\n",n->client_ip , message->msg);
    ws_send_text(n, message->msg);
    ws_send_text_all(message->msg);
    
    struct key_value a_kv[MAX_NUM_KV_STR];
    parseCmdParamStr2KeyValue(message->msg, a_kv);
    for(int j = 0; a_kv[j].key[0]!=0; j++){
        print_dbg("a_kv[%d]: {%s: %s}\n", j,a_kv[j].key, a_kv[j].value);
    }
    
//     parseHttpHeadersGetStr2KeyValues(n->headers->get, a_kv);
//     for(int j = 0; a_kv[j].key[0]!=0; j++){
//         print_dbg("a_kv[%d]: {%s: %s}\n", j,a_kv[j].key, a_kv[j].value);
//     }
    

    __pEnd
}


/*
 *see also another websocket: https://github.com/payden/libwebsock
 */
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
#endif