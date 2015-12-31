#define __LIBRARY__ 
#include    <stdio.h> 
#include    <stdlib.h> 
#include    <unistd.h> 
#include    <linux/kernel.h> 
#include    <fcntl.h> 
#include    <sys/types.h> 
#define BUFFERSIZE    10 

static _syscall2(sem_t *,sem_open,const char *,name,int,value); 
static _syscall1(int,sem_post,sem_t *,sem); 
static _syscall1(int,sem_wait,sem_t *,sem); 
static _syscall1(int,sem_getvalue,sem_t *,sem); 
static _syscall1(int,sem_unlink,const char*,name); 


sem_t *empty; 
sem_t *full; 
sem_t *mutex; 

int main(void) 
{ 

    char err_desc[255]; 
    char empty_sem[]= "empty"; 
    char full_sem[]= "full"; 
    char mutex_sem[]="mutex"; 

    int in = open("pc.log", O_CREAT|O_TRUNC|O_RDWR, 0666); 
    int of = open("pc.log", O_CREAT|O_TRUNC|O_RDWR, 0666); 
    int log = open("pclog.log", O_CREAT|O_TRUNC|O_RDWR, 0666); 
    char buflog[255];
    int tmp; 

    int itemValue = -1; 
    int fileLen,tmpValue,i,j; 
    if(fcntl(in,F_CHSIZE,0)!=0) 
    { 
        printf("in main process. ftruncate error!\n"); 
        close(in); 
        close(of); 
        return 0; 
    } 
    empty = sem_open(empty_sem,BUFFERSIZE); 

    if(empty == NULL) 
    { 
        printf("create semaphore empty error!\n"); 
        exit(1); 
    }     

    full = sem_open(full_sem,0); 
    if(full ==NULL) 
    { 
        printf("create semaphore full error!\n"); 
        exit(1); 
    } 

    mutex = sem_open(mutex_sem,1); 
    if(mutex == NULL) 
    { 
        printf("create semaphore mutex error!\n"); 
        exit(1); 
    } 

    tmpValue=sem_getvalue(empty); 
    printf("now empty's value = %d\n",tmpValue); 

    tmpValue=sem_getvalue(mutex); 
    printf("now mutex's value = %d\n",tmpValue); 

    tmpValue=sem_getvalue(full); 
    printf("now full's value = %d\n",tmpValue); 

    if(!fork()) 
    { 
        printf("producer process %u !now itemValue=%d\n",getpid(),itemValue); 

        while(itemValue<50) 
        { 
            itemValue++; 

            if(sem_wait(empty)!=0) 
            { 
                printf("in producer sem_wait(empty) error!\n"); 
                perror(err_desc); 
                break; 
            } 


            if(sem_wait(mutex)!=0) 
            { 
                printf("in producer sem_wait(mutex) error!\n"); 
                perror(err_desc); 

                break; 
            } 

            fileLen=lseek(in,0,SEEK_END); 
            write(in,&itemValue,sizeof(itemValue)); 

            if(sem_post(mutex)!=0) 
            { 
                printf("in producer sem_post(mutex) error!\n"); 
                perror(err_desc); 
                break; 
            } 

            if(sem_post(full)!=0) 
            { 
                printf("in producer sem_post(full) error!\n"); 
                perror(err_desc); 
                break; 
            } 

        } 
        /*
        tmpValue=sem_getvalue(empty); 
        printf("now empty's value = %d\n",tmpValue); 

        tmpValue=sem_getvalue(mutex); 
        printf("now mutex's value = %d\n",tmpValue); 

        tmpValue=sem_getvalue(full); 
        printf("now full's value = %d\n",tmpValue); 
        */
        while(1) 
            ; 

        close(in); 
    } 

    for(i=0; i < 5; i++) 
    { 
        if(!fork()) 
        { 
            printf("customer process(%u) begin to run!\n",getpid()); 

            while(1) 
            { 

                if(sem_wait(full)!=0) 
                { 
                    printf("in customer %u sem_wait(full) error!\n",getpid()); 
                    perror(err_desc); 
                    break; 
                } 

                if(sem_wait(mutex)!=0) 
                { 
                    printf("in customer %u,sem_post(empty) error!",getpid()); 
                    perror(err_desc); 
                    break; 
                } 

                lseek(of,0,SEEK_SET); 
                read(of,&itemValue,sizeof(itemValue)); 
                
                lseek(log,0,SEEK_END); 
                sprintf(buflog,"%u:%d\n",getpid(),itemValue); 
                write(log,&buflog,sizeof(char)*strlen(buflog)); 

                
                fileLen=lseek(in,0,SEEK_END); 
                for(j=1;j<(fileLen/sizeof(itemValue));j++) 
                { 
                    lseek(in,j*sizeof(itemValue),SEEK_SET); 
                    read(in,&tmpValue,sizeof(tmpValue)); 
                    lseek(in,(j-1)*sizeof(itemValue),SEEK_SET); 
                    write(in,&tmpValue,sizeof(tmpValue)); 
                } 

                if(fcntl(in,F_CHSIZE,fileLen-sizeof(tmpValue))!=0) 
                { 
                    printf("ftruncate error!\n"); 
                    break; 
                } 

                if(sem_post(mutex)!=0) 
                { 
                    printf("in customer %u,sem_post(empty) error!\n",getpid()); 
                    perror(err_desc); 
                    break; 
                } 

                if(sem_post(empty)!=0) 
                { 

                    printf("in customer %u,sem_post(empty) error!\n",getpid()); 
                    perror(err_desc); 

                } 
            } 

            close(of); 
        } 
    } 

    printf("now, main process exit!\n"); 

    return 0; 
}