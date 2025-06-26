#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <signal.h>

void signal_handler(int sig,siginfo_t *info, void *context){
    char msg[80];
    int length;

    length = snprintf(msg,80,"I captured a SIGUSR1 sent by the process with PID %d !\n",info->si_pid);
    write(STDOUT_FILENO, msg, length+1);
    _exit(EXIT_SUCCESS);
}

int main() {
    struct sigaction act;
    memset(&act,0,sizeof(act));
    act.sa_sigaction = signal_handler;
    act.sa_flags = SA_SIGINFO | SA_RESTART;
    sigaction(SIGUSR1,&act,NULL);

    pid_t pid = fork();

    if (pid == 0){
        kill(getppid(),SIGUSR1);
        exit(EXIT_SUCCESS);
    }

    int status;
    waitpid(pid,status,0);    
    
    exit(EXIT_SUCCESS);
}