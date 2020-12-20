#ifndef _SERVER_H_
#define _SERVER_H_
#define SER_CFG_NAME ".serconf"
#define DEFAULT_PORT "1993"
int init_server(void);
void serv_do_hello(int sockfd);
void test_server(void);
void quit_handler(int signum);
void* process_req(void*);
void res_free_handler(int signum);
#endif // _SERVER_H_
