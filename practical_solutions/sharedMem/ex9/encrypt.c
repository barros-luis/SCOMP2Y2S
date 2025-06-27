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

typedef struct{
    char txt[50];
    int read;
    int msg_exists;
    int encrypted;
    int key;  // Store the encryption key
    int shutdown;  // NEW: Signal to terminate other programs
} Message;

int main() {
    Message *msg;

    int fd = shm_open("/ied", O_RDWR, S_IRUSR | S_IWUSR);
    msg = mmap(NULL, sizeof(Message), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    printf("(Encrypt) Waiting for messages...\n");
    
    // Run continuously until shutdown signal
    while(!msg->shutdown){
        // Wait for a new message
        while(!msg->msg_exists && !msg->shutdown){
            usleep(1000);
        }
        
        // Check if we should shutdown
        if (msg->shutdown) {
            break;
        }

        srand(time(NULL));
        
        // Generate random key between 1 and 5
        int key = (rand() % 5) + 1;
        msg->key = key;  // Store the key for decryption
        
        char txt_encrypted[50];
        int len = strlen(msg->txt);
        
        // Encrypt each character by adding the key to its ASCII value
        for (int i = 0; i < len; i++){
            txt_encrypted[i] = msg->txt[i] + key;
        }
        txt_encrypted[len] = '\0';  // Null terminate
        
        // Copy encrypted message back to shared memory
        strcpy(msg->txt, txt_encrypted);
        
        msg->encrypted = 1;
        printf("(Encrypt) Message encrypted with key %d\n", key);
        
        // Wait for this message to be processed before continuing
        while(msg->encrypted && !msg->shutdown){
            usleep(1000);
        }
    }

    printf("(Encrypt) Shutting down...\n");
    exit(EXIT_SUCCESS);
}