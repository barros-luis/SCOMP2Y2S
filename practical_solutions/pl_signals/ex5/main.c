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

int main(int argc,char *argv[]) {
    if (argc < 2){
        fprintf(stderr, "Usage: %s prog1 ... progn\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int n = argc - 1;
    pid_t pids[n];

    for (int i = 0; i < n; i++) {
        pid_t pid = fork();
        if (pid == 0){
            execlp(argv[i+1], argv[i+1], NULL);
            perror("execlp failed");
            exit(EXIT_FAILURE);
        } else {
            pids[i] = pid;
        }
    }

    sleep(1);

    // Stop all except the first
    for (int i = 1; i < n; i++){
        kill(pids[i], SIGSTOP);
    } 

    int current = 0;
    while(1){
        sleep(5);
        kill(pids[current], SIGSTOP);
        current = (current + 1) % n;
        kill(pids[current], SIGCONT);
    }

    // Not reached
    exit(EXIT_SUCCESS);
}