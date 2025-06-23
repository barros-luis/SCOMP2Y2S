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

typedef struct {
    char name[50];
    char region[50];
    int phone;
}record_t;

volatile sig_atomic_t found = 0;

void found_handler(int sig) {
    found = 1;
}

int main() {
    record_t database[10000];
    pid_t pids[10];
    
    printf("Please tell us what is your region :\n");
    char buffer[256];
    fgets(buffer,sizeof(buffer),stdin);
    buffer[strcspn(buffer,"\n")] = '\0';

    // Set up signal handler for SIGUSR1
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_handler = found_handler;
    sigaction(SIGUSR1, &act, NULL);

    for (int i = 0; i < 10; i++){
        pid_t pid = fork();
        int start = i * 1000;
        int finish = start + 1000;

        if (pid == 0){
            for (int j = start; j < finish; j++){
                if (strcmp(database[j].region,buffer) == 0){
                    printf("WORKSHOP FOUND - (%s) with phone number %d !\n",database[j].name,database[j].phone);
                    kill(getppid(),SIGUSR1);
                    exit(EXIT_SUCCESS);
                }
            }
            exit(EXIT_SUCCESS);
        } else {
            pids[i] = pid;
        }
    }

    // Wait for a child to signal that the region was found
    while (!found) {
        pause();
    }
    
    // Terminate all children
    for (int i = 0; i < 10; i++){
        kill(pids[i],SIGTERM);
    }
    // Optionally, wait for all children to finish
    for (int i = 0; i < 10; i++){
        waitpid(pids[i], NULL, 0);
    }
    
    exit(EXIT_SUCCESS);
}