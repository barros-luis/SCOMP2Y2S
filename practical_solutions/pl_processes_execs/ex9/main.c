#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#define CHILDREN 6
#define TOTAL 1200000

int main() {
    pid_t pid;
    int part = TOTAL / CHILDREN;
    
    for (int i = 0; i < CHILDREN; i++){
        int start = i * part + 1;  // Calculate start before fork
        int end = (i + 1) * part;  // Calculate end before fork
        
        pid = fork();

        if (pid < 0) {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        }

        if (pid == 0){
            // Child process
            for (int idx = start; idx <= end; idx++){
                printf("Current number is %d\n", idx);
            }
            exit(EXIT_SUCCESS);
        }

    }

    for (int i= 0; i < CHILDREN;i++){
        int status;
        wait(&status);
    }
    
    exit(EXIT_SUCCESS);
}