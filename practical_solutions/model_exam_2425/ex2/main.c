#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

// --- Configuration ---
#define NUM_WRITERS 3
#define NUM_READERS 10
#define WRITE_ITERATIONS 2 // How many times each writer will write
#define READ_ITERATIONS 3  // How many times each reader will read

// --- Shared Data Structure ---
// This is the data that will be protected.
typedef struct {
    char string1[50];
    char string2[50];
} SharedData;

// --- Global Shared Variables ---
SharedData* shared_resource; // The actual shared data, allocated on the heap.
int reader_count = 0;        // Counts how many readers are currently active.

// --- Synchronization Primitives ---
pthread_mutex_t rw_mutex;     // Main lock: Writers must acquire this. First reader acquires it, last reader releases it.
pthread_mutex_t count_mutex;  // Secondary lock: Protects access to the 'reader_count' variable.
pthread_mutex_t print_mutex;  // Tertiary lock: Ensures clean, non-interleaved terminal output.

// --- Writer Thread Logic ---
void* writer(void* arg) {
    long id = (long)arg;

    for (int i = 0; i < WRITE_ITERATIONS; i++) {
        // --- Entry Section ---
        pthread_mutex_lock(&rw_mutex); // A writer must acquire the main lock to enter.
                                       // It will block here if any readers are active or another writer is active.

        // --- Critical Section ---
        time_t now = time(NULL);
        struct tm* t = localtime(&now);
        
        sprintf(shared_resource->string1, "Writer ID: %ld", id);
        // Use strftime for a clean, newline-free timestamp.
        strftime(shared_resource->string2, 50, "%Y-%m-%d %H:%M:%S", t);
        
        // Lock the print mutex to ensure the output block is atomic.
        pthread_mutex_lock(&print_mutex);
        printf("--- Writer %ld wrote. ---\n", id);
        printf("    Content: %s\n\n", shared_resource->string2);
        pthread_mutex_unlock(&print_mutex);

        // --- Exit Section ---
        pthread_mutex_unlock(&rw_mutex); // Release the lock, allowing other writers or readers to proceed.
    }

    return NULL;
}

// --- Reader Thread Logic ---
void* reader(void* arg) {
    long id = (long)arg;

    for (int i = 0; i < READ_ITERATIONS; i++) {
        // --- Entry Section ---
        pthread_mutex_lock(&count_mutex); // Lock to safely modify the reader_count
        reader_count++;
        if (reader_count == 1) {
            // If this is the *first* reader, it must acquire the main lock.
            // This effectively blocks any waiting writers.
            pthread_mutex_lock(&rw_mutex);
        }
        pthread_mutex_unlock(&count_mutex); // Unlock the counter mutex.

        // --- Critical Section ---
        // To prevent messy output, we copy the data to local variables first
        // and then print it inside a dedicated print lock.
        char local_str1[50];
        char local_str2[50];
        int current_readers;

        strcpy(local_str1, shared_resource->string1);
        strcpy(local_str2, shared_resource->string2);

        // We lock count_mutex only to get a consistent value for printing.
        pthread_mutex_lock(&count_mutex);
        current_readers = reader_count;
        pthread_mutex_unlock(&count_mutex);

        // Now, lock the print mutex to ensure the entire block prints without interruption.
        pthread_mutex_lock(&print_mutex);
        printf("Reader %ld read:\n", id);
        printf("  -> \"%s\"\n", local_str1);
        printf("  -> \"%s\"\n", local_str2);
        printf("  (Active Readers now: %d)\n\n", current_readers);
        pthread_mutex_unlock(&print_mutex);
        
        // --- Exit Section ---
        pthread_mutex_lock(&count_mutex); // Lock to safely modify the reader_count
        reader_count--;
        if (reader_count == 0) {
            // If this is the *last* reader leaving, it must release the main lock.
            // This signals to any waiting writers that they can now proceed.
            pthread_mutex_unlock(&rw_mutex);
        }
        pthread_mutex_unlock(&count_mutex);

        sleep(1); // Simulate doing other work
    }

    return NULL;
}


// --- Main Program ---
int main() {
    // 1. Initialization
    shared_resource = (SharedData*)malloc(sizeof(SharedData));
    if (shared_resource == NULL) {
        perror("Failed to allocate shared memory");
        return 1;
    }
    sprintf(shared_resource->string1, "Initial Data");
    sprintf(shared_resource->string2, "No timestamp yet");

    pthread_mutex_init(&rw_mutex, NULL);
    pthread_mutex_init(&count_mutex, NULL);
    pthread_mutex_init(&print_mutex, NULL);

    pthread_t writer_threads[NUM_WRITERS];
    pthread_t reader_threads[NUM_READERS];

    // 2. Create Threads
    for (long i = 0; i < NUM_WRITERS; i++) {
        pthread_create(&writer_threads[i], NULL, writer, (void*)i);
    }
    for (long i = 0; i < NUM_READERS; i++) {
        pthread_create(&reader_threads[i], NULL, reader, (void*)i);
    }

    // 3. Wait for Threads to Complete (Join)
    for (int i = 0; i < NUM_WRITERS; i++) {
        pthread_join(writer_threads[i], NULL);
    }
    for (int i = 0; i < NUM_READERS; i++) {
        pthread_join(reader_threads[i], NULL);
    }

    // 4. Cleanup
    pthread_mutex_destroy(&rw_mutex);
    pthread_mutex_destroy(&count_mutex);
    pthread_mutex_destroy(&print_mutex);
    free(shared_resource);

    printf("All threads have finished.\n");

    return 0;
}
