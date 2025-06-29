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

void* string_write(void *string){
    printf("The string is %s !\n", (char*)string);
    fflush(stdout);
    pthread_exit((void*)NULL);
}

int main() {
    char first_name[6] = "Luis";   
    char last_name[8] = "Barros";  

    pthread_t threads[2];

    for (int i = 0; i < 2; i++){
        int result;
        if (i == 0){
            result = pthread_create(&threads[i], NULL, string_write, (void*)first_name);
        } else {
            result = pthread_create(&threads[i], NULL, string_write, (void*)last_name);
        }
        
        if (result != 0) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    }

    write(STDOUT_FILENO, "All threads created!\n", 21);

    for (int i = 0; i < 2; i++){
        int result = pthread_join(threads[i], NULL);
        if (result != 0) {
            perror("pthread_join");
            exit(EXIT_FAILURE);
        }
    }

    exit(EXIT_SUCCESS);
}