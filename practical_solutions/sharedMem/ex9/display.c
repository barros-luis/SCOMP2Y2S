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

    printf("(Display) Waiting for encrypted messages...\n");
    
    // Run continuously until shutdown signal
    while(!msg->shutdown){
        // Wait for an encrypted message
        while((!msg->msg_exists || !msg->encrypted) && !msg->shutdown){
            usleep(1000);
        }
        
        // Check if we should shutdown
        if (msg->shutdown) {
            break;
        }

        // Decrypt the message using the stored key
        char decrypted[50];
        int len = strlen(msg->txt);
        
        for (int i = 0; i < len; i++){
            decrypted[i] = msg->txt[i] - msg->key;  // Subtract the key to decrypt
        }
        decrypted[len] = '\0';
        
        printf("(Display) Decrypted message: %s\n", decrypted);
        
        // Mark as read and clear the message
        msg->read = 1;
        msg->txt[0] = '\0';
        msg->encrypted = 0;
        msg->key = 0;
    }

    printf("(Display) Shutting down...\n");
    exit(EXIT_SUCCESS);
}