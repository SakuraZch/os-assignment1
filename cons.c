
// 2020/10/23 //

#include <time.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <semaphore.h>

#define MAX_THREADS 3 // 最大线程数设置为3
#define MAX_BUF_SIZE 20 // 缓冲区最大规模设置为20

// 缓冲区结构体
typedef struct buf	
{		
	int rear;			
	int head;				
	int buffer[MAX_BUF_SIZE];			
}buf;

sem_t *full = 0; // 缓冲区满的信号量
sem_t *empty = 0; // 缓冲区空的信号量
sem_t *s_mutex = 0; // 针对缓冲区结构体的互斥锁
void *ptr = NULL; // 全局空指针，用于写入数据

// 返回负指数分布的采样值
double neg_exp_distribute(double arg_c){
    double res;
    do
    {
        res = ((double)rand() / RAND_MAX);
    }while((res==0) || (res==1));
    res = -(1/arg_c*log(res));
    return res;
}

void *cons_func(void *arc_c_ptr)
{
    double arg_c = *((int*)arc_c_ptr);
    pthread_t temp_thread = pthread_self();

    while(1)
    {
        int interval = neg_exp_distribute(arg_c); // 读取数据的时间间隔
        sleep((unsigned int)neg_exp_distribute(interval)); // 以时间间隔为依据休眠对应时间

        struct buf *shared_mem_ptr = ((struct buf*)ptr);
        sem_wait(full); // 等缓冲区满，准备读取数据
        sem_wait(s_mutex); // 获取缓冲区互斥锁后才能读取

        // 从循环队列中读取数据
        int item = shared_mem_ptr->buffer[shared_mem_ptr->head];
        shared_mem_ptr->head = (shared_mem_ptr->head+1) % MAX_BUF_SIZE;

        // 打印读取到的数据
        printf("Reading item... buffer: [%d]\tthread id: [%lu]\titem content: (%d)\n", shared_mem_ptr->rear, (unsigned long)temp_thread, item);

        sem_post(s_mutex); // 释放缓冲区互斥锁
        sem_post(empty); // 数据已读完，设缓冲区状态为空

        
    }
    pthread_exit(0);
}

int main(int argc, char* argv[])
{   
    // sem_open的第二个参数O_CREAT表示如果不存在指定标识符的信号量就创建，第三个参数表示创建后的默认权限，但由于下列三个信号量我们已在全局声明，该函数就仅仅是初始化它们
    full = sem_open("full", O_CREAT);
    empty = sem_open("empty", O_CREAT);
    s_mutex = sem_open("mutex", O_CREAT);

    int shared_mem_fd = shm_open("buffer", O_RDWR, 0666); // 通过共享内存进行数据共享
    ptr = mmap(0, sizeof(struct buf), PROT_READ | PROT_WRITE, MAP_SHARED, shared_mem_fd, 0); // 将其映射到文件中以备生产者进程访问

    int arg_c = atoi(argv[1]); // 读取控制参数c，并转为整形数
    pthread_t td[MAX_THREADS]; // 线程标识符数组
    int err[MAX_THREADS];
    for(int ii = 0; ii < MAX_THREADS; ii++) // 创建MAX_THREADS个线程
    {
        err[ii] = pthread_create(&td[ii], NULL, cons_func, (void*)&arg_c);
        if(err[ii] < 0)
        {
            perror("Thread create failed!\n");
            exit(-1);
        }
    }

    // 回收资源
    for (int ii = 0; ii < MAX_THREADS; ii++)    
    {
        pthread_join(td[ii], NULL);
    }
    munmap(ptr, sizeof(struct buf));
    shm_unlink("buffer");
    sem_close(full);
    sem_close(empty);

    return 0;  
}