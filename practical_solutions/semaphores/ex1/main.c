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
    sem_t *sem;
    sem = sem_open("/sem_ex1", O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR, 1);

    for (int i = 0; i < 5; i++){
        pid_t pid = fork();

        if (pid == 0){
            sem_wait(sem);

            FILE *file = fopen("teste.txt","a+");
            for (int j = 0; j < 200; j++){
                fprintf(file,"%d ",j*i);
            }
            fclose(file);

            sem_post(sem);
            exit(EXIT_SUCCESS);
        }
    }

    for (int i = 0; i < 5; i++){
        wait(NULL);
    }

    printf("Children terminated, lets see the results!\n");

    FILE *file = fopen("teste.txt","r");
    char buffer[1024];
    while(fgets(buffer, sizeof(buffer), file)){
        printf("%s", buffer);
    }

    fclose(file);

    sem_close(sem);
    sem_unlink("/sem_ex1");
    
    exit(EXIT_SUCCESS);
}