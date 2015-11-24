#define __LIBRARY__					/* 有它，_syscall1等才有效。详见unistd.h */
#include <unistd.h>				/* 有它，编译器才能获知自定义的系统调用的编号 */
#include <errno.h>
#include <asm/segment.h>
#include <linux/kernel.h>
#define MAXNUM 32
_syscall1(int, iam, const char*, name);		/* iam()在用户空间的接口函数 */
_syscall2(int, whoami,char*,name,unsigned int,size);	/* whoami()在用户空间的接口函数 */



char usname[MAXNUM];

int sys_iam(const char* name)
{
	int cnt, ans;
	cnt = 0;

	while(get_fs_byte(name + cnt) != '\0' && cnt < MAXNUM)
		cnt++;
	if(cnt > 23)
	{
		return -EINVAL;
	}
	for(ans = 0; ans <= cnt; ans++)
		usname[ans] = get_fs_byte(name + ans);
	return cnt;
}


int sys_whoami(char* name, unsigned int size)
{
	int cnt, ans;
	cnt = 0;
	while(usname[cnt] != '\0' && cnt < MAXNUM)
		cnt++;
	if(cnt > size)
	{
		return -EINVAL;
	}
	
	for(ans = 0; ans <= cnt; ans++)
		put_fs_byte(usname[ans], (name + ans));
	return cnt;
}
