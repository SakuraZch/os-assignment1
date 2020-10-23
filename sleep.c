
// 2020/10/21 //

// ./tmp/xv6-riscv

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
    if(argc <= 0)
    {
        printf("Error, receive no parameter!\n");
        exit(-1);
    }
    else if(argc >= 3)
    {
        printf("Error, receive to many parameters!\n");
        exit(-1);
    }
    else
    {
        int sleepTime = atoi(argv[1]);
        printf("Sleep time: %d\n", sleepTime);
        sleep(sleepTime);
        printf("Wake up!\n");
        exit(0);
    }
}