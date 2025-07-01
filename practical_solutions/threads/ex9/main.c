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

int data[1000];
int result[1000];
int threads_done = 0;

pthread_mutex_t mutex;
pthread_mutex_t mutex_result;
pthread_mutex_t mutex_print;
pthread_cond_t cond;

void* perform_calculations(void* arg){
    int idx = *((int*)arg);
    int start = idx * 200;
    int finish = start + 200;

    for (int i = start; i < finish; i++){
        result[i] = (data[i] * 10) + 2;
    }

    pthread_mutex_lock(&mutex);
    threads_done++;
    if (threads_done == 5){
        pthread_cond_broadcast(&cond);
    }
    pthread_mutex_unlock(&mutex);

    pthread_mutex_lock(&mutex_result);
    while(threads_done < 5)
        pthread_cond_wait(&cond, &mutex_result);

    pthread_mutex_unlock(&mutex_result);

    for (int i = start; i < finish; i++){
        pthread_mutex_lock(&mutex_print);
        printf("%d\n", result[i]);
        pthread_mutex_unlock(&mutex_print);
    }

    pthread_exit((void*)NULL);
}

int main() {
    pthread_t threads[5];

    pthread_mutex_init(&mutex,NULL);
    pthread_mutex_init(&mutex_result,NULL);
    pthread_mutex_init(&mutex_print,NULL);
    pthread_cond_init(&cond, NULL);

    srand(time(NULL));

    for (int i = 0; i < 1000; i++){
        data[i] = (rand() % 5) + 1;
    }

    int indices[5];
    for (int i = 0; i < 5; i++){
        indices[i] = i;
        pthread_create(&threads[i],NULL,perform_calculations,(void*)&indices[i]);
    }
    
    for (int i = 0; i < 5; i++){
        pthread_join(threads[i],NULL);
    }

    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&mutex_result);
    pthread_mutex_destroy(&mutex_print);
    pthread_cond_destroy(&cond);
    
    exit(EXIT_SUCCESS);
}