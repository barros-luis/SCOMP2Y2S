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

    if (pid < 0){
        perror("fork failed\n");
        close(fd[P_READ]);
        close(fd[P_WRITE]);
        exit(EXIT_FAILURE);
    }

    if (pid == 0){
        printf("(Child) gonna exec sort fx.txt !\n");

        close(fd[P_READ]);
        dup2(fd[P_WRITE],1);
        close(fd[P_WRITE]);
        execlp("sort","sort","fx.txt",NULL);

        perror("exec error");
        exit(EXIT_SUCCESS);
    }

    printf("(Parent) lets see the output!\n");
    close(fd[P_WRITE]);

    ssize_t bytes_read;
    char buffer[256];
    while ((bytes_read = read(fd[P_READ],buffer,sizeof(buffer) - 1)) > 0){
        buffer[bytes_read] = '\0';
        printf("%s",buffer);
    }

    close(fd[P_READ]);
    
    wait(NULL);

    exit(EXIT_SUCCESS);
}