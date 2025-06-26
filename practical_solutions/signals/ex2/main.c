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

void signal_handler(int sig){
    const char *msg = "Division by zero occured!\n";
    write(1,msg,strlen(msg));
    _exit(EXIT_FAILURE);
}


int main(int argc,char *argv[]) {
    if (argc < 3){
        perror("Usage command integer integer\n");
        exit(EXIT_FAILURE);
    }

    int num_1 = atoi(argv[1]);
    int num_2 = atoi(argv[2]);

    struct sigaction act;
    memset(&act,0,sizeof(act));
    act.sa_handler = signal_handler;
    act.sa_flags = SA_RESTART;
    sigaction(SIGUSR1,&act,NULL);

    int result = 0;

    if (num_2 == 0){
        kill(getpid(),SIGUSR1);    
    } else {
        result = num_1/num_2;
    }

    printf("The final result is %d !\n",result);
    
    exit(EXIT_SUCCESS);
}