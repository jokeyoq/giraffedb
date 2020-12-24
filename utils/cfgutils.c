#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include "../dast/strlist.h"
#include "cfgutils.h"
#define DONOTHING ;

bool is_cfg_existed(char* cfgname)
{
    struct stat st;
    char* fullpath;
    int ret;
    fullpath = combine_path(cfgname);
    printf("CONFIG FILES PATH: %s\n", fullpath);
    ret = stat(fullpath, &st);
    free(fullpath);
    return ret == 0 ? true : false;
}
bool remove_cfg(char* cfgname)
{
    int ret;
    char* fullpath;
    fullpath = combine_path(cfgname);
    ret = remove(fullpath);
    free(fullpath);
    return ret == -1 ? false : true;
}
bool create_cfg(char* cfgname)
{
    int fd;
    char* fullpath;
    fullpath = combine_path(cfgname);
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IXUSR | S_IXGRP;
    fd = creat(fullpath, mode);
    free(fullpath);
    if(fd == -1) return false;
    else close(fd);
    return true;
}
char* combine_path(char* cfgname)
{
    int i, j;
    int fplen = strlen(CONFIGS_PATH)+strlen(cfgname);
    char* fullpath = (char*)malloc(fplen+1);
    for(i = 0; i < strlen(CONFIGS_PATH); i++)
    {
        fullpath[i] = CONFIGS_PATH[i];
    }
    for(j = 0; j < strlen(cfgname); j++)
    {
        fullpath[i++] = cfgname[j];
    }
    fullpath[i] = '\0';
    return fullpath;
}
void read_til_nextline(FILE* fp)
{
    int c;
    while((c = fgetc(fp) != '\n') && c != EOF);
    return;
}
struct strlist* read_strs_til_nextline(FILE* fp)
{
    /*读取一行所有字符串装载进strlist直到换行*/
    int c, i;
    char str[BUFSIZ];
    struct strlist* list = create_str_list();
    i = 0;
    while((c = fgetc(fp)) != '\n' && c != EOF)
    {
        if(c != '\t' && c != ' ')
        {
            str[i++] = c;
        }
        else
        {
            str[i] = '\0';
            i = 0;
            insert_back(list, str);/*这里不能用insert_front*/
        }
    }
    str[i] = '\0';
    insert_back(list, str);/*这里不能用insert_front*/
    return list;
}
struct strlist* get_cfg_values(char* cfgname, char* key)
{
    FILE* fp;
    struct strlist* list;
    char str[BUFSIZ];
    int c, i;
    char* fullpath;
    i = 0;
    fullpath = combine_path(cfgname);
    fp = fopen(fullpath, "r");
    free(fullpath);
    while((c = fgetc(fp)) != EOF)
    {
        if(c != '\t' && c != ' ')
        {
            str[i++] = c;
        }
        else
        {
            str[i] = '\0';
            i = 0;
            if(strcmp(str, key) == 0)
            {
                printf("Get the key:\"%s\"\n", str);
                list = read_strs_til_nextline(fp);
                fclose(fp);
                return list;
            }
            else
            {
                read_til_nextline(fp);
            }
        }
    }
    fclose(fp);
    return create_str_list();/*修复了此处BUG 先前是return NULL*/
}
char* get_cfg_value(char* cfgname, char* key, int n)
{
    /*此处是否有bug？key只能位于第一列*/
    struct strlist* list, *p;
    char* rtc;
    int i;
    list = get_cfg_values(cfgname, key);
    rtc = NULL;
    if(list->next == NULL) return NULL;
    i = 1;
    p = list;
    while((p = get_next_item(p)) != NULL)
    {
        if(i++ == n)
        {
            rtc = (char*)malloc(strlen(p->str)+1);
            strcpy(rtc, p->str);
            clear_all(list);
            break;
        }
        else if( i > n) break;
    }
    return rtc;/*没有找到*/
}

bool insert_cfg_record(char* cfgname, char* key, struct strlist* vlist)
{
    FILE *fp;
    struct strlist* p;
    char* fullpath;
    fullpath = combine_path(cfgname);
    fp = fopen(fullpath, "a+");
    fputs(key, fp);
    fputc('\t', fp);
    p = vlist;
    while((p = get_next_item(p)) != NULL)
    {
        fputs(p->str, fp);
        if(p->next == NULL)
        {
            fputc('\n', fp);
        }
        else
        {
            fputc('\t', fp);
        }
    }
    free(fullpath);
    fclose(fp);
    return true;
}
bool update_cfg_value(char* cfgname, char* key, char* nvalue, int n)
{
    FILE* fp, *nfp;
    char str[BUFSIZ];
    int i, c, cv, flen, kflag, vflag;/*cv用于记录当前读到第几个value*/
    int is_checking_key;
    char* fullpath, *newpath;
    fullpath = combine_path(cfgname);
    flen = strlen(fullpath);
    newpath = (char*)malloc(flen+5);
    strncpy(newpath, fullpath, flen);
    newpath[flen] = '.';
    newpath[flen+1] = 't';
    newpath[flen+2] = 'm';
    newpath[flen+3] = 'p';
    newpath[flen+4] = '\0';
    fp = fopen(fullpath, "r");
    nfp = fopen(newpath, "w");
    kflag = 0;
    vflag = 0;
    is_checking_key = 1;
    i = 0;
    cv = 0;
    while((c = fgetc(fp)) != EOF)
    {
        if(c == '\t' || c == '\n' || c ==  ' ')
        {
            str[i] = '\0';
            if(is_checking_key == 1 && strcmp(key, str) == 0)
            {
                kflag = 1;
                fputs(str, nfp);
                fputc(c, nfp);
                break;
            }
            else
            {
                fputs(str, nfp);
                fputc(c, nfp);
            }
            i = 0;
            if(c == '\n')
                is_checking_key = 1;
            else
                is_checking_key = 0;
        }
        else
        {
            str[i++] = c;
        }
    }
    i = 0;
    /*已经找到了key，开始找第n个value然后修改*/
    while((c = fgetc(fp)) != EOF && cv < n)
    {
        if(c == '\t' || c == ' ' || c == '\n')
        {
            str[i] = '\0';
            cv++;
            if(cv == n)
            {
                vflag = 1;
                fputs(nvalue, nfp);
                fputc(c, nfp);
                break;
            }
            else
            {
                fputs(str, nfp);
                fputc(c, nfp);
            }
            i = 0;
        }
        str[i++] = c;
    }
    /*填充之后的内容*/
    while((c = fgetc(fp)) != EOF)
        fputc(c, nfp);
    fclose(fp);
    fclose(nfp);
    remove(fullpath);
    rename(newpath, fullpath);
    free(fullpath);
    free(newpath);
    if(kflag == 1 && vflag == 1) return true;
    else return false;
}
bool remove_cfg_record(char* cfgname, char* key)
{
    /*
    这里fgets(直接读完一整行了 有错误！！！！ 使用fgetc替代修改
    */
    int c, kflag, i, j;
    int flen;
    int is_checking_key;/*用来判断找key的while循环状态，=1时正常进行判断， =0时取消跳过判断 因为后面是value,而要比较的是key《*/
    FILE* fp, *new_fp;
    char* fullpath = combine_path(cfgname);/*别忘了combine*/
    flen = strlen(fullpath);
    char* new_path = (char*)malloc(flen+5);/* fullpath.tmp */
    strncpy(new_path, fullpath, flen);
    is_checking_key = 1;
    new_path[flen] = '.';
    new_path[flen+1] = 't';
    new_path[flen+2] = 'm';
    new_path[flen+3] = 'p';
    new_path[flen+4] = '\0';
    char str[BUFSIZ];
    new_fp = fdopen(creat(new_path, 0644),"w");/*一个新文件 xxx.tmp用于写入删除or更改后的配置文件，之后删除旧文件，改名新文件*/
    i = 0;
    kflag = 0;
    fp = fopen(fullpath, "r+");
    while((c = fgetc(fp)) != EOF)
    {
        /*找到Key所在行，注意这里key必须位于第一列*/
        if(c == '\t' || c == ' ' || c == '\n')
        {
            str[i] = '\0';
            if(is_checking_key == 1 && strcmp(str, key) == 0)
            {
                printf("Key: [%s] len:[%d]\n", str, i);
                kflag = 1;
                if(c != EOF && c != '\n')
                    read_til_nextline(fp);
                break;
            }
            else
            {
                for(j = 0; j < i; j++)
                {
                    fputc(str[j], new_fp);
                }
                fputc(c, new_fp);
                is_checking_key = 0;
                if(c == '\n')
                {
                    is_checking_key = 1;/*新的一行第一列 自然是key 置标志位为1*/
                }
                i = 0;
                continue;
            }
        }
        else
        {
            str[i++] = c;
        }
    }
    while(c != EOF && (c = fgetc(fp)) != EOF)
    {
        fputc(c, new_fp);/*输入除去key行余下的内容*/
    }
    fclose(new_fp);
    fclose(fp);
    remove(fullpath);
    rename(new_path, fullpath);
    free(fullpath);
    free(new_path);
    return kflag == 1 ? true : false;
}
void test_cfg_utils()
{
}
