#ifndef _SERVER_H_
#define _SERVER_H_
#define SER_CFG_NAME ".serconf"
#define DEFAULT_PORT "1993"
int init_server(void);
void serv_do_hello(int sockfd);
void test_server(void);
void quit_handler(int signum);
void* process_req(void*);
bool process_cmd(char* cmd, int sockfd);
void res_free_handler(int signum);
bool msg_to_sock(int sockfd, char* msg);
void show_help_info(int sockfd);
#endif // _SERVER_H_
