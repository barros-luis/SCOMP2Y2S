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
#define P_READ 0
#define P_WRITE 1

#define PROCESSES 10
#define SALES_COUNT 50000

typedef struct{
    int customer_code;
    int product_code;
    int quantity;
} record_t;

int main() {
    record_t sales[SALES_COUNT];
    // Seed the random number generator once at the start
    srand(time(NULL));
    // Populate the sales array with random values
    for (int i = 0; i < SALES_COUNT; i++) {
        sales[i].customer_code = (rand() % 1000) + 1; // Customer codes from 1 to 1000
        sales[i].product_code  = (rand() % 100) + 1;  // Product codes from 1 to 100
        sales[i].quantity      = (rand() % 50) + 1;       // Quantities from 1 to 100
    }

    int fd[2];
    pipe(fd);

    for (int i = 0; i < PROCESSES; i++){
        pid_t pid = fork();

        if (pid == 0){
            close(fd[P_READ]);
            int start = i * 5000;
            int finish = start + 5000;

            for (int j = start; j < finish; j++){
                if (sales[j].quantity > 20){
                    char buffer[32];
                    snprintf(buffer,sizeof(buffer),"%d",sales[j].product_code);
                    write(fd[P_WRITE],buffer,strlen(buffer) + 1);
                }
            }
            close(fd[P_WRITE]);
            exit(EXIT_SUCCESS);
        }        
    }
    close(fd[P_WRITE]);
    int *larger = (int *)malloc(45000 * sizeof(int));

    char bufferP[32];
    ssize_t n;
    int i = 0;
    while ((n = read(fd[P_READ], bufferP, sizeof(bufferP))) > 0) {
        if (i < 45000){
            larger[i] = atoi(bufferP);
            i++;
        } else {
            larger = (int *)realloc(larger, 50000 * sizeof(int));
            larger[i] = atoi(bufferP);
            i++;
        }
    }
    close(fd[P_READ]);
    for (int z = 0 ; z < PROCESSES; z++){
        wait(NULL);
    }

    for (int z = 0; z < i; z++){
        printf("%d\n", larger[z]);
    }
    free(larger);
    exit(EXIT_SUCCESS);
}