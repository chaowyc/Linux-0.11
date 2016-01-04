#include    <stdlib.h> 
#include    <unistd.h> 
#include    <linux/kernel.h> 
#include    <fcntl.h> 
#include    <linux/types.h> 
#include    <sys/stat.h> 

#define BUFFERSIZE    10 

int main(void) 
{ 
    char err_desc[255]; 
    int in = open("pc.log", O_CREAT|O_RDWR, 0666); 
    int of = open("pc.log",O_CREAT|O_RDWR,0666); 
    int itemValue = -1; 
    int fileLen,tmpValue,i,j; 

    //文件大小设置为0； 
    ftruncate(in,0); 

    if(!fork()) 
    { 
        //printf("producer process %u !now itemValue=%d\n",getpid(),itemValue); 
        while(itemValue<50) //根据实验要求，这里是499就不输出500
        { 
            itemValue++; 
            //将数值写到文件尾部 
            lseek(in,0,SEEK_END); 
            write(in,&itemValue,sizeof(itemValue)); 
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
            } 
            close(of); 
        } 
    } 

    printf("now, main process exit!\n"); 
    //return 
    return 0; 
}