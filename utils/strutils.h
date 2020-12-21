#ifndef _STRUTILS_H_
#define _STRUTILS_H_
/*
定义了几个字符串处理函数
strlist* separate_strs(char* arglsit, char separator); 返回把以separator间隔的字符串组分割后的@字符串链表
如"map -s newmap" ->
[map]->[-s]->[newmap]
*/
struct strlist* separate_strs(char* arglist, char separator);

void str_clear(char* str, int n);

void test_strutils(void);
/*测试函数*/
#endif
