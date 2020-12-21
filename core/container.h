#ifndef _CONTAINER_H_
#define _CONTAINER_H_
#define RES_MAX 1024
#define CTR_TYPE int
#define UMAPS 0
#define MAPS 1
#define UMAPI 2
/*管理资源的创建&释放*/
struct item
{
    void* _item;
    char* iname;
    struct item* next;
};
struct container
{
    /*容器：存放items，记录存放的item的数量，根据free_scan_interval定地扫描item的res_type
    = LONGLIVE 则扫描到2次释放
    = FOREVER 则永不释放，直到程序关闭
    = NORMAL 则扫描到1次释放
    或被手动释放
    */
    struct item* item_list;
    int item_num;
    int free_scan_interval;

};

struct container* create_container(void);

STATUS set_free_scan_interval(struct container* ctr, int ival);

STATUS add_item(struct container* ctr, void* item, char* iname);

STATUS free_all_items(struct container* ctr, CTR_TYPE ctr_type);

STATUS delete_item(struct container* ctr, CTR_TYPE ctr_type, char* iname);

bool is_item_exist(struct container* ctr, char* iname);

void print_all_items_in_ctr(struct container* ctr);

struct item* get_item_by_name(struct item* it, char* itname);

void free_handler(int);

void test_container(void);
#endif // _CONTAINER_H_
