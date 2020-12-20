#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "strutils.h"
#include "../dast/strlist.h"
struct strlist* separate_strs(char* arglist, char separator)
{
    struct strlist* list;
    char* substr;
    char* str_with_sep, *tofree;
    char* begin;
    int arglen;
    int len;
    arglen = strlen(arglist);
    list = create_str_list();
    str_with_sep = (char*)malloc(sizeof(char)*arglen+2);
    tofree = str_with_sep;
    strncpy(str_with_sep, arglist, arglen);
    if(str_with_sep[arglen-1]!=separator)
    {
        /*在字符串末尾添加多一个分隔符 方便尾部处理*/
        str_with_sep[arglen] = separator;
        str_with_sep[arglen+1] = '\0';
    }
    else
    {
        str_with_sep[arglen] = '\0';
    }
    if(list == NULL) return NULL;
    begin = str_with_sep;
    printf("%s--\n", str_with_sep);
    len = 0;
    while((*str_with_sep)!='\0')
    {
        len++;
        if((*str_with_sep) == separator)
        {
            if((*begin) == separator && len == 1)
            {
                /*考虑到"&ana"这种特殊情况，其实也可以写个trim消除这种情况*/
                begin = ++str_with_sep;
                len = 0;
                continue;
            }
            else
            {
                substr = (char*)malloc(sizeof(char)*len);
                strncpy(substr, begin, len);
                substr[len-1] = '\0';
                insert_back(list, substr);
                len = 0;
                begin = ++str_with_sep;
            }
        }
        else
        {
            str_with_sep++;
        }
    }
    free(tofree);
    return list;
};
struct strlist* get_next_item(struct strlist* head)
{
    return (head = head->next);
}
void str_clear(char* str, int n)
{
    int i;
    for(i = 0; i < n; i++)
    {
        str[i] = ' ';
    }
}
void test_strutils()
{
    struct strlist* list1 = separate_strs("ana&pepe&ceta", '&');
    struct strlist* list2 = separate_strs("ana&pepe&ceta&", '&');
    struct strlist* list3 = separate_strs("&ana&pepe&ceta", '&');
     struct strlist* list4 = separate_strs(" ana pepe ceta", ' ');
    print_list(list1);
    print_list(list2);
    print_list(list3);
    print_list(list4);
}
