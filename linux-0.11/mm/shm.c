#define __LIBRARY__
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#include <linux/sched.h>
#include <asm/segment.h>
#include <string.h>
#include <linux/kernel.h>

#define LOW_MEM 0x100000

static struct shmid_ds shm_list[NR_SHM]={{0,0,0,0}};
extern void add_mem_count(long addr);
extern void remove_page(long addr);

int sys_shmget(int key,int size,int shmflag)
{

    int i;
    int return_key=-1;
    unsigned long page;


    if(size>PAGE_SIZE)
    {
        return -EINVAL;
    }


    for(i=0;i<NR_SHM;i++)
    {
        if(shm_list[i].key==key)
        {
            return i;
        }
    }


    for(i=0;i<NR_SHM;i++)
    {
        if(shm_list[i].key==0)
        {

            return_key=i;
            break;
        }
    }


    page=get_free_page();
    printk("shmget get memory's address is 0x%08x\n",page);
    if(page==0)
    {
        return -ENOMEM;
    }


    shm_list[return_key].key=key;
    shm_list[return_key].size=size;
    shm_list[return_key].page=page;
    shm_list[return_key].attached=0;


    return return_key;
}


void * sys_shmat(int shmid,const void *shmaddr, int shmflag)
{
    unsigned long data_base;
    unsigned long brk;
    unsigned long page;


    if(shm_list[shmid].key<=0 || shm_list[shmid].page==0)
    {
        return (void*)-EINVAL;
    }


    data_base=get_base(current->ldt[2]);
    printk("current's data_base = 0x%08x,new page = 0x%08x\n",data_base,shm_list[shmid].page);

    brk=current->brk+data_base;
    current->brk +=PAGE_SIZE;

    page=put_page(shm_list[shmid].page,brk);
    if(page==0)
    {

        return (void*)-ENOMEM;
    }

    shm_list[shmid].attached++;
    return (void *)(brk - data_base);

}


int sys_shmdt(int shmid)
{
    unsigned long data_base;

    if(shm_list[shmid].key<=0 && shm_list[shmid].page==0)
    {
        return -EINVAL;
    }
    shm_list[shmid].attached--;

    data_base=get_base(current->ldt[2]);
    current->brk-=PAGE_SIZE;

    remove_page(data_base+current->brk);

    return 0;
}


int sys_shmctl(int shmid,int shmcmd, struct shmid_ds * buf)
{
    int ret=0;
    switch(shmcmd)
    {
        case 0:
            if(shm_list[shmid].attached>=0 && shm_list[shmid].page!=0)
            {
                free_page(shm_list[shmid].page);
            }
            else
            {
                ret=-EINVAL; 
            }
            break;
    }

    return ret;

}