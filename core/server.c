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
#if 0

bool process_cmd(char* cmd, int sockfd)
{
    /*
    new maps name; new umaps name; new umapi name;
    */
    if(cmd == NULL | strlen(cmd) == 0) return false;
    int i, j;
    struct map_s* maps;
    char* cbuf;
    struct umap_i* mapi;
    struct item* it;
    char buf[BUFSIZ];
    struct strlist* cmd_list, *p, *q, *vs;
    cmd_list = create_str_list();
    i = 0;
    j = 0;
    /*装载命令的每一个单词进strlist*/
    while(cmd[i] != '\0')
    {
        if(cmd[i] == ' ')
        {
            buf[j] = '\0';
            insert_back(cmd_list, buf);
            j = 0;
        }
        else
        {
            buf[j] = cmd[i];
            j++;
        }
        i++;
    }
    if(j != 0)
    {
        buf[j] = '\0';
        insert_back(cmd_list, buf);
    }
    /*逐个分析命令并且执行不同操作*/
    p = cmd_list;
    if((p = get_next_item(p)) != NULL)
    {
        if(strcmp(p->str, "new") == 0)
        {
            /*创建map*/
            p = p->next;
            if(p->next == NULL)
            {
                /*错误的格式*/
                show_help_info(sockfd);
                clear_all(cmd_list);
                free(cmd_list);
                return false;
            }
            else if(strcmp(p->str, "maps") == 0)
            {
                maps = create_map_s();
                p = p->next;
                if(p == NULL | strlen(p->str) < 1)
                {
                    show_help_info(sockfd);
                    clear_all(cmd_list);
                    free(cmd_list);
                    return false;
                }
                else
                {
                    if(is_item_exist(ctr_map_s, p->str) == true || is_item_exist(ctr_umap_s, p->str) == true || is_item_exist(ctr_umap_i, p->str) == true)
                    {
                        /*已有同名map*/
                        msg_to_sock(sockfd, "item is already exist in db");
                        clear_all(cmd_list);
                        free(cmd_list);
                        return false;
                    }
                    else
                    {
                        add_item(ctr_map_s, (void*)maps, p->str);
                    }
                }
            }
            else if(strcmp(p->str, "umaps") == 0)
            {
                maps = create_umap_s();
                p = p->next;
                if(p == NULL | strlen(p->str) < 1)
                {
                    show_help_info(sockfd);
                    clear_all(cmd_list);
                    free(cmd_list);
                    return false;
                }
                else
                {
                    if(is_item_exist(ctr_umap_s, p->str) == true || is_item_exist(ctr_map_s, p->str) == true || is_item_exist(ctr_umap_i, p->str) == true)
                    {
                        /*已有同名map*/
                        msg_to_sock(sockfd, "item is already exist in db");
                        clear_all(cmd_list);
                        free(cmd_list);
                        return false;
                    }
                    else
                    {
                        add_item(ctr_umap_s, (void*)maps, p->str);
                    }
                }
            }
            else if(strcmp(p->str, "umapi") == 0)
            {
                mapi = create_umap_i();
                p = p->next;
                if(p == NULL | strlen(p->str) < 1)
                {
                    show_help_info(sockfd);
                    clear_all(cmd_list);
                    free(cmd_list);
                    return false;
                }
                else
                {
                    if(is_item_exist(ctr_umap_i, p->str) == true || is_item_exist(ctr_map_s, p->str) == true || is_item_exist(ctr_map_s, p->str) == true)
                    {
                        /*已有同名map*/
                        msg_to_sock(sockfd, "item is already exist in db");
                        clear_all(cmd_list);
                        free(cmd_list);
                        return false;
                    }
                    else
                    {
                        add_item(ctr_umap_i, (void*)mapi, p->str);
                    }
                }
            }
            else
            {
                show_help_info(sockfd);
                clear_all(cmd_list);
                free(cmd_list);
                return false;
            }
        }
        else if(strcmp(p->str, "delete") == 0)
        {
            /*删除map*/
            p = p->next;
            if(p == NULL | strlen(p->str) < 1)
            {
                show_help_info(sockfd);
                clear_all(cmd_list);
                return false;
            }
            else
            {
                delete_item(ctr_map_s, MAPS, p->str);
                delete_item(ctr_umap_s, UMAPS, p->str);
                delete_item(ctr_umap_i, UMAPI, p->str);
            }
        }
        else if(strcmp(p->str, "get") == 0)
        {
            /*根据key获取value: get name key*/
            p = p->next;
            if(p == NULL | strlen(p->str) < 1)
            {
                show_help_info(sockfd);
                clear_all(cmd_list);
                free(cmd_list);
                return false;
            }
            else if(is_item_exist(ctr_map_s, p->str) == true || is_item_exist(ctr_umap_s, p->str) == true || is_item_exist(ctr_umap_i, p->str) == true)
            {
                /*后期可以建立<name, type>映射关系 简化代码*/
                if(p->next == NULL) return false;
                it = get_item_by_name(ctr_map_s->item_list->next, p->str);
                if(it != NULL)
                {
                    void* m = it->_item;
                    /*这里其实可能有多值，但是这里暂时只是写入第一个对应值*/
                    vs = getv_map_s((struct map_s*)m, p->next->str);
                    if(vs->next != NULL)
                    {
                        msg_to_sock(sockfd, vs->next->str);
                    }
                    clear_all(vs);
                    free(vs);
                    clear_all(cmd_list);
                    free(cmd_list);
                    return true;
                }
                it = get_item_by_name(ctr_umap_s->item_list->next, p->str);
                if(it != NULL)
                {
                    cbuf = getv_umap_s((struct map_s*)it->_item, p->next->str);
                    msg_to_sock(sockfd, cbuf);
                    free(cbuf);
                    clear_all(cmd_list);
                    free(cmd_list);
                    return true;
                }
                it = get_item_by_name(ctr_umap_i->item_list->next, p->str);
                if(it != NULL)
                {
                    struct int_check r = getv_umap_i((struct umap_i*)it->_item, p->next->str);
                    if(r.is_null != 1)
                    {
                        sprintf(buf, "%d", r.v);
                        msg_to_sock(sockfd, buf);
                    }

                    clear_all(cmd_list);
                    free(cmd_list);
                    return true;
                }
            }
            else
            {
                msg_to_sock(sockfd, "item not found!\n");
                show_help_info(sockfd);
                clear_all(cmd_list);
                free(cmd_list);
                return false;
            }
        }
        else if(strcmp(p->str, "insert") == 0)
        {
            /*在容器库中已有的map中插入键值对: insert mapname angela baby*/
            p = p->next;
            if(p == NULL | strlen(p->str) < 1)
            {
                show_help_info(sockfd);
                clear_all(cmd_list);
                free(cmd_list);
                return false;
            }
            else if(is_item_exist(ctr_map_s, p->str) == true || is_item_exist(ctr_umap_s, p->str) == true || is_item_exist(ctr_umap_i, p->str) == true)
            {
                char* map_name = p->str;
                char* k, *v;
                p = p->next;
                if(p == NULL | strlen(p->str) < 1)
                {
                    show_help_info(sockfd);
                    clear_all(cmd_list);
                    free(cmd_list);
                    return false;
                }
                else
                {
                    k = p->str;
                    p = p->next;
                    if(p == NULL | strlen(p->str) < 1)
                    {
                        show_help_info(sockfd);
                        clear_all(cmd_list);
                        free(cmd_list);
                        return false;
                    }
                    else
                    {
                        v = p->str;
                        it = get_item_by_name(ctr_map_s->item_list, map_name);
                        if(it != NULL)
                        {
                            void* m = it->_item;
                            insert_map_s((struct map_s*)m, k, v);
                            clear_all(cmd_list);
                            free(cmd_list);
                            return true;
                        }
                        it = get_item_by_name(ctr_umap_s->item_list, map_name);
                        if(it != NULL)
                        {
                            void* m = it->_item;
                            insert_umap_s((struct map_s*)m, k, v);
                            clear_all(cmd_list);
                            free(cmd_list);
                            return true;
                        }
                        it = get_item_by_name(ctr_umap_i->item_list, map_name);
                        if(it != NULL)
                        {
                            void* m = it->_item;
                            insert_umap_i((struct umap_i*)m, k, atoi(v));
                            clear_all(cmd_list);
                            free(cmd_list);
                            return true;
                        }
                    }
                }
            }
            else
            {
                msg_to_sock(sockfd, "item not found!");
                show_help_info(sockfd);
                clear_all(cmd_list);
                free(cmd_list);
                return false;
            }
        }
        else if(strcmp(p->str, "remove") == 0)
        {
            /*根据key删除一个映射entry: remove mapname key*/
            p = p->next;
            if(p == NULL | strlen(p->str) < 1)
            {
                    show_help_info(sockfd);
                    clear_all(cmd_list);
                    free(cmd_list);
                    return false;
            }
            if(is_item_exist(ctr_map_s, p->str) == true || is_item_exist(ctr_umap_s, p->str) == true  || is_item_exist(ctr_umap_i, p->str) == true)
            {
                it = get_item_by_name(ctr_map_s->item_list, p->str);
                if(it != NULL)
                {
                    void* m = it->_item;
                    p = p->next;
                    if(p == NULL) return false;
                    delete_map_s((struct map_s*)m, p->str);
                    return true;
                }
                it = get_item_by_name(ctr_umap_s->item_list, p->str);
                if(it != NULL)
                {
                    void* m = it->_item;
                    p = p->next;
                    if(p == NULL) return false;
                    delete_map_s((struct map_s*)m, p->str);
                    return true;
                }
                it = get_item_by_name(ctr_umap_i->item_list, p->str);
                if(it != NULL)
                {
                    void* m = it->_item;
                    p = p->next;
                    if(p == NULL) return false;
                    delete_umap_i((struct umap_i*)m, p->str);
                    return true;
                }
                clear_all(cmd_list);
                free(cmd_list);
                return false;
            }
        }
        else if(strcmp(p->str, "exit") == 0)
        {
            close(sockfd);
        }
        else
        {
            show_help_info(sockfd);
            return false;
        }
    }
    else
    {
        show_help_info(sockfd);
        return false;
    }
}
#endif // 0
bool process_cmd(char* cmdlist, int sockfd)
{
    struct strlist* cmd_list;
    void* map;
    cmd_list = separate_strs(cmdlist, ' ');
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
    process_cmd("new maps m1", 1);
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
