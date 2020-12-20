#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <signal.h>
#include <sys/socket.h>
#include "../utils/cfgutils.h"
#include "../utils/netutils.h"
#include "../dast/strlist.h"
#include "../dast/map.h"
#include "container.h"
#include "server.h"
/*
.serconf
key v1(is running)  v2(pid)  v3(port)
stat    running pid port
*/
static struct container* ctr_map_s;
static struct container* ctr_umap_s;
static struct container* ctr_umap_i;
struct SOCKFD
{
    int sockfd;
};
int init_server()
{
    /*
    exit 1: sock creat failed
    exit 2: server has already started
    exit 3: wrong fd from accept
    */
    struct strlist* vs;
    struct SOCKFD* sf;
    int sock, sockfd;
    struct stat st;
    pthread_t tid;
    char* value;
    char pid[BUFSIZ];
    sprintf(pid, "%d", getpid());
    sf = (struct SOCKFD*)malloc(sizeof(struct SOCKFD));
    sock = get_serv_sock(atoi(DEFAULT_PORT), SOCK_STREAM);
    if(sock == -1) exit(1);
    vs = create_str_list();
    /*检查配置目录是否存在*/
    if(stat(CONFIGS_PATH, &st) == -1) mkdir(CONFIGS_PATH, 0644);
    /*检查配置文件是否存在*/
    if(is_cfg_existed(SER_CFG_NAME))
    {
        /*配置文件已经存在 检查允许状态*/
        value = get_cfg_value(SER_CFG_NAME, "stat", 1);
        if(strcmp(value, "running") == 0)
        {
            free(value);
            close(sock);
            printf("%d server has already started...exit...\n", sock);
            exit(1);
        }
        else
        {
            update_cfg_value(SER_CFG_NAME, "stat", "running", 1);
            printf("Server start on: pid [%s], port[%s]\n", pid, DEFAULT_PORT);
        }
    }
    else
    {
        /*配置文件不存在，创建并且写入允许状态*/
        create_cfg(SER_CFG_NAME);
        printf("Server start: with pid [%s], port[%s]\n", pid, DEFAULT_PORT);
        insert_back(vs, "running");
        insert_back(vs, pid);
        insert_back(vs, DEFAULT_PORT);
        //insert_back(vs, ctime(time(NULL)));
        insert_cfg_record(SER_CFG_NAME, "stat", vs);
        clear_all(vs);
        free(vs);
    }
    /*设置关闭处理函数*/
    signal(SIGINT, quit_handler);
    /*设置定期资源清理*/
    signal(SIGALRM, res_free_handler);
    struct itimerval itv;
    itv.it_interval.tv_sec = 60*60*60*12;/*12 hours*/
    itv.it_interval.tv_usec = 0;
    itv.it_value.tv_sec = 60*60*60*12;
    itv.it_value.tv_usec = 0;
    setitimer(ITIMER_REAL, &itv, NULL);
    /*sock正常获取，配置文件写入成功 开始初始化容器*/
    ctr_map_s = create_container();
    ctr_umap_s = create_container();
    ctr_umap_i = create_container();

    /*容器初始化成功， 开始接收请求*/

    while(true)
    {
        sockfd = accept(sock, NULL, NULL);
        if(sockfd == -1)
        {
           exit(3);
        }
        sf->sockfd = sockfd;
        pthread_create(&tid, NULL, process_req, (void*)sf);/*远程服务*/
    }
}
void quit_handler(int signum)
{
    /*关闭*/
    update_cfg_value(SER_CFG_NAME, "stat", "stop", 1);
    free_all_items(ctr_map_s, MAPS);
    free_all_items(ctr_umap_i, UMAPI);
    free_all_items(ctr_umap_s, UMAPS);
    exit(0);
}
void serv_do_hello(int sockfd)
{
    char str[] = "Welcome to Giraffe DB I got you!";
    write(sockfd, str, strlen(str+1));
}
void res_free_handler(int signum)
{
    free_all_items(ctr_map_s, MAPS);
    free_all_items(ctr_umap_i, UMAPI);
    free_all_items(ctr_umap_s, UMAPS);
}
void* process_req(void* sinfo)
{
    int sockfd = ((struct SOCKFD*)sinfo)->sockfd;
    char buf[BUFSIZ];
    read(sockfd, buf, BUFSIZ);
    if(strcmp(buf, "hello") == 0)
    {
        serv_do_hello(sockfd);
    }
    else if(strcmp(buf, "map<") == 0)
    {
        ;/*处理其它的shell输入*/
    }
    else if(strcmp(buf, "stop server") == 0)
    {
        quit_handler(0);
    }
    return NULL;
}
void test_server()
{
    init_server();
}
