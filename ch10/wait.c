#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    int status;
    pid_t pid = fork(); //这里的子进程将在第13行通过 return 语句终止

    if (pid == 0)
    {
        return 3;
    }
    else
    {
        printf("Child PID: %d \n", pid);
        pid = fork(); //这里的子进程将在 21 行通过 exit() 函数终止
        if (pid == 0)
        {
            exit(7);
        }
        else
        {
            printf("Child PID: %d \n", pid);
            wait(&status);         //之间终止的子进程相关信息将被保存到 status 中，同时相关子进程被完全销毁
            if (WIFEXITED(status)) //通过 WIFEXITED 来验证子进程是否正常终止。如果正常终止，则调用 WEXITSTATUS 宏输出子进程返回值
                printf("Child send one: %d \n", WEXITSTATUS(status));

            wait(&status); //因为之前创建了两个进程，所以再次调用 wait 函数和宏
            if (WIFEXITED(status))
                printf("Child send two: %d \n", WEXITSTATUS(status));
            sleep(30);
        }
    }
    return 0;
}
