#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include "../dast/map.h"
#include "../utils/symbols.h"
#include "container.h"
#define ONE_MIN 60
struct container* create_container()
{
    struct container* ctr;
    struct item* head;
    ctr = (struct container*)malloc(sizeof(struct container));
    head = (struct item*)malloc(sizeof(struct item));
    head->iname = (char*)malloc(5);
    strcpy(head->iname, "head");
    head->_item = NULL;
    head->next = NULL;
    ctr->free_scan_interval = 10*ONE_MIN;
    ctr->item_list = head;
    ctr->item_num = 0;
    return ctr;
}
STATUS set_free_scan_interval(struct container* ctr, int ival)
{
    if(ctr == NULL) return ERROR;
    ctr->free_scan_interval = ival;
    return OK;
}
STATUS add_item(struct container* ctr, void* item, char* iname)
{
    struct item* it;
    it = (struct item*)malloc(sizeof(struct item));
    it->_item = item;
    it->iname = (char*)malloc(strlen(iname)+1);
    strcpy(it->iname, iname);
    it->next = ctr->item_list->next;
    ctr->item_list->next = it;
    ctr->item_num++;
    return OK;
}
STATUS free_all_items(struct container* ctr, CTR_TYPE ctr_type)
{
    struct item*p, *q;
    p = ctr->item_list->next;
    while(p != NULL)
    {
        q = p->next;
        if(ctr_type == MAPS)
        {
            printf("deleting...[%s]\n", p->iname);
            clear_map_s((struct map_s*)p->_item);
            free(p->_item);
            free(p->iname);
        }
        else if(ctr_type == UMAPS)
        {
            clear_umap_s((struct map_s*)p->_item);
            free(p->_item);
            free(p->iname);
        }
        else if(ctr_type == UMAPI)
        {
            clear_umap_i((struct umap_i*)p->_item);
            free(p->_item);
            free(p->iname);
        }
        free(p);
        p = q;
        ctr->item_num--;
    }
    ctr->item_list->next = p;
    return OK;
}
STATUS delete_item(struct container* ctr, CTR_TYPE ctr_type, char* iname)
{
    struct item*p, *prev;
    p = ctr->item_list->next;
    prev = ctr->item_list;
    while(p != NULL)
    {
        if(strcmp(iname, p->iname) == 0)
        {
            prev->next = p->next;
            if(ctr_type == MAPS)
            {
                clear_map_s((struct map_s*)p->_item);
                free(p->iname);
            }
            else if(ctr_type == UMAPS)
            {
                clear_umap_s((struct map_s*)p->_item);
                free(p->iname);
            }
            else if(ctr_type == UMAPI)
            {
                clear_umap_i((struct umap_i*)p->_item);
                free(p->iname);
            }
            free(p);
            p = NULL;
            break;
        }
        prev = p;
        p = p->next;
    }
    ctr->item_num--;
    return OK;
}
void print_all_items_in_ctr(struct container* ctr)
{
    struct item* it;
    it = ctr->item_list->next;
    while(it != NULL)
    {
        printf("[%s]\n", it->iname);
        it = it->next;
    }
}
bool is_item_exist(struct container* ctr, char* iname)
{
    if(ctr->item_num == 0) return false;
    struct item* i;
    i = ctr->item_list->next;
    while(i != NULL)
    {
        if(strcmp(i->iname, iname) == 0) return true;
        i = i->next;
    }
    return false;
}
void test_container()
{

}
