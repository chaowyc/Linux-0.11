#include    <stdlib.h> 
#include    <unistd.h> 
#include    <linux/kernel.h> 
#include    <fcntl.h> 
#include    <linux/types.h> 
#include    <sys/stat.h> 
#include <semaphore.h>

#define BUFFERSIZE    10 

sem_t *empty; // = sem_open("empty",O_CREAT | O_EXCL,0644,BUFFERSIZE); 
sem_t *full; // = sem_open("full",O_CREAT | O_EXCL,0644,0); 
sem_t *mutex; // = sem_open("mutex",O_CREAT | O_EXCL,0644,1); 

int main(void) 
{ 
    char err_desc[255]; 
    char empty_sem[]= "empty"; 
    char full_sem[]= "full"; 
    char mutex_sem[]="mutex"; 
    int in = open("pc.log", O_CREAT|O_RDWR, 0666); 
    int of = open("pc.log",O_CREAT|O_RDWR,0666); 
    int itemValue = -1; 
    int fileLen,tmpValue,i,j; 
    sem_unlink("empty");
    sem_unlink("full");
    sem_unlink("mutex");
    //文件大小设置为0； 
    ftruncate(in,0); 
    empty = sem_open(empty_sem,O_CREAT,0644,BUFFERSIZE); 
    if(empty == SEM_FAILED) 
    { 
        printf("create semaphore empty error!\n"); 
        exit(1); 
    } 
    full = sem_open(full_sem,O_CREAT,0644,0); 
    if(full == SEM_FAILED) 
    { 
        printf("create semaphore full error!\n"); 
        exit(1); 
    } 

    mutex = sem_open(mutex_sem,O_CREAT,0644,1); 
    if(mutex == SEM_FAILED) 
    { 
        printf("create semaphore mutex error!\n"); 
        exit(1); 
    } 

    /* 测试用来查看定义是否真的成功
    sem_getvalue(empty,&tmpValue); 
    printf("now empty's value = %d\n",tmpValue); 
    sem_getvalue(mutex,&tmpValue); 
    printf("now mutex's value = %d\n",tmpValue); 
    sem_getvalue(full,&tmpValue); 
    printf("now full's value = %d\n",tmpValue); 
         */
    if(!fork()) 
    { 
        //printf("producer process %u !now itemValue=%d\n",getpid(),itemValue); 
        while(itemValue<50) //根据实验要求，这里是499就不输出500
        { 
            itemValue++; 
            if(sem_wait(empty)!=0) 
            { 
                printf("in producer sem_wait(empty) error!\n"); 
                perror(err_desc); 
                break; 
            } 
            //producer 
            if(sem_wait(mutex)!=0) 
            { 
                printf("in producer sem_wait(mutex) error!\n"); 
                perror(err_desc); 
                //printf("error msg : %s\n",err_desc); 
                break; 
            } 
            //将数值写到文件尾部 
            lseek(in,0,SEEK_END); 
            write(in,&itemValue,sizeof(itemValue)); 
            if(sem_post(mutex)!=0) 
            { 
                printf("in producer sem_post(mutex) error!\n"); 
                perror(err_desc); 
                break; 
            } 
            //发送full信号 
            if(sem_post(full)!=0) 
            { 
                printf("in producer sem_post(full) error!\n"); 
                perror(err_desc); 
                break; 
            } 

        } //producer process 

        //如果此处退出生产者，则会导致消费者也异外退出且会多生成一个消费者原因不明。
        while(1) ; 
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
                //读取第一个数值并显示 
                lseek(of,0,SEEK_SET); 
                read(of,&itemValue,sizeof(itemValue)); 
                printf("%u:%d\n",getpid(),itemValue); 
                //将其余数值前移，截断文件4个字节 
                fileLen=lseek(in,0,SEEK_END); 
                for(j=1;j<(fileLen/sizeof(itemValue));j++) 
                { 
                    lseek(in,j*sizeof(itemValue),SEEK_SET); 
                    read(in,&tmpValue,sizeof(tmpValue)); 
                    lseek(in,(j-1)*sizeof(itemValue),SEEK_SET); 
                    write(in,&tmpValue,sizeof(tmpValue)); 
                } 
                ftruncate(in,fileLen-sizeof(itemValue)); 

                if(sem_post(mutex)!=0) 
                { 
                    printf("in customer %u,sem_post(empty) error!\n",getpid()); 
                    perror(err_desc); 
                    break; 
                } 
                //发送empty 
                if(sem_post(empty)!=0) 
                { 
                    //此时是否表明已经生产结束？ 
                    printf("in customer %u,sem_post(empty) error!\n",getpid()); 
                    perror(err_desc); 
                } 

            } 
            close(of); 
        } 
    } 

    printf("now, main process exit!\n"); 
    //return 
    return 0; 
}