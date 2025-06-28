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
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <semaphore.h>
#define P_READ 0
#define P_WRITE 1

int main() {
    sem_t *sem1, *sem2, *sem3;
    
    sem1 = sem_open("/sem1", O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR, 1);  // Child 1 starts
    sem2 = sem_open("/sem2", O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR, 0);  // Child 2 waits
    sem3 = sem_open("/sem3", O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR, 0);  // Child 3 waits

    for (int i = 0; i < 3; i++){
        pid_t pid = fork();

        if (pid == 0){
            if (i == 0) {
                sem_wait(sem1);
                printf("Sistemas");
                fflush(stdout);
                sem_post(sem2);
                
                sem_wait(sem1);
                printf(" is");
                fflush(stdout);
                sem_post(sem2);
                exit(EXIT_SUCCESS);
            }
            else if (i == 1) {
                sem_wait(sem2);
                printf(" de");
                fflush(stdout);
                sem_post(sem3);
                
                sem_wait(sem2);
                printf(" the");
                fflush(stdout);
                sem_post(sem3);
                exit(EXIT_SUCCESS);
            }
            else if (i == 2) {
                sem_wait(sem3);
                printf(" Computadores");
                fflush(stdout);
                sem_post(sem1);
                
                sem_wait(sem3);
                printf(" best!");
                fflush(stdout);
                printf("\n");
                fflush(stdout);
                exit(EXIT_SUCCESS);
            }
        }
    }

    for (int i = 0; i < 3; i++){
        wait(NULL);
    }

    sem_close(sem1);
    sem_close(sem2);
    sem_close(sem3);
    sem_unlink("/sem1");
    sem_unlink("/sem2");
    sem_unlink("/sem3");
    
    exit(EXIT_SUCCESS);
}