#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/kernel.h>
#include <fcntl.h>
#include <linux/types.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <sys/types.h>

#define BUFFERSIZE    10
struct int_buffer
{
    int int_count;
    int head;
    int tail;
    int data[BUFFERSIZE];
};

sem_t *mutex; // = sem_open("mutex",O_CREAT | O_EXCL,0644,BUFFERSIZE);

int main(void)
{
    char err_desc[255];
    char mutex_sem[]= "mutex";
    int itemValue = -1;

    key_t key;
    struct int_buffer * logbuf;
    int shmid_main;

    key = ftok("./tmp.txt",0x03);
    shmid_main=shmget(key,sizeof(struct int_buffer),IPC_CREAT|0666);
    if(shmid_main == -1)
    {
        printf("shmget(1234,..) error!\n");
        perror(err_desc);
        return -1;
    }

    mutex = sem_open(mutex_sem,O_CREAT,0644,1);
    if(mutex == SEM_FAILED)
    {
        printf("create semaphore mutex error!\n");
        return 1;
    }



    logbuf=(struct int_buffer *)shmat(shmid_main,NULL,0);
    if((long)logbuf==-1)
    {
        printf("in producer shmat(shmid_main,NULL,0) error!\n");
        perror(err_desc);
        exit(-1);
    }

    while(itemValue<499)
    {
        itemValue++;

        printf("producer(%u) int_count=>%d  itemValue=> %d\n",getpid(),logbuf->int_count,itemValue);
        while(logbuf->int_count==10)
            ;
        if(sem_wait(mutex)!=0)
        {
            printf("in producer sem_wait(mutex) error!\n");
            perror(err_desc);
            break;
        }
        //
        logbuf->int_count+=1;
        logbuf->data[logbuf->head]=itemValue;
        (logbuf->head)++;
        if(logbuf->head>=BUFFERSIZE)
        {
            logbuf->head=0;
        }


        if(sem_post(mutex)!=0)
        {
            printf("in producer sem_post(mutex) error!\n");
            perror(err_desc);
            break;
        }


    }

    //
    if(shmdt(logbuf)!=0)
    {
        printf("in producer shmdt(logbuf) error!\n");
        perror(err_desc);
    }

    return 0;
}