#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "strlist.h"
struct strlist* create_str_list(void)
{
    struct strlist* head;
    head = (struct strlist*)malloc(sizeof(struct strlist));
    if(head == NULL) return NULL;
    head->next = NULL;
    head->str = "HEAD OF STR LIST";
    return head;
}
STATUS insert_back(struct strlist* head, char* str)
{
    struct strlist* p;
    p = (struct strlist*)malloc(sizeof(struct strlist));
    if(p == NULL) return ERROR;
    p->next = NULL;
    p->str = (char*)malloc(strlen(str)+1);
    strcpy(p->str, str);/*这里修复了一个Bug 这里直接p->str = str, 会有错误 本地内存释放就会有问题*/
    while(head->next!=NULL)
    {
        /*定位到尾节点*/
        head = head->next;
    }
    head->next = p;
    return OK;
}
STATUS insert_front(struct strlist* head, char* str)
{
    struct strlist* p;
    p = (struct strlist*)malloc(sizeof(struct strlist));
    if(p == NULL) return ERROR;
    p->next = head->next;
    p->str = (char*)malloc(strlen(str)+1);
    strcpy(p->str, str);/*这里修复了一个Bug 这里直接p->str = str, 会有错误 本地内存释放就会有问题*/
    /*本来在栈区的字符串被释放，list第二个字符串可能就放在了第一个指针指向的地址，导致明明不同的插入字符串，打印时却全是最后一个插入的字符串*/
    head->next = p;
    return OK;
}
STATUS clear_all(struct strlist* head)
{
    struct strlist* p;
    head = head->next;
    while(head != NULL)
    {
        p = head->next;
        //printf("[%s] deleted.\n", head->str);
        free(head);
        head = p;
    }
    return OK;
}
STATUS delete_str(struct strlist* head, char* str)
{
    struct strlist* p, *prev;
    int flag;/*匹配之后置为1,没有匹配字符串则返回0*/
    flag = 0;
    prev = head;/*置前驱指针*/
    head = head->next;
    while(head != NULL)
    {
        if(strcmp(str, head->str)==0)
        {
            flag = 1;
            p = head->next;
            free(head->str);
            free(head);
            prev->next = p;
            break;
        }
        prev = head;
        head = head->next;
    }
    return flag == 1 ? OK : ERROR;
}
void print_list(struct strlist* head)
{
    struct strlist*p;
    p = head->next;
    printf("Print all the string list:\n");
    while(p)
    {
        printf("[%s]\n", p->str);
        p = p->next;
    }
    return;
}
void test_strlist(void)
{
    struct strlist* list = create_str_list();
    if(list == NULL)
    {
        printf("Error: create_str_list");
        return;
    }
    insert_back(list, "agustin");
    insert_back(list, "pepe");
    insert_back(list, "orange");
    insert_back(list, "nane");
    insert_back(list, "uruguay");
    print_list(list);
    delete_str(list, "nane");
    delete_str(list, "orange");
    printf("after delete nane & orange:\n");
    print_list(list);
    printf("insert into head jack & ana:\n");
    insert_front(list, "jack");
    insert_front(list, "ana");
    print_list(list);
    printf("clear all:\n");
    clear_all(list);
    return;
}
