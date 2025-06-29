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

// Global variables for results
int highest_balance;
int lowest_balance;
int average_balance;


void* highest_balance_thread(void *arg){
    int *database = (int*)arg;
    int local_highest = database[0];

    for (int i = 1; i < 1000; i++){
        if (database[i] > local_highest){
            local_highest = database[i];
        }
    }

    highest_balance = local_highest;

    pthread_exit((void*)NULL);
}

void* lowest_balance_thread(void *arg){
    int *database = (int*)arg;
    int local_lowest = database[0]; 

    for (int i = 1; i < 1000; i++){
        if (database[i] < local_lowest){
            local_lowest = database[i];
        }
    }

    lowest_balance = local_lowest;

    pthread_exit((void*)NULL);
}

void* average_balance_thread(void *arg){
    int *database = (int*)arg;
    long long local_sum = 0;  // Use long long to avoid overflow

    for (int i = 0; i < 1000; i++){
        local_sum += database[i];
    }

    int local_average = local_sum / 1000;


    average_balance = local_average;

    pthread_exit((void*)NULL);
}   

int main() {
    int database[1000];

    srand(time(NULL));

    for (int i = 0; i < 1000; i++){
        database[i] = (rand() % 20) + 1;
    }

    pthread_t threads[3];

    for (int i = 0; i < 3; i++){
        int result;
        if (i == 0){
            result = pthread_create(&threads[i], NULL, highest_balance_thread, (void*)database);
        } else if (i == 1){
            result = pthread_create(&threads[i], NULL, lowest_balance_thread, (void*)database);
        } else {
            result = pthread_create(&threads[i], NULL, average_balance_thread, (void*)database);
        }
        
        if (result != 0) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < 3; i++){
        int result = pthread_join(threads[i], NULL);
        if (result != 0) {
            perror("pthread_join");
            exit(EXIT_FAILURE);
        }
    }

    printf("Highest balance: %d\n", highest_balance);
    printf("Lowest balance: %d\n", lowest_balance);
    printf("Average balance: %d\n", average_balance);
    
    exit(EXIT_SUCCESS);
}