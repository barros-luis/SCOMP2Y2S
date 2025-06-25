#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

int main() {
    int pipe1[2], pipe2[2];
    pid_t pid1, pid2, pid3;
    
    // Create two pipes
    if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
        perror("pipe");
        exit(1);
    }
    
    // First child: ls -la
    pid1 = fork();
    if (pid1 == 0) {
        // Child 1: ls -la -> pipe1
        close(pipe1[0]);  // Close read end
        close(pipe2[0]);  // Close both ends of pipe2
        close(pipe2[1]);
        
        // Redirect stdout to pipe1 write end
        dup2(pipe1[1], STDOUT_FILENO);
        close(pipe1[1]);
        
        execlp("ls", "ls", "-la", NULL);
        perror("execlp ls");
        exit(1);
    }
    
    // Second child: sort
    pid2 = fork();
    if (pid2 == 0) {
        // Child 2: pipe1 -> sort -> pipe2
        close(pipe1[1]);  // Close write end of pipe1
        close(pipe2[0]);  // Close read end of pipe2
        
        // Redirect stdin from pipe1 read end
        dup2(pipe1[0], STDIN_FILENO);
        close(pipe1[0]);
        
        // Redirect stdout to pipe2 write end
        dup2(pipe2[1], STDOUT_FILENO);
        close(pipe2[1]);
        
        execlp("sort", "sort", NULL);
        perror("execlp sort");
        exit(1);
    }
    
    // Parent: wc -l
    close(pipe1[0]);  // Close both ends of pipe1
    close(pipe1[1]);
    close(pipe2[1]);  // Close write end of pipe2
    
    // Redirect stdin from pipe2 read end
    dup2(pipe2[0], STDIN_FILENO);
    close(pipe2[0]);
    
    execlp("wc", "wc", "-l", NULL);
    perror("execlp wc");
    exit(1);
    
    return 0;
} 