
// 2020/10/23 //

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

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
double neg_exp_distribute(double arg_p){
    double res;
    do
    {
        res = ((double)rand() / RAND_MAX);
    }while((res==0) || (res==1));
    res = -(1/arg_p*log(res));
    return res;
}

// 生产者线程核心函数
void* prod_func(void *arg_p_ptr)
{
    double arg_p = (double)(*(int*) arg_p_ptr);
    pthread_t temp_thread = pthread_self();

    while(1)
    {        
        int interval = neg_exp_distribute(arg_p); // 写入数据的时间间隔
        usleep((unsigned int)(neg_exp_distribute(interval)*1e6)); // 以时间间隔为依据休眠对应时间
        int item = rand() % 14; // 随机产生需要写入的数据

        struct buf *shared_mem_ptr = ((struct buf*) ptr); // 空指针强制类型转换为缓冲区结构体指针，准备进行数据写入
        sem_wait(empty); // 等缓冲区空，准备写入数据
        sem_wait(s_mutex); // 获取缓冲区互斥锁后才能写入

        // 写入并更新循环队列尾部指针
        shared_mem_ptr->buffer[shared_mem_ptr->rear] = item;
        shared_mem_ptr->rear = (shared_mem_ptr->rear + 1) % MAX_BUF_SIZE;
        // 打印写入信息
        printf("Writing item... buffer: [%d]\tthread id: [%lu]\titem content: (%d)\n",shared_mem_ptr->rear, (unsigned long)temp_thread, item);
        
        sem_post(s_mutex); // 释放缓冲区互斥锁
        sem_post(full); // 数据已写满，设缓冲区状态为满
    }

    pthread_exit(0);
}

int main(int argc, char* argv[])
{   
    struct buf shared_mem; // 缓冲区结构体实例
    memset(&shared_mem, 0, sizeof(struct buf)); // 初始化缓冲区，全设为0

    // sem_open的第二个参数O_CREAT表示如果不存在指定标识符的信号量就创建，第三个参数表示创建后的默认权限，但由于下列三个信号量我们已在全局声明，该函数就仅仅是初始化它们
    full = sem_open("full", O_CREAT, 0666, 0);
    empty = sem_open("empty", O_CREAT, 0666, 0);
    s_mutex = sem_open("mutex", O_CREAT, 0666, 0);

    // sem_init的第二个参数设置为1表示本进程中的线程不可共享信号量，第三个参数表示信号量需要被初始化的值
    sem_init(full, 1, 0);
    sem_init(empty, 1, MAX_BUF_SIZE);
    sem_init(s_mutex, 1, 1);

    int shared_mem_fd = shm_open("buffer", O_CREAT | O_RDWR, 0666); // 通过共享内存进行数据共享
    ftruncate(shared_mem_fd, sizeof(struct buf)); // 将其截断到一个缓冲区结构体的规模
    ptr = mmap(0, sizeof(struct buf), PROT_WRITE, MAP_SHARED, shared_mem_fd,0); // 将其映射到文件中以备生产者进程访问

    int arg_p = atoi(argv[1]); // 读取控制参数p，并转为整形数
    pthread_t td[MAX_THREADS]; // 线程标识符数组
    int err[MAX_THREADS];
    for(int ii = 0; ii < MAX_THREADS; ii++) // 创建MAX_THREADS个线程
    {
        err[ii] = pthread_create(&td[ii], NULL, prod_func, (void*)&arg_p);
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

    return 0;
}
