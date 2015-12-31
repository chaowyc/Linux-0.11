#define __LIBRARY__ 
#include <unistd.h> 
#include <stdarg.h> 
#include <errno.h> 
#include <asm/segment.h> 
#include <asm/system.h> 
#include <string.h> 
#include <linux/kernel.h> 
#include <linux/sched.h> 

static sem_t semaphore_list[NR_SEMAPHORE]={{"",0,NULL}};

sem_t * sys_sem_open(const char * semname, int value) 
{ 
    int i,cursem=-1; 
    char curname[NR_SEMANAME]; 
    //int name_len=0; 
    char* p=(char *)semname; 
    char c; 
    for(i=0;i<NR_SEMANAME;i++) 
    { 
        c=get_fs_byte(p++); 
        if(c=='\0') 
            break; 
        else 
            curname[i]=c;//sema[cursem].sem_name[i]=c; 
    } 
    curname[i]='\0'; 

    for(i=0; i<NR_SEMAPHORE; i++) 
    { 
        /*if(semaphore_list[i].sem_name[0] == '\0') 
        { 
            cursem=i; 
            break; 
        } 
        else*/ 

        //printk("return semaphore, id is %d,the name is %s,the value is %d\n",i,semaphore_list[i].sem_name,semaphore_list[i].value); 
        if(strcmp(curname,semaphore_list[i].sem_name)==0) 
        { 
            printk("return semaphore, id is %d,the name is %s,the value is %d\n",i,semaphore_list[i].sem_name,semaphore_list[i].value); 
            return &semaphore_list[i]; 
        } 

    } 
    for(i=0; i<NR_SEMAPHORE; i++) 
    { 
        if(semaphore_list[i].sem_name[0] == '\0') 
        { 
            cursem=i; 
            break; 
        } 
    } 

    if(cursem==-1) 
    { 
        printk("now,no blank list,cursem=%d\n",cursem); 
        return NULL; 
    } 
    i=0; 
    for(;curname[i]!='\0';i++) 
    { 
        semaphore_list[cursem].sem_name[i]=curname[i]; 
    } 
    semaphore_list[cursem].sem_name[i]='\0'; 
    semaphore_list[cursem].value=value; 
    semaphore_list[cursem].semp=NULL; 

    return &semaphore_list[cursem]; 
} 

/**/ 
int sys_sem_wait(sem_t * sem) 
{ 
    cli(); 
    sem->value--;  
    if(sem->value<0) 
    { 
        sleep_on(&(sem->semp)); 
    } 
    sti(); 
    return 0; 
} 

/**/ 
int sys_sem_post(sem_t * sem) 
{ 
    cli(); 
    sem->value++; 
    if(sem->value<=0) 
    { 
        //wake_up 
        //printk("process [%u] wakeup!current pointor is %u,semp is %u\n",current->pid,current,sem->semp); 
        wake_up(&(sem->semp)); 
        
    } 
    sti(); 
    return 0; 
} 

int sys_sem_getvalue(sem_t * sem) 
{ 
    if(sem != NULL) 
        return sem->value; 
    else 
        return -1; 
} 
int sys_sem_unlink(const char * name) 
{ 
    int i;//,cursem=-1; 
    char curname[NR_SEMANAME]; 
    //int name_len=0; 
    char* p=(char*)name; 
    char c; 
    for(i=0;i<NR_SEMANAME;i++) 
    { 
        c=get_fs_byte(p++); 
        if(c=='\0') 
            break; 
        else 
            curname[i]=c;//sema[cursem].sem_name[i]=c; 
    } 
    curname[i]='\0'; 
    for(i=0; i<NR_SEMAPHORE; i++) 
    { 
        if(strcmp(curname,semaphore_list[i].sem_name)==0) 
        { 
            semaphore_list[i].sem_name[0]='\0'; 
            semaphore_list[i].value=0; 
            semaphore_list[i].semp=NULL; 
            return 0; 
        } 
    } 
    // 
    return -1; 
}