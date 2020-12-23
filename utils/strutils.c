#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "strutils.h"
#include "../dast/strlist.h"
static char* trim_str(char* str, char todel);
char* trim_str(char* str, char todel)
{
    int str_len, left, right, newlen;
    char* new_str;
    str_len = strlen(str);
    left = 0;
    right = str_len-1;
    while(str[left] == todel) left++;
    while(str[right] == todel) right--;
    newlen = right - left + 1;
    new_str = (char*)malloc(newlen+1);
    strncpy(new_str, str+left, newlen);
    new_str[newlen] = '\0';
    return new_str;
}
struct strlist* separate_strs(char* arglist, char separator)
{
    /*重构*/
    struct strlist* list;
    char str[BUFSIZ];
    int i, j, c;
    list = create_str_list();
    arglist = trim_str(arglist, separator);
    i = 0;  j = 0;
    while((c = arglist[i++]) != '\0')
    {
        if(c == separator)
        {
            str[j] = '\0';
            insert_back(list, str);
            j = 0;
            continue;
        }
        str[j++] = c;
    }
    if(j != 0)
    {
        str[j] = '\0';
        insert_back(list, str);
    }
    return list;
}
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
}
