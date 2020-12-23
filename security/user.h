#ifndef _USER_H_
#define _USER_H_
#define USER_CFG_NAME ".user"
/*ON, OF, PH三种帐号状态：ON对应已经登陆 OF对应离线 PH对应帐号被禁止*/
bool init_user_cfg(char* cfgname);/*初始化用户配置文件*/
bool create_user(char* cfgname, char* username, char* passwd, char* status);/*这里的passwd经过加密存储*/
bool remove_user(char* cfgname, char* username);/*删除用户*/
int login_checker(char* cfgname, char* username, char* passwd);/*核对用户名及密码，修改 return 1 表示密码错误 -1表示帐号不存在 0 成功*/
void test_user(void);
#endif
