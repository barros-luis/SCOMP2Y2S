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

    shm_unlink("/ied");

    int fd = shm_open("/ied", O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
    ftruncate(fd, sizeof(Message));
    msg = mmap(NULL, sizeof(Message), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    // Initialize shared memory
    msg->read = 0;
    msg->msg_exists = 0;
    msg->encrypted = 0;
    msg->key = 0;
    msg->shutdown = 0;  // Initialize shutdown flag

    int wants_to_proceed = 1;  // Changed to 1 to enter the loop

    while(wants_to_proceed){
        printf("Do you want to generate one more msg?(y/n)\n");
        char bff[64];
        fgets(bff, sizeof(bff), stdin);
        bff[strcspn(bff, "\n")] = '\0';

        if (strcmp(bff, "n") == 0){  // Changed condition
            wants_to_proceed = 0;
            break;
        }
    
        printf("Yo please enter your message! :\n");

        char buffer[64];
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = '\0';
    
        strcpy(msg->txt, buffer);
        msg->msg_exists = 1;

        while(!msg->read){
            usleep(1000);
        }
        msg->read = 0;
        msg->msg_exists = 0;
        msg->encrypted = 0;
        msg->key = 0;
    }

    // Signal other programs to terminate
    msg->shutdown = 1;
    printf("Signaling other programs to terminate...\n");
    
    // Give other programs time to see the shutdown signal
    sleep(2);

    munmap(msg, sizeof(Message));
    close(fd);
    shm_unlink("/ied");

    exit(EXIT_SUCCESS);
}