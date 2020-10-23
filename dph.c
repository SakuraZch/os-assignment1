
// 2020/10/22 //

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

// 打印宏
#define PHL_DEBUG
#ifdef PHL_DEBUG
#define PHL_PRT(...) do{printf("[%s]", __func__); printf(""__VA_ARGS__);}while(0)
#else
#define PHL_PRT(...)
#endif

#define PHILOSHOPHER_NUM 5 // 定义哲学家最大数目

sem_t fork_num[PHILOSHOPHER_NUM]; // 叉子数组

void time_of_think_or_eat(void)
{
    int time = (rand()%PHILOSHOPHER_NUM) + 1; // 思考时间，1 ~ 5
    printf("time = %d s\n", time);
    sleep(time);
}

void* pickup_forks(void* philoshopher_id)
{
    int p_id = *((int *)philoshopher_id);

    // 定义左右手变量表示哲学家左右手的叉子
    int left_fork = (p_id-1+PHILOSHOPHER_NUM)%PHILOSHOPHER_NUM;             
    int right_fork = p_id;
    
    int left_get = 0;
    int right_get = 0;

    while(1)
    {
        // 哲学家要思考了
        PHL_PRT("philosopher %d is going to think ", p_id);
        time_of_think_or_eat();

        // 获取左右手的叉子
        left_get = sem_trywait(&fork_num[left_fork]);
        right_get = sem_trywait(&fork_num[right_fork]);
 
        // 哲学家通过获取左右手的叉子来判断是否可以吃饭
        if(left_get == 0 && right_get == 0) // 都为 0 表示 OK ,可以吃饭
        {
            PHL_PRT("philosopher %d going to eat ", p_id);
            time_of_think_or_eat();

            // 放回叉子，即return_forks
            sem_post(&fork_num[left_fork]);
            sem_post(&fork_num[right_fork]);
        }
        else if(left_get == 0 && right_get != 0) // 左手获取了叉子，等待1s，判断右手看是否有叉子
        {
            sleep(1);
            right_get = sem_trywait(&fork_num[right_fork]);  // 再次获取右手叉子
            if(right_get == 0) // 获取成功
            {
                PHL_PRT("Philosopher %d going to eat ", p_id);
                time_of_think_or_eat();

                // 放回叉子，即return_forks
                sem_post(&fork_num[left_fork]);
                sem_post(&fork_num[right_fork]);
            }
            else
            {
                sem_post(&fork_num[right_fork]);
            }
        }
        else if(right_get == 0 && left_get != 0) // 右手获取了叉子，等待 1 s，判断左手看是否有叉子
        {
            sleep(1);
            left_get = sem_trywait(&fork_num[left_fork]); // 再次获取右手叉子
            if (left_get == 0) // 获取成功
            {
                PHL_PRT("Philosopher %d is going to eat ", p_id);
                time_of_think_or_eat();

                // 放回叉子，即return_forks
                sem_post(&fork_num[left_fork]);
                sem_post(&fork_num[right_fork]);
            }
            else
            {
                sem_post(&fork_num[left_fork]);
            }
        }
        else  // 表示左右手都没有获得叉子，直接进入思考
        {
            continue;
        }
    }
}

int main(int argc, char* argv[])
{
    srand((unsigned)time(NULL)); // 种下时间种子，产生随机数表示哲学家的思考和吃饭时间
 
    int ii = 0;
    for(ii = 0; ii < PHILOSHOPHER_NUM; ii++)          
    {
        if(sem_init(&fork_num[ii], 0, 1) < 0) // 初始化信号量为 1，且共享（参数2设为0）
        {
            perror("sem_init error!\n");
            exit (-1);
        }
        else
        {
            printf("semaphore %d is created\n", ii);
        }
    }
    
    // 模拟思考吃饭过程
    pthread_t td[PHILOSHOPHER_NUM];
    int err[PHILOSHOPHER_NUM];
    int jj = 0, kk = 0;
    for(jj = 0; jj < PHILOSHOPHER_NUM; jj++)
    {
        err[jj] = pthread_create(&td[jj], NULL, pickup_forks, (void*)&jj);
        if(err[jj] < 0)
        {
            perror("Philosopher create failed!\n");
            for(kk = 0; kk < PHILOSHOPHER_NUM; kk++)
            {
                if(sem_destroy(&fork_num[kk]) < 0)
                {
                    perror("Semaphore error!\n");
                    exit(-1);
                }
            }
            exit(-1);
        }
        usleep(100);
    }
   
    int tmp = 0;
    while(1)
    {
        printf("-----------1s passed-------------\n");
        sleep(1); // 主函数睡眠
        tmp++;
        if(tmp == 1000)
        {
            break;
        }
    }

    for(ii = 0; ii < PHILOSHOPHER_NUM; ii++)          
    {
        if(sem_destroy(&fork_num[ii]) < 0) // 初始化信号量为 1，且共享
        {
            perror("sem_init error!\n");
            exit (-1);
        }
        else
        {
            printf("semaphore %d is destoryed\n", ii);
        }
    }

    return 0;
}