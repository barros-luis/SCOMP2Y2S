#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <time.h>


char create_twins(pid_t list[2]) {
    pid_t pid1, pid2;

    // Create first child
    pid1 = fork();
    if (pid1 < 0) {
        perror("First fork failed");
        exit(EXIT_FAILURE);
    }

    if (pid1 == 0) {
        // First child process
                return 'a';
    }
    
    // Parent process continues here
    // Create second child
    pid2 = fork();
    if (pid2 < 0) {
        perror("Second fork failed");
        exit(EXIT_FAILURE);
    }
    
    if (pid2 == 0) {
        // Second child process
                return 'b';
            }
    
    list[0] = pid1;  // First child PID
    list[1] = pid2;  // Second child PID
    
    return 'p';
}

int main() {
    pid_t child_pids[6];  // Array to store all 6 child PIDs
    char results[3];      // Array to store results from create_twins calls
    
    // Create 3 pairs of twins (6 processes total)
    for (int i = 0; i < 3; i++) {
        results[i] = create_twins(&child_pids[i * 2]);
        
        if (results[i] == 'p') {
            // Parent process - continue to next iteration
            continue;
        } else if (results[i] == 'a') {
            // First child of this pair
            printf("Child PID: %d, Parent PID: %d\n", getpid(), getppid());
            exit('a');
        } else if (results[i] == 'b') {
            // Second child of this pair
            printf("Child PID: %d, Parent PID: %d\n", getpid(), getppid());
            exit('b');
        }
    }
    
    // Only parent reaches here - wait for all 6 children in order
    printf("Parent PID: %d\n", getpid());
    
    for (int i = 0; i < 6; i++) {
        int status;
        waitpid(child_pids[i], &status, 0);
        
        if (WIFEXITED(status)) {
            printf("Child %d (PID: %d) terminated with exit value: %c\n", 
                   i + 1, child_pids[i], WEXITSTATUS(status));
        }
    }
    
    printf("All children have terminated\n");
    exit(EXIT_SUCCESS);
}