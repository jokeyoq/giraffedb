#ifndef _CFGUTILS_H_
#define _CFGUTILS_H_
#define bool int
#define true 1
#define false 0
#define CONFIGS_PATH "configs/"
bool is_cfg_existed(char* cfgname);

bool create_cfg(char* cfgname);

bool remove_cfg(char* cfgname);

char* combine_path(char* cfgname);

struct strlist* get_cfg_values(char* cfgname, char* key);/*内部函数，获取key对应的strlist 用于get_cfg_value*/
void read_til_nextline(FILE* fp);

struct strlist* read_strs_til_nextline(FILE* fp);

char* get_cfg_value(char* cfgname, char* key, int n);/*key value1 value2 ...一个key对应不止一个value 获取第n个value*/

bool update_cfg_value(char* cfgname, char* key, char* nvalue, int n);

bool remove_cfg_record(char* cfgname, char* key);/*移除整条记录*/

bool insert_cfg_record(char* cfgname, char* key, struct strlist* vlist);

void test_cfg_utils(void);


#endif
