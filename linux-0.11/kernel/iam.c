
#include <linux/sched.h>
#include <linux/tty.h>
#include <linux/kernel.h>


int sys_iam(const char* name)
{
	printk("you are win\n");
	return 0;
}