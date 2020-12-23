#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <signal.h>
#include <sys/socket.h>
#include "../utils/cfgutils.h"
#include "../utils/netutils.h"
#include "../utils/strutils.h"
#include "../dast/strlist.h"
#include "../dast/map.h"
#include "../utils/symbols.h"
#include "container.h"
#include "server.h"
/*
.serconf
key v1(is running)  v2(pid)  v3(port)
stat    running pid port
*/
static bool process_cmd_new(struct strlist* cmd, int sockfd);
static bool process_cmd_insert(struct strlist* cmd, int sockfd);
static bool process_cmd_remove(struct strlist* cmd, int sockfd);
static bool process_cmd_del(struct strlist* cmd, int sockfd);
static bool process_cmd_get(struct strlist* cmd, int sockfd);
static bool process_cmd_err(char* info, int sockfd);
static bool process_cmd_print(struct strlist* cmd, int sockfd);
/*服务器全局变量*/
static struct container* ctr_map_s;
static struct container* ctr_umap_s;
static struct container* ctr_umap_i;
static struct umap_i* type_map; /*记录了建立的数据结构名称与类型的映射关系*/
static bool is_item_exist(char* iname);
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
    type_map = create_umap_i();
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

bool process_cmd(char* cmdlist, int sockfd)
{
    struct strlist* cmd_list, *q;
    struct item* it;
    struct int_check ic;
    void* map;
    cmd_list = separate_strs(cmdlist, ' ');
    q = cmd_list->next;
    if(q == NULL)
    {
        msg_to_sock(sockfd, "false");
        return false;
    }
    else if(strcmp(q->str, "new") == 0) return process_cmd_new(q->next, sockfd);
    else if(strcmp(q->str, "insert") == 0) return process_cmd_insert(q->next, sockfd);
//    else if(strcmp(q->str, "remove") == 0) return process_cmd_remove(q->next, sockfd);/*删除map中的一个或多个entry,取决于是否unique*/
//    else if(strcmp(q->str, "del") == 0) return process_cmd_del(q->next, sockfd);/*删除整个map*/
//    else if(strcmp(q->str, "get") == 0) return process_cmd_get(q->next, sockfd);
    else if(strcmp(q->str, "print") == 0) return process_cmd_print(q->next, sockfd);
    else return process_cmd_err("ERR_FMT", sockfd);

}
bool is_item_exist(char* iname)
{
    struct int_check ic = getv_umap_i(type_map, iname);
    switch(ic.v)
    {
    case TMAPS:
        return get_item_by_name(ctr_map_s->item_list, iname) == NULL ? false : true;
        break;
    case TUMAPS:
        return get_item_by_name(ctr_umap_s->item_list, iname) == NULL ? false : true;
        break;
    case TUMAPI:
        return get_item_by_name(ctr_umap_i->item_list, iname) == NULL ? false : true;
        break;
    }
    return false;
}
bool process_cmd_new(struct strlist* cmd, int sockfd)
{
    /*new mapname [umaps|umapi|maps]*/
    char* map_name;
    if(cmd == NULL) {return process_cmd_err("ERR_FMT", sockfd);}
    else if(is_item_exist(cmd->str) == true) {return process_cmd_err("ITEM_EXIST", sockfd);}
    else
    {
        map_name = cmd->str;
        cmd = cmd->next;
        if(cmd == NULL) {return process_cmd_err("ERR_FMT", sockfd);}
        else if(strcmp(cmd->str, "maps") == 0)
        {
            add_item(ctr_map_s, (void*)create_map_s(), map_name);
            insert_umap_i(type_map, map_name, TMAPS);/*更新变量-类型映射表*/
        }
        else if(strcmp(cmd->str, "umaps") == 0)
        {
            add_item(ctr_umap_s, (void*)create_umap_s(), map_name);
            insert_umap_i(type_map, map_name, TUMAPS);
        }
        else if(strcmp(cmd->str, "umapi") == 0)
        {
            add_item(ctr_umap_i, (void*)create_umap_s(), map_name);
            insert_umap_i(type_map, map_name, TUMAPI);
        }
        else
        {
            return process_cmd_err("ERR_FMT", sockfd);
        }
        msg_to_sock(sockfd, "true");
        return true;
    }
}
bool process_cmd_insert(struct strlist* cmd, int sockfd)
{
    /*insert map_name key value */
    struct int_check ic;
    struct item* it;
    char* key, *value;
    int ivalue;
    void* map;
    if(cmd == NULL) {return process_cmd_err("ERR_FMT", sockfd);}
    else if(is_item_exist(cmd->str) == false) {return process_cmd_err("ITEM_NOT_EXIST", sockfd);}
    else
    {
        ic = getv_umap_i(type_map, cmd->str);
        switch(ic.v)
        {
        case TMAPS:
            it = get_item_by_name(ctr_map_s->item_list, cmd->str);
            map = it->_item;
            /*继续next两层获取key & value*/
            cmd = cmd->next;
            if(cmd == NULL){return process_cmd_err("ERR_FMT", sockfd);}
            else
            {
                key = cmd->str;/*保存待插入key*/
                cmd = cmd->next;
                if(cmd == NULL){return process_cmd_err("ERR_FMT", sockfd);}
                else
                {
                    value = cmd->str;/*保存待插入value*/
                    insert_map_s((struct map_s*)map, key, value);
                    msg_to_sock(sockfd, "true");
                    return true;
                }
            }
            break;
        case TUMAPS:
            it = get_item_by_name(ctr_umap_s->item_list, cmd->str);
            map = it->_item;
            /*继续next两层获取key & value*/
            cmd = cmd->next;
            if(cmd == NULL){return process_cmd_err("ERR_FMT", sockfd);}
            else
            {
                key = cmd->str;/*保存待插入key*/
                cmd = cmd->next;
                if(cmd == NULL){return process_cmd_err("ERR_FMT", sockfd);}
                else
                {
                    value = cmd->str;/*保存待插入value*/
                    if(insert_umap_s((struct map_s*)map, key, value) == false)
                    {
                        return process_cmd_err("ENTRY_SAME_UNIQUE", sockfd);/*try to insert a same entry in a unique map*/
                    }
                    msg_to_sock(sockfd, "true");
                    return true;
                }
            }
            break;
        case TUMAPI:
            it = get_item_by_name(ctr_umap_i->item_list, cmd->str);
            map = it->_item;
            /*继续next两层获取key & value*/
            cmd = cmd->next;
            if(cmd == NULL){return process_cmd_err("ERR_FMT", sockfd);}
            else
            {
                key = cmd->str;/*保存待插入key*/
                cmd = cmd->next;
                if(cmd == NULL){return process_cmd_err("ERR_FMT", sockfd);}
                else
                {
                    ivalue = atoi(cmd->str);/*保存待插入value*/
                    if(insert_umap_i((struct umap_i*)map, key, ivalue) == false)
                    {
                        return process_cmd_err("ENTRY_SAME_UNIQUE", sockfd);
                    }
                    msg_to_sock(sockfd, "true");
                    return true;
                }
            }
            break;
        }
    }
}
bool process_cmd_print(struct strlist* cmd, int sockfd)
{
    char print_buf[BUFSIZ];
    struct int_check ic;
    char* key, *value;
    struct item* it;
    struct map_s* maps;
    struct entry_s* ets;
    void* vmap;
    struct umap_i* umapi;
    int ivalue, i;
    if(cmd == NULL)
    {
        return process_cmd_err("ERR_FMT", sockfd);
    }
    else if(is_item_exist(cmd->str) == false)
    {
        return process_cmd_err("ITEM_NOT_EXIST", sockfd);
    }
    else
    {
        ic = getv_umap_i(type_map, cmd->str);
        switch(ic.v)
        {
        case TMAPS:
        case TUMAPS:
            /*打印MAPS和UMAPS是一样的 不需要区别处理*/
            if((it = get_item_by_name(ctr_map_s->item_list, cmd->str)) == NULL)
            {
                it = get_item_by_name(ctr_umap_s->item_list, cmd->str);
            }
            vmap = it->_item;
            maps = (struct map_s*)vmap;
            for(i = 0; i < SIZHASHTAB; i++)
            {
                ets = maps->entries[i]->next;
                while(ets != NULL)
                {
                    printf("\n<%s,%s> in[%d]\n", ets->key, ets->value, i);
                    ets = ets->next;
                }
            }
            msg_to_sock(sockfd, "true");
            return true;
        }
    }
}
bool process_cmd_err(char* info, int sockfd)
{
    char* buf = (char*)malloc(strlen(info) + strlen("false ") + 1);
    strcpy(buf, "false ");
    strcat(buf, info);
    msg_to_sock(sockfd, buf);
    return false;
}
void msg_to_sock(int sockfd, char* msg)
{
    write(sockfd, msg, strlen(msg));
}
void show_help_info(int sockfd)
{
    char help[] = "Correct format: \nnew [maps|umaps|umapi] name\n";
    write(sockfd, help, strlen(help));
}
void test_server()
{
    //init_server();
    ctr_map_s = create_container();
    ctr_umap_s = create_container();
    ctr_umap_i = create_container();
    type_map = create_umap_i();
    process_cmd("new m1 maps", 1);
    process_cmd("insert m1 angela baby", 1);
    process_cmd("insert m1 nisha shark", 1);
    process_cmd("insert m1 apple pie", 1);
    process_cmd("print m1", 1);
    #if 0
    struct item* it = get_item_by_name(ctr_umap_i->item_list, "m1");
    struct umap_i* ms = (struct map_s*)it->_item;
    insert_umap_i(ms, "angela", 100);
    process_cmd("get m1 angela", 1);
    #endif
    //process_cmd("insert m1 huge fs", 1);
    //process_cmd("insert m1 angela ga", 1);
    //process_cmd("print m1", 2);
}
