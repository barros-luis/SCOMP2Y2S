#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define N_CHILDREN 2

int main(){
    pid_t pid;
    pid_t pids[N_CHILDREN];
    int status;

    for (int i = 0; i < N_CHILDREN; i++){
        pid = fork();
        if (pid < 0){
            perror("Fork failed");
            exit(EXIT_FAILURE);
        }

        if (pid == 0){ // child work here
            if (i == 0){
                printf("I am the first child and I have pid of %d\n", getpid());
                sleep(5);
                exit(1);
            }
            if (i == 1){
                printf("I am the second child and I have pid %d\n", getpid());
                exit(2);
            }
        } else { // parent stores the child PID
            pids[i] = pid;
        }
    }

    // Only parent reaches here
    printf("I am the father and I have pid %d\n", getpid());
        
    // Wait for first child, then second child
    for (int i = 0; i < N_CHILDREN; i++){
        waitpid(pids[i], &status, 0);
        if (WIFEXITED(status)){
            printf("Child %d terminated normally with exit value %d\n", i+1, WEXITSTATUS(status));
        }
    }
    
    return EXIT_SUCCESS;
}