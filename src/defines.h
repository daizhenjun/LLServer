/* 函数结果状态代码 */
#define TRUE 1
#define FALSE 0
#define OK 1
#define ERROR 0
#define INFEASIBLE -1 //失效
#define ALLOC_MEMORY_FAIL -2 //分配内存失败

typedef int status; /* status是函数的类型,其值是函数结果状态代码，如OK等 */

typedef int boolean; /* boolean是布尔类型,其值是TRUE或FALSE */
