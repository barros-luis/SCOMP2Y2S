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

void handle_parent(int sig) {
    char msg[80];
    int length;
    length = snprintf(msg, 80, "Parent: I am going to read!\n");
    write(STDOUT_FILENO, msg, length);
}

void handle_child(int sig) {
    char msg[80];
    int length;
    length = snprintf(msg, 80, "Child: I am going to erase and write!\n");
    write(STDOUT_FILENO, msg, length);
}

int main() {
    // Set up signal handlers
    struct sigaction act_parent, act_child;
    
    // Parent handler for SIGUSR1
    memset(&act_parent, 0, sizeof(act_parent));
    act_parent.sa_handler = handle_parent;
    act_parent.sa_flags = SA_RESTART;
    sigaction(SIGUSR1, &act_parent, NULL);
    
    // Child handler for SIGUSR2
    memset(&act_child, 0, sizeof(act_child));
    act_child.sa_handler = handle_child;
    act_child.sa_flags = SA_RESTART;
    sigaction(SIGUSR2, &act_child, NULL);
    
    // Block all signals except SIGUSR1 and SIGUSR2
    sigset_t mask;
    sigemptyset(&mask);
    sigfillset(&mask);
    sigdelset(&mask, SIGUSR1);
    sigdelset(&mask, SIGUSR2);
    
    // Apply signal mask
    if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1) {
        perror("sigprocmask failed");
        return 1;
    }
    
    pid_t pid = fork();
    
    if (pid == 0) {
        // Child process
        while (1) {
            // Check for pending SIGINT
            sigset_t pending;
            sigpending(&pending);
            if (sigismember(&pending, SIGINT)) {
                printf("Child: SIGINT pending, terminating\n");
                exit(EXIT_SUCCESS);
            }
            
            // Write a random number to file
            srand(time(NULL) + getpid());
            int num_to_write = (rand() % 9) + 1;
            
            FILE *file = fopen("data.txt", "w");
            if (file != NULL) {
                fprintf(file, "%d", num_to_write);
                fclose(file);
                printf("Child: Wrote number %d to file\n", num_to_write);
            }
            
            // Signal parent to read
            kill(getppid(), SIGUSR1);
            
            // Wait for parent's signal to continue
            pause();
        }
    } else {
        // Parent process
        while (1) {
            // Check for pending SIGINT
            sigset_t pending;
            sigpending(&pending);
            if (sigismember(&pending, SIGINT)) {
                printf("Parent: SIGINT pending, terminating\n");
                kill(pid, SIGTERM);
                waitpid(pid, NULL, 0);
                exit(EXIT_SUCCESS);
            }
            
            // Wait for child's signal to read
            pause();
            
            // Read number from file
            FILE *file = fopen("data.txt", "r");
            if (file != NULL) {
                char numRead[10];
                if (fgets(numRead, sizeof(numRead), file) != NULL) {
                    printf("Parent: I read the number %s!\n", numRead);
                }
                fclose(file);
            }
            
            // Signal child to continue
            kill(pid, SIGUSR2);
        }
    }
    
    return 0;
}