#define __LIBRARY__
#include <unistd.h>
_syscall2(int,sem_open,const char *,name,unsigned int,value)
_syscall1(int,sem_wait,int,sem)
_syscall1(int,sem_post,int,sem)
_syscall1(int,sem_unlink,const char *,name)
_syscall1(int,sem_getvalue,sem_t *,sem); 
