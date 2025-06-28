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
#define BUFFER_SIZE 10
#define TOTAL_VALUES 30

typedef struct{
    int arr[BUFFER_SIZE];
    int head;      // Where to write next
    int tail;      // Where to read next
    int count;     // Number of items in buffer
} CircularBuffer;

int main() {
    CircularBuffer *buffer;
    int fd = shm_open("/shm_buffer", O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
    ftruncate(fd, sizeof(CircularBuffer));
    buffer = mmap(NULL, sizeof(CircularBuffer), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    // Initialize buffer
    buffer->head = 0;
    buffer->tail = 0;
    buffer->count = 0;

    // Semaphores for synchronization
    sem_t *mutex;      // Mutual exclusion for buffer access
    sem_t *full;       // Number of full slots
    sem_t *empty;      // Number of empty slots
    
    mutex = sem_open("/sem_mutex", O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR, 1);
    full = sem_open("/sem_full", O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR, 0);
    empty = sem_open("/sem_empty", O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR, BUFFER_SIZE);

    // Create 2 producer processes
    for (int i = 0; i < 2; i++){
        pid_t pid = fork();

        if (pid == 0){
            // Producer process
            int value = i * 100;  // Producer 0 starts at 0, Producer 1 starts at 100
            
            for (int j = 0; j < TOTAL_VALUES/2; j++) {  // Each producer produces 15 values
                sem_wait(empty);   // Wait for empty slot
                sem_wait(mutex);   // Enter critical section
                
                // Add value to buffer
                buffer->arr[buffer->head] = value++;
                buffer->head = (buffer->head + 1) % BUFFER_SIZE;
                buffer->count++;
                
                printf("Producer %d: produced %d\n", i, value-1);
                fflush(stdout);
                
                sem_post(mutex);   // Exit critical section
                sem_post(full);    // Signal that a slot is full
            }
            exit(EXIT_SUCCESS);
        }
    }

    // Parent process as consumer
    printf("Consumer: Starting to consume values...\n");
    fflush(stdout);
    
    for (int i = 0; i < TOTAL_VALUES; i++) {
        sem_wait(full);    // Wait for full slot
        sem_wait(mutex);   // Enter critical section
        
        // Remove value from buffer
        int value = buffer->arr[buffer->tail];
        buffer->tail = (buffer->tail + 1) % BUFFER_SIZE;
        buffer->count--;
        
        printf("Consumer: consumed %d (buffer count: %d)\n", value, buffer->count);
        fflush(stdout);
        
        sem_post(mutex);   // Exit critical section
        sem_post(empty);   // Signal that a slot is empty
    }

    // Wait for children to finish
    for (int i = 0; i < 2; i++){
        wait(NULL);
    }

    printf("Consumer: All values consumed!\n");
    fflush(stdout);

    // Clean up
    sem_close(mutex);
    sem_close(full);
    sem_close(empty);
    sem_unlink("/sem_mutex");
    sem_unlink("/sem_full");
    sem_unlink("/sem_empty");
    munmap(buffer, sizeof(CircularBuffer));
    close(fd);
    shm_unlink("/shm_buffer");
    
    exit(EXIT_SUCCESS);
}