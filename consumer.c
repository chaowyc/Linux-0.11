#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include    <linux/kernel.h>
#include    <fcntl.h>
#include    <linux/types.h>
#include    <sys/stat.h>
#include    <sys/shm.h>

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
    int get_count=0;

    char err_desc[255];
    char mutex_sem[]= "mutex";
    int itemValue = -1;

    int shmid_main;
    key_t key;    
    struct int_buffer * logbuf;
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


    //
    logbuf=(struct int_buffer *)shmat(shmid_main,NULL,0);
    if((long)logbuf==-1)
    {
        printf("in producer shmat(shmid_main,NULL,0) error!\n");
        perror(err_desc);
        exit(-1);
    }

    while(get_count<500)
    {
        //
        while(logbuf->int_count<=0)
            ;


        if(sem_wait(mutex)!=0)
        {
            printf("in customer %u,sem_post(empty) error!",getpid());
            perror(err_desc);
            break;
        }


        itemValue=logbuf->data[logbuf->tail];
        logbuf->int_count--;
        (logbuf->tail)++;
        if(logbuf->tail>=BUFFERSIZE)
        {
            logbuf->tail=0;
        }
        printf("customer: %u:%d\n",getpid(),itemValue);
        get_count++;


        if(sem_post(mutex)!=0)
        {
            printf("in customer %u,sem_post(empty) error!\n",getpid());
            perror(err_desc);
            break;
        }

    }

    //detach the shared memory
    if(shmdt(logbuf)!=0)
    {
        printf("in customer shmdt(logbuf) error!\n");
        perror(err_desc);
    }
    //delete the shared memory
    if(shmctl(shmid_main,IPC_RMID,0)==-1)
    {
        printf("in customer shmctl(shmid，IPC_RMID，0) error!\n");
        perror(err_desc);
    }

    sem_unlink("mutex");

    return 0;
}