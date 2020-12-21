#ifndef _MAP_H_
#define _MAP_H_
#define STATUS int
#define OK 1
#define ERROR 0
#define SIZHASHTAB 512
#define bool int
#define true 1
#define false 0
/*
定义了Unique Map & General Map数据类型 支持 字符串到字符串&字符串到整形数字的映射
*/
struct entry_s
{
    char* key;
    char* value;
    struct entry_s* next;/*单链表解决哈希冲突*/
};
struct entry_i
{
    char* key;
    int value;
    struct entry_i* next;
};
struct map_s
{
    struct entry_s* entries[SIZHASHTAB];/*指针数组*/
    int map_size;
};
struct umap_i
{
    struct entry_i* entries[SIZHASHTAB];
    int map_size;
};
struct int_check
{
    /*可以判断是否为空的封装int*/
    int v;
    bool is_null;
};

struct map_s* create_map_s(void);
/*创建空允许重复的map*/
STATUS insert_map_s(struct map_s* m, char* k, char* v);
/*插入<key,value>到的Map*/
STATUS delete_map_s(struct map_s* m, char* k);
/*根据k删除对应所有entry， 对于非unique的map，返回值为删除个entry个数*/
struct strlist* getv_map_s(struct map_s* m, char* k);
/*根据k查到v*/
STATUS clear_map_s(struct map_s* m);
/*清空map 返回删除的entry个数*/
void print_map_s(struct map_s* m);
/*打印map_s*/
void test_map_s(void);
/*测试函数*/


/*----------unique map----------------*/
/*----------umap_s--------------------*/
struct map_s* create_umap_s(void);

STATUS insert_umap_s(struct map_s* m, char* k, char* v);

STATUS delete_umap_s(struct map_s* m, char* k);

char* getv_umap_s(struct map_s* m, char* k);

STATUS clear_umap_s(struct map_s* m);

void test_umap_s(void);
/*----------umap_i--------------------*/
struct umap_i* create_umap_i(void);

STATUS insert_umap_i(struct umap_i* m, char* k, int v);

STATUS delete_umap_i(struct umap_i* m, char* k);

struct int_check getv_umap_i(struct umap_i* m, char* k);

STATUS clear_umap_i(struct umap_i* m);

void print_umap_i(struct umap_i* m);

void test_umap_i(void);
#endif
