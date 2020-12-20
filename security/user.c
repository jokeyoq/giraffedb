#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "user.h"
#include "../utils/symbols.h"
#include "../utils/cfgutils.h"
#include "../dast/strlist.h"
static char* encoding(char* oripsd);/*密码加密算法*/
char* encoding(char* oripsd)
{
    char* npsd;
    int i;
    int c;
    npsd = (char*)malloc(strlen(oripsd)+1);
    for(i = 0; i < strlen(oripsd); i++)
    {
        c = (oripsd[i]+i);
        if(c == '\t' || c == ' ' || c == '\n' || c =='\0' || c =='\r') c = '%';
        npsd[i] = c;
    }
    npsd[i] = '\0';
    return npsd;
}
bool init_user_cfg(char* cfgname)
{
    struct strlist* vs;
    char* pwd;
    pwd = encoding("giraffe");
    vs = create_str_list();
    if(!is_cfg_existed(cfgname))
    {
        create_cfg(cfgname);
        insert_back(vs, pwd);/*passed*/
        insert_back(vs, "OF");/*user status*/
        insert_cfg_record(cfgname, "giraffe", vs);
    }
    clear_all(vs);
    free(vs);
    free(pwd);
    return true;
}
bool create_user(char* cfgname, char* username, char* passwd, char* status)
{
    char* ret;
    char* pwdecd;
    struct strlist* vs;
    ret = get_cfg_value(cfgname, username, 1);
    if(ret != NULL) return false;
    vs = create_str_list();
    pwdecd = encoding(passwd);
    insert_back(vs, pwdecd);
    insert_back(vs, status);

    insert_cfg_record(cfgname, username, vs);
    clear_all(vs);
    free(vs);
    free(pwdecd);
    return true;
}
bool remove_user(char* cfgname, char* username)
{
    /*这里后续需要考虑status 为 ON 的情况*/
    return remove_cfg_record(cfgname, username);
}

int login_checker(char* cfgname, char* username, char* passwd)
{
    char* pwd_encoded;
    char* pwd_cfg;
    pwd_cfg = get_cfg_value(cfgname, username, 1);
    if(pwd_cfg == NULL) return -1;
    pwd_encoded = encoding(passwd);
    if(strcmp(pwd_encoded, pwd_cfg) == 0) return 0;
    else return -2;
}
void test_user()
{
    //init_user_cfg(".usertest");
    //create_user(".usertest", "kimy", "ultra", "OF");
    //remove_user(".usertest", "kimy");
    int b = login_checker(".usertest", "", "sultra");
    if(b == 0)
        printf("Yes!\n");
    else if(b == 1)
        printf("No!\n");
    else if(b == -1)
        printf("User not exist.\n");
}

