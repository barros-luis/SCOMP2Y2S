#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <time.h>

int main(int argc, char *argv[]) {
    printf("Number of arguments: %d\n", argc);
    
    if (argc < 2) {
        printf("Usage: %s <command1> <command2> ...\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    pid_t pids[argc - 1];
    
    for (int i = 1; i < argc; i++) {
        pid_t pid = fork();
        
        if (pid < 0) {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        }
        
        if (pid == 0) {
            printf("Executing: %s\n", argv[i]);
            execlp(argv[i], argv[i], NULL);
            
            perror("execlp failed");
            exit(EXIT_FAILURE);
        } else {
            pids[i - 1] = pid;
        }
    }
    
    printf("Parent waiting for all commands to complete...\n");
    
    for (int i = 0; i < argc - 1; i++) {
        int status;
        waitpid(pids[i], &status, 0);
        
        if (WIFEXITED(status)) {
            printf("Command %d (PID: %d) finished with exit code: %d\n", 
                   i + 1, pids[i], WEXITSTATUS(status));
        }
    }
    
    printf("All commands have completed\n");
    exit(EXIT_SUCCESS);
}