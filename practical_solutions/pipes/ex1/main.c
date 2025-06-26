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

#define P_READ 0
#define P_WRITE 1

int main() {
    int fd[2];
    pipe(fd);
    pid_t pid = fork();
    char buffer[256];

    if (pid == 0) {
        // Child process
        close(fd[P_WRITE]); // Close unused write end
        read(fd[P_READ], buffer, sizeof(buffer));
        printf("This is the parent pid %d !\n", atoi(buffer));
        close(fd[P_READ]);
        exit(EXIT_SUCCESS);
    }

    // Parent process
    close(fd[P_READ]); // Close unused read end
    pid_t pPid = getpid();
    printf("(Parent) this is my pid : %d!\n", pPid);

    char wBuffer[32];
    snprintf(wBuffer, sizeof(wBuffer), "%d", pPid);

    write(fd[P_WRITE], wBuffer, strlen(wBuffer) + 1); // +1 for null terminator
    close(fd[P_WRITE]);
    
    wait(NULL); // Wait for child to finish
    exit(EXIT_SUCCESS);
}