#ifndef _STRLIST_H_
#define _STRLIST_H_
#define OK 1
#define ERROR 0
#define STATUS int
/*
定义了带头节点的单向字符串链表结构
以及一些操作函数
*/
struct strlist
{
    char* str;
    struct strlist* next;
};
struct strlist* create_str_list(void);
/*返回一个带头节点的空链表，创建失败则返回NULL*/

STATUS insert_back(struct strlist* head, char* str);
/*在尾部插入一个字符串，成功返回1, 失败返回0*/

STATUS insert_front(struct strlist* head, char* str);
/*在头部插入一个字符串，成功返回1, 失败返回0*/

STATUS clear_all(struct strlist* head);
/*释放除了头节点外的所有节点，成功返回1, 失败返回0*/
/*这里其实都没有考虑char*动态分配释放的问题，以后再解决*/
STATUS delete_str(struct strlist* head, char* str);
/*删除给定str，成功返回1, 失败返回0*/

struct strlist* get_next_item(struct strlist* head);
/*遍历操作函数
    while((p = get_next_item(p)) != NULL)
    {
        printf("%s\n", p->str);
    }
    记得新建变量保存
*/

void print_list(struct strlist* head);
/*打印整个字符串链表*/
void test_strlist(void);
/*测试函数*/
#endif
