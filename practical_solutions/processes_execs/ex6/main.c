#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <time.h>

int main() {
    int data[100000];
    int result[100000];

    srand(time(NULL));

    for (int i= 0; i < 100000;i++){
        data[i] = (rand() % 10) + 1;
    }

    pid_t pid;
    pid = fork();
    if (pid < 0){
        perror("fork failed\n");
        exit(EXIT_FAILURE);
    }

    if (pid == 0){
        for (int i = 0; i < 50000;i++){
            result[i] = data[i]*4 + 20;
        }
        printf("Calculations done from me - Child\n");
        exit(EXIT_SUCCESS);
    }

    for (int i = 50000; i < 100000;i++){
        result[i] = data[i]*4 + 20;
    }
    
    printf("Calculations done from me - Parent\n");
    
    int status;
    waitpid(pid, &status, 0);
    
    for (int i = 0; i < 100000; i++){
        printf("The value being read is %d\n", result[i]);
    }
    
    exit(EXIT_SUCCESS);
}