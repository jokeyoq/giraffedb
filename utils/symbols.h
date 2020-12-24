#define bool int
#define true 1
#define false 0
#define STATUS int
#define OK 1
#define ERROR 0
#define CHAR char
typedef enum
{
    TMAPS = 1,
    TUMAPS,
    TUMAPI
}MAP_TYPE;
/*这里如果加了条件包含 会在第二个引用的文件出现问题？*/
