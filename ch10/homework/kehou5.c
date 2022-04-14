#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

void ctrl_handling(int sig);

int main(int argc, char *argv[])
{
    struct sigaction act;
    act.sa_handler = ctrl_handling;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGINT, &act, 0);//输入ctrl+c发出信号
    
    while(1)
    {
        sleep(1);
        puts("美好的一天！");
    }
    return 0;
}

void ctrl_handling(int sig)
{
    char c;
    if(sig == SIGINT)
    {
        fputs("Do you want to exit(Y to exit)?", stdout);
        scanf("%c", &c);
        if(c == 'y' || c == 'Y')
            exit(1);
    }
}