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

sig_atomic_t volatile keep_going;

int main() {
    sem_t *sem_west;
    sem_t *sem_east;

    sem_east = sem_open("/sem_east", O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR, 0);
    sem_west = sem_open("/sem_west", O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR, 1);

    keep_going = 1;
    while(keep_going){
        printf("Do you want to keep going? (y/n)\n");
        char buffer[12];
        fgets(buffer,sizeof(buffer), stdin);
        buffer[strcspn(buffer,"\n")] = '\0';

        if (strcmp(buffer,"n") == 0){
            keep_going = 0;
            break;
        }

        for (int i = 0; i < 2; i++){ // Fork 2 processes to simulate cars from east and west
            pid_t pid = fork();

            if (pid == 0){
                if (i == 0){ // simulate cars coming from east
                    sem_wait(sem_west);
                    printf("Traversing the bridge towards coming from east..VRUUUM\n");
                    sleep(1);
                    sem_post(sem_east);
                    exit(EXIT_SUCCESS);
                } else if (i == 1){ // simulate cars coming from west
                    sem_wait(sem_east);
                    printf("Traversing the bridge coming from west..VRUUUM\n");
                    sleep(1);
                    sem_post(sem_west);
                    exit(EXIT_SUCCESS);
                }
            }
        }

        // Wait for both children to finish before next iteration
        for (int i = 0; i < 2; i++){
            wait(NULL);
        }
    }
    
    sem_close(sem_east);
    sem_close(sem_west);
    sem_unlink("/sem_east");
    sem_unlink("/sem_west");

    exit(EXIT_SUCCESS);
}