#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    pid_t pid = fork();

    if (pid == 0)
    {
        puts("Hi, I am a child Process");
    }
    else
    {
        printf("Child Process ID: %d \n", pid);
        sleep(30);
    }

    if (pid == 0)
        puts("End child proess");
    else
        puts("End parent process");
    return 0;
}
