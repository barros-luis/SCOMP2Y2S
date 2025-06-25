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
        perror("fork failed");
        exit(EXIT_FAILURE);
    }

    if (pid == 0){
        close(fd[P_WRITE]);
        char buffer_R[256];

        ssize_t bytes_read;
        while((bytes_read = read(fd[P_READ],buffer_R,sizeof(buffer_R))) > 0){
            buffer_R[bytes_read] = '\0';
            printf("Current directory name is %s\n",buffer_R);
        }

        exit(EXIT_SUCCESS);

    }
    
    close(fd[P_READ]);
    printf("Please enter the system directory for listing content :\n");
    char buffer[256];
    fgets(buffer,sizeof(buffer), stdin);
    buffer[strcspn(buffer,"\n")] = '\0';

    dup2(fd[P_WRITE],1);
    close(fd[P_WRITE]);

    execlp("ls","ls",buffer,NULL);

    
    exit(EXIT_SUCCESS);
}