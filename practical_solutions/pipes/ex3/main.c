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

    if (pid == 0) {
        // Child process
        close(fd[P_WRITE]);
        char buffer[256];
        ssize_t n = read(fd[P_READ], buffer, sizeof(buffer) - 1);
        if (n > 0) {
            buffer[n] = '\0'; // Ensure null-terminated
            printf("Content of the file: (%s)\n", buffer);
        } else {
            printf("Child: No data read from pipe.\n");
        }
        close(fd[P_READ]);
        exit(EXIT_SUCCESS);
    } else {
        // Parent process
        close(fd[P_READ]);
        FILE *file = fopen("teste.txt", "r");
        if (!file) {
            perror("fopen failed");
            close(fd[P_WRITE]);
            exit(EXIT_FAILURE);
        }
        char file_content[256];
        if (fgets(file_content, sizeof(file_content), file) == NULL) {
            perror("fgets failed");
            fclose(file);
            close(fd[P_WRITE]);
            exit(EXIT_FAILURE);
        }
        fclose(file);
        write(fd[P_WRITE], file_content, strlen(file_content));
        close(fd[P_WRITE]);
        wait(NULL);
    }

    exit(EXIT_SUCCESS);
}