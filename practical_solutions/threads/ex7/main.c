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

int chips = 0;
int drinks = 0;
int t1_ready = 0;  // T1 has bought chips
int t2_ready = 0;  // T2 has bought beer

pthread_mutex_t mutex;
pthread_cond_t both_ready;  // Condition variable to signal when both threads are ready

void* thread1_function(void* arg){
    // T1: buy chips, then wait for T2 to buy beer, then eat and drink
    pthread_mutex_lock(&mutex);
    chips++;
    t1_ready = 1;
    write(STDOUT_FILENO, "T1: Chips bought\n", 17);
    fflush(stdout);
    
    // Signal that T1 is ready
    if (t1_ready && t2_ready) {
        pthread_cond_broadcast(&both_ready);
    }
    pthread_mutex_unlock(&mutex);
    
    // Wait for both threads to be ready
    pthread_mutex_lock(&mutex);
    while (!t1_ready || !t2_ready) {
        pthread_cond_wait(&both_ready, &mutex);
    }
    
    // Eat and drink
    chips--;
    drinks--;
    write(STDOUT_FILENO, "T1: Eating and drinking\n", 24);
    fflush(stdout);
    pthread_mutex_unlock(&mutex);
    
    pthread_exit((void*)NULL);
}

void* thread2_function(void* arg){
    // T2: buy beer, then wait for T1 to buy chips, then eat and drink
    pthread_mutex_lock(&mutex);
    drinks++;
    t2_ready = 1;
    write(STDOUT_FILENO, "T2: Beer bought\n", 16);
    fflush(stdout);
    
    // Signal that T2 is ready
    if (t1_ready && t2_ready) {
        pthread_cond_broadcast(&both_ready);
    }
    pthread_mutex_unlock(&mutex);
    
    // Wait for both threads to be ready
    pthread_mutex_lock(&mutex);
    while (!t1_ready || !t2_ready) {
        pthread_cond_wait(&both_ready, &mutex);
    }
    
    // Eat and drink
    chips--;
    drinks--;
    write(STDOUT_FILENO, "T2: Eating and drinking\n", 24);
    fflush(stdout);
    pthread_mutex_unlock(&mutex);
    
    pthread_exit((void*)NULL);
}

int main() {
    pthread_t threads[2];
    
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&both_ready, NULL);

    // Create exactly 2 threads as specified
    pthread_create(&threads[0], NULL, thread1_function, NULL);  // T1
    pthread_create(&threads[1], NULL, thread2_function, NULL);  // T2

    // Wait for both threads to finish
    for (int i = 0; i < 2; i++){
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&both_ready);
    
    exit(EXIT_SUCCESS);
}