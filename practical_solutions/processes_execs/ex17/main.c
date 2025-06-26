#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

// Simplified version of system() function
int my_system(const char *command) {
    if (command == NULL) {
        return 1;  // system() returns 1 if command is NULL
    }
    
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("Fork failed");
        return -1;
    }
    
    if (pid == 0) {
        // Child process - execute the command
        execl("/bin/sh", "sh", "-c", command, NULL);
        
        // If execl returns, it means it failed
        perror("execl failed");
        exit(EXIT_FAILURE);
    } else {
        // Parent process - wait for child to finish
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        } else {
            return -1;  // Child didn't exit normally
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <command>\n", argv[0]);
        printf("Example: %s \"ls -l\"\n", argv[0]);
        return 1;
    }
    
    printf("Executing command: %s\n", argv[1]);
    
    int result = my_system(argv[1]);
    
    printf("Command finished with exit code: %d\n", result);
    
    return 0;
} 