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
int threads_eaten = 0;
int total_threads = 6;

pthread_mutex_t mutex;
pthread_cond_t resources_available;  // Condition variable for chips and drinks

void* thread_function(void* arg){
    int thread_id = *(int*)arg;
    int has_eaten = 0;
    
    while (threads_eaten < total_threads) {
        // Randomly buy chips or beer
        int buy_choice = rand() % 2;  // 0 = chips, 1 = beer
        
        pthread_mutex_lock(&mutex);
        if (buy_choice == 0) {
            chips++;
            printf("Thread %d: Chips bought (total chips: %d, drinks: %d)\n", 
                   thread_id, chips, drinks);
            fflush(stdout);
        } else {
            drinks++;
            printf("Thread %d: Beer bought (total chips: %d, drinks: %d)\n", 
                   thread_id, chips, drinks);
            fflush(stdout);
        }
        
        // Try to eat and drink if resources are available and haven't eaten yet
        if (chips > 0 && drinks > 0 && !has_eaten) {
            chips--;
            drinks--;
            has_eaten = 1;
            threads_eaten++;
            printf("Thread %d: Eating and drinking! (remaining: %d chips, %d drinks, eaten: %d/%d)\n", 
                   thread_id, chips, drinks, threads_eaten, total_threads);
            fflush(stdout);
        }
        
        // Signal other threads that resources might be available
        if (chips > 0 && drinks > 0) {
            pthread_cond_broadcast(&resources_available);
        }
        
        pthread_mutex_unlock(&mutex);
        
        // If this thread hasn't eaten yet, wait for resources
        if (!has_eaten) {
            pthread_mutex_lock(&mutex);
            while ((chips == 0 || drinks == 0) && threads_eaten < total_threads) {
                pthread_cond_wait(&resources_available, &mutex);
            }
            pthread_mutex_unlock(&mutex);
        }
        
        // Small delay to prevent busy waiting
        usleep(100000);  // 100ms
    }
    
    printf("Thread %d: Finished! (eaten: %d/%d)\n", thread_id, threads_eaten, total_threads);
    fflush(stdout);
    
    pthread_exit((void*)NULL);
}

int main() {
    pthread_t threads[6];
    int thread_ids[6];
    
    // Initialize random seed
    srand(time(NULL));
    
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&resources_available, NULL);

    printf("Starting 6 threads that will continuously buy chips/beer until all have eaten...\n");
    fflush(stdout);

    // Create 6 threads
    for (int i = 0; i < 6; i++){
        thread_ids[i] = i + 1;  // Thread IDs: 1, 2, 3, 4, 5, 6
        int result = pthread_create(&threads[i], NULL, thread_function, &thread_ids[i]);
        if (result != 0) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    }

    // Wait for all threads to finish
    for (int i = 0; i < 6; i++){
        int result = pthread_join(threads[i], NULL);
        if (result != 0) {
            perror("pthread_join");
            exit(EXIT_FAILURE);
        }
    }

    printf("\n=== FINAL SUMMARY ===\n");
    printf("Final chips remaining: %d\n", chips);
    printf("Final drinks remaining: %d\n", drinks);
    printf("Total threads that ate: %d/%d\n", threads_eaten, total_threads);
    printf("All threads completed!\n");
    fflush(stdout);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&resources_available);
    
    exit(EXIT_SUCCESS);
}
