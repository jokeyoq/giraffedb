#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "map.h"
#include "strlist.h"
#define INTMAX 65535
#define SEP printf("---------------\n") /*打印一行分隔符*/
static int hash_str(char* str);
/*底层哈希函数 累加str个字符的ascii值得到*/
int hash_str(char* str)
{
    int sum;
    sum = 0;
    while((*str) != '\0')
    {
        sum += (int)(*str);
        str++;
    }
    return sum % SIZHASHTAB;/*0 ~ SIZHASHTAB-1*/
}
/*map_s(not unique)*/
struct map_s* create_map_s(void)
{
    struct map_s* m;
    struct entry_s* e;
    int i;
    m = (struct map_s*)malloc(sizeof(struct map_s));
    if(m == NULL) return NULL;
    for(i = 0; i < SIZHASHTAB; i++)
    {
        e = (struct entry_s*)malloc(sizeof(struct entry_s));
        e->key = "HEADK";
        e->value = "HEADV";
        e->next = NULL;
        m->entries[i] = e;
        m->entries[i]->next = NULL;
    }
    m->map_size = 0;
    return m;
};
STATUS insert_map_s(struct map_s* m, char* k, char* v)
{
    int pos;
    struct entry_s* entry;
    entry = (struct entry_s*)malloc(sizeof(struct entry_s));
    pos = hash_str(k);
    entry->key = (char*)malloc(strlen(k)+1);
    strcpy(entry->key, k);
    entry->value = (char*)malloc(strlen(v)+1);
    strcpy(entry->value, v);
    /*这里用头插法*/
    entry->next = m->entries[pos]->next;
    m->entries[pos]->next = entry;
    m->map_size++;
    return OK;
}

STATUS delete_map_s(struct map_s* m, char* k)
{
    int pos;
    int delcnt;
    struct entry_s* p, *q, *prev;
    delcnt = 0;
    pos = hash_str(k);
    p = m->entries[pos]->next;
    prev = m->entries[pos];
    while(p != NULL)
    {
        if(strcmp(k, p->key)==0)
        {
            delcnt++;
            m->map_size--;
            q = p->next;
            free(p->key);
            free(p->value);
            free(p);
            prev->next = q;
        }
        prev = p;
        p = p->next;
    }
    return delcnt;
}

struct strlist* getv_map_s(struct map_s* m, char* k)
{
    int pos;
    struct entry_s* p;
    struct strlist* list;
    pos = hash_str(k);
    list = create_str_list();
    p = m->entries[pos]->next;
    while(p != NULL)
    {
        if(strcmp(k, p->key)==0)
        {
            insert_back(list, p->value);
        }
        p = p->next;
    }
    return list;
}
STATUS clear_map_s(struct map_s* m)
{
    int i;
    int c;
    struct entry_s* p, *q;
    c = 0;
    for(i = 0; i < SIZHASHTAB; i++)
    {
        p = m->entries[i]->next;
        while(p != NULL)
        {
            q = p->next;
            free(p->key);
            free(p->value);
            free(p);
            p = q;
            c++;
        }
        m->entries[i]->next = NULL;
    }
    m->map_size = 0;
    return c;
}
void print_map_s(struct map_s* m)
{

    int i;
    struct entry_s* p;
    for(i = 0; i < SIZHASHTAB; i++)
    {
        p = m->entries[i]->next;
        while(p != NULL)
        {
            printf("<%s,%s>\n", p->key, p->value);
            p = p->next;
        }
    }
}
/*unque map_s here:*/
struct map_s* create_umap_s(void)
{
    struct map_s* m;
    struct entry_s* e;
    int i;
    m = (struct map_s*)malloc(sizeof(struct map_s));
    if(m == NULL) return NULL;
    for(i = 0; i < SIZHASHTAB; i++)
    {
        e = (struct entry_s*)malloc(sizeof(struct entry_s));
        e->key = "HEADK";
        e->value = "HEADV";
        e->next = NULL;
        m->entries[i] = e;
        m->entries[i]->next = NULL;
    }
    m->map_size = 0;
    return m;
};
STATUS insert_umap_s(struct map_s* m, char* k, char* v)
{
    int pos;
    struct entry_s* entry;
    if(getv_umap_s(m, k) != NULL) return ERROR;
    entry = (struct entry_s*)malloc(sizeof(struct entry_s));
    pos = hash_str(k);
    entry->key = (char*)malloc(strlen(k)+1);
    strcpy(entry->key, k);
    entry->value = (char*)malloc(strlen(v)+1);
    strcpy(entry->value, v);
    /*这里用头插法*/
    entry->next = m->entries[pos]->next;
    m->entries[pos]->next = entry;
    m->map_size++;
    return OK;
}

STATUS delete_umap_s(struct map_s* m, char* k)
{
    int pos;
    int delcnt;
    struct entry_s* p, *q, *prev;
    delcnt = 0;
    pos = hash_str(k);
    p = m->entries[pos]->next;
    prev = m->entries[pos];
    while(p != NULL)
    {
        if(strcmp(k, p->key)==0)
        {
            delcnt++;
            m->map_size--;
            q = p->next;
            free(p->key);
            free(p->value);
            free(p);
            prev->next = q;
            return OK;
        }
        prev = p;
        p = p->next;
    }
    return ERROR;
}

char* getv_umap_s(struct map_s* m, char* k)
{
    int pos;
    struct entry_s* p;
    char* ret;
    pos = hash_str(k);
    p = m->entries[pos]->next;
    ret = NULL;
    while(p != NULL)
    {
        if(strcmp(k, p->key)==0)
        {
            ret = p->value;
            return ret;
        }
        p = p->next;
    }
    return ret;
}
STATUS clear_umap_s(struct map_s* m)
{
    int i;
    int c;
    struct entry_s* p, *q;
    c = 0;
    for(i = 0; i < SIZHASHTAB; i++)
    {
        p = m->entries[i]->next;
        while(p != NULL)
        {
            q = p->next;
            free(p->key);
            free(p->value);
            free(p);
            p = q;
            c++;
        }
        m->entries[i]->next = NULL;
    }
    m->map_size = 0;
    return c;
}
/*umap_i*/
struct umap_i* create_umap_i(void)
{
    struct umap_i* m;
    struct entry_i* e;
    int i;
    m = (struct umap_i*)malloc(sizeof(struct umap_i));
    if(m == NULL) return NULL;
    for(i = 0; i < SIZHASHTAB; i++)
    {
        e = (struct entry_i*)malloc(sizeof(struct entry_i));
        e->key = "HEADK";
        e->value = INTMAX;
        e->next = NULL;
        m->entries[i] = e;
        m->entries[i]->next = NULL;
    }
    m->map_size = 0;
    return m;
};
STATUS insert_umap_i(struct umap_i* m, char* k, int v)
{
    int pos;
    struct entry_i* entry;
    struct int_check ic;
    ic = getv_umap_i(m, k);
    if(ic.is_null == false) return ERROR;
    entry = (struct entry_i*)malloc(sizeof(struct entry_i));
    pos = hash_str(k);
    entry->key = (char*)malloc(strlen(k)+1);
    strcpy(entry->key, k);
    entry->value = v;
    /*这里用头插法*/
    entry->next = m->entries[pos]->next;
    m->entries[pos]->next = entry;
    m->map_size++;
    return OK;
}

STATUS delete_umap_i(struct umap_i* m, char* k)
{
    int pos;
    struct entry_i* p, *q, *prev;
    pos = hash_str(k);
    p = m->entries[pos]->next;
    prev = m->entries[pos];
    while(p != NULL)
    {
        if(strcmp(k, p->key)==0)
        {
            m->map_size--;
            q = p->next;
            free(p->key);
            free(p);
            p = NULL;
            prev->next = q;
            break;
        }
        prev = p;
        p = p->next;
    }

    return true;
}

struct int_check getv_umap_i(struct umap_i* m, char* k)
{
    int pos;
    struct entry_i* p;
    struct int_check ic;
    ic.is_null = true;
    pos = hash_str(k);
    p = m->entries[pos]->next;
    while(p != NULL)
    {
        if(strcmp(k, p->key)==0)
        {
            ic.is_null = false;
            ic.v = p->value;
        }
        p = p->next;
    }
    return ic;
}
STATUS clear_umap_i(struct umap_i* m)
{
    int i;
    int c;
    struct entry_i* p, *q;
    c = 0;
    for(i = 0; i < SIZHASHTAB; i++)
    {
        p = m->entries[i]->next;
        while(p != NULL)
        {
            q = p->next;
            free(p->key);
            free(p);
            p = q;
            c++;
        }
        m->entries[i]->next = NULL;
    }
    m->map_size = 0;
    return c;
}
void print_umap_i(struct umap_i* m)
{

    int i;
    struct entry_i* p;
    for(i = 0; i < SIZHASHTAB; i++)
    {
        if(m->entries[i] == NULL) continue;
        p = m->entries[i]->next;
        while(p != NULL)
        {
            printf("<%s,%d>\n", p->key, p->value);
            p = p->next;
        }
    }
}
void test_map_s()
{

}
void test_umap_s()
{
}
void test_umap_i()
{

}
