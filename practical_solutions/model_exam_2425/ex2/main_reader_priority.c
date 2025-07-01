#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

// --- Configuration ---
#define NUM_WRITERS 3
#define NUM_READERS 10
#define WRITE_ITERATIONS 2
#define READ_ITERATIONS 3

// --- Shared Data Structure ---
typedef struct {
    char string1[50];
    char string2[50];
} SharedData;

// --- Global Shared Variables ---
SharedData* shared_resource;
int reader_count = 0;
int writer_count = 0;
int waiting_writers = 0;  // Writers waiting to write

// --- Synchronization Primitives ---
pthread_mutex_t mutex;
pthread_cond_t reader_cond;
pthread_cond_t writer_cond;
pthread_mutex_t print_mutex;

// --- Writer Thread Logic ---
void* writer(void* arg) {
    long id = (long)arg;

    for (int i = 0; i < WRITE_ITERATIONS; i++) {
        // --- Entry Section ---
        pthread_mutex_lock(&mutex);
        waiting_writers++;  // Announce we want to write
        
        // Wait until no readers are active (reader priority)
        while (reader_count > 0 || writer_count > 0) {
            pthread_cond_wait(&writer_cond, &mutex);
        }
        
        waiting_writers--;  // No longer waiting
        writer_count++;     // Now we're writing
        pthread_mutex_unlock(&mutex);

        // --- Critical Section ---
        time_t now = time(NULL);
        struct tm* t = localtime(&now);
        
        snprintf(shared_resource->string1, sizeof(shared_resource->string1), "Writer ID: %ld", id);
        strftime(shared_resource->string2, sizeof(shared_resource->string2), "%Y-%m-%d %H:%M:%S", t);
        
        pthread_mutex_lock(&print_mutex);
        printf("--- Writer %ld wrote. ---\n", id);
        printf("    Content: %s\n\n", shared_resource->string2);
        pthread_mutex_unlock(&print_mutex);

        // --- Exit Section ---
        pthread_mutex_lock(&mutex);
        writer_count--;
        
        // Signal waiting readers first (priority), then writers
        pthread_cond_broadcast(&reader_cond);
        if (waiting_writers > 0) {
            pthread_cond_signal(&writer_cond);
        }
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

// --- Reader Thread Logic ---
void* reader(void* arg) {
    long id = (long)arg;

    for (int i = 0; i < READ_ITERATIONS; i++) {
        // --- Entry Section ---
        pthread_mutex_lock(&mutex);
        
        // Wait only if a writer is currently writing
        while (writer_count > 0) {
            pthread_cond_wait(&reader_cond, &mutex);
        }
        
        reader_count++;
        pthread_mutex_unlock(&mutex);

        // --- Critical Section ---
        char local_str1[50];
        char local_str2[50];
        int current_readers;

        strcpy(local_str1, shared_resource->string1);
        strcpy(local_str2, shared_resource->string2);

        pthread_mutex_lock(&mutex);
        current_readers = reader_count;
        pthread_mutex_unlock(&mutex);

        pthread_mutex_lock(&print_mutex);
        printf("Reader %ld read:\n", id);
        printf("  -> \"%s\"\n", local_str1);
        printf("  -> \"%s\"\n", local_str2);
        printf("  (Active Readers: %d, Waiting Writers: %d)\n\n", current_readers, waiting_writers);
        pthread_mutex_unlock(&print_mutex);
        
        // --- Exit Section ---
        pthread_mutex_lock(&mutex);
        reader_count--;
        
        // If this was the last reader and writers are waiting, signal them
        if (reader_count == 0 && waiting_writers > 0) {
            pthread_cond_signal(&writer_cond);
        }
        pthread_mutex_unlock(&mutex);

        sleep(1);
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
    snprintf(shared_resource->string1, sizeof(shared_resource->string1), "Initial Data");
    snprintf(shared_resource->string2, sizeof(shared_resource->string2), "No timestamp yet");

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&reader_cond, NULL);
    pthread_cond_init(&writer_cond, NULL);
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

    // 3. Wait for Threads to Complete
    for (int i = 0; i < NUM_WRITERS; i++) {
        pthread_join(writer_threads[i], NULL);
    }
    for (int i = 0; i < NUM_READERS; i++) {
        pthread_join(reader_threads[i], NULL);
    }

    // 4. Cleanup
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&reader_cond);
    pthread_cond_destroy(&writer_cond);
    pthread_mutex_destroy(&print_mutex);
    free(shared_resource);

    printf("All threads have finished.\n");

    return 0;
} 