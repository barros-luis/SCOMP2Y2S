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

#define P_READ 0
#define P_WRITE 1
#define N 5

typedef struct{
    char str[100];
    ssize_t bytes_written;
    int current_child;  // Which child should write next
} Message;

int main() {
    Message *msg;
    pid_t pids[N];

    int fd = shm_open("/message", O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
    ftruncate(fd, sizeof(Message));
    msg = mmap(NULL, sizeof(Message), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    msg->bytes_written = 0;
    msg->current_child = 0;
    memset(msg->str, 0, sizeof(msg->str));

    for (int i = 0; i < N; i++){
        pid_t pid = fork();

        if (pid == 0){
            // Child process
            while (msg->current_child != i) {
                usleep(10000); // Small delay to prevent busy waiting
            }
            
            printf("(Child %d) please type the next word:\n", i);
            char buffer[64];
            fgets(buffer, sizeof(buffer), stdin);
            buffer[strcspn(buffer, "\n")] = '\0';

            // Append the word to the message
            strcpy(msg->str + msg->bytes_written, buffer);
            msg->bytes_written += strlen(buffer);
            
            // Add space if not the last word
            if (i < N - 1) {
                strcpy(msg->str + msg->bytes_written, " ");
                msg->bytes_written += 1;
            }
            
            msg->current_child = i + 1;  // Next child's turn
            exit(EXIT_SUCCESS);
        } else {
            pids[i] = pid;
        }
    }
    
    printf("(Parent) waiting for children to complete...\n");
    int status;

    for (int i = 0; i < N; i++){
        waitpid(pids[i], &status, 0);
    }

    printf("(Parent) children terminated! Final message: \n");
    printf("%s\n", msg->str);

    munmap(msg, sizeof(Message));
    close(fd);
    shm_unlink("/message");

    exit(EXIT_SUCCESS);
}