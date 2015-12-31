#define NR_SEMAPHORE    64
#define NR_SEMANAME    255
typedef struct semaphore 
{ 
    char sem_name[NR_SEMANAME]; 
    int value; 
    struct task_struct * semp; 
}sem_t;