#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

#define P_READ 0
#define P_WRITE 1

int main() {
    int fd[2];
    if (pipe(fd) == -1) {
        perror("pipe failed");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        // --- Child Process: Reads from pipe and filters ---

        // Close the write end, we only read from the pipe
        close(fd[P_WRITE]);

        printf("\n--- Directories Found by Child ---\n");

        // Use fdopen to get a FILE* stream for easy line-by-line reading
        FILE *pipe_stream = fdopen(fd[P_READ], "r");
        if (pipe_stream == NULL) {
            perror("fdopen failed");
            exit(EXIT_FAILURE);
        }

        char line_buffer[512];
        // Read each line from the 'ls -l' output sent by the parent
        while (fgets(line_buffer, sizeof(line_buffer), pipe_stream) != NULL) {
            // Use sscanf for more robust parsing of the 'ls -l' output
            char permissions[11];
            char filename[256];

            // This format string skips the link count, user, group, size, and date fields.
            // It reads the permissions and the filename.
            if (sscanf(line_buffer, "%10s %*d %*s %*s %*s %*s %*s %*s %255[^\n]", permissions, filename) == 2) {
                // Check if the first character of permissions is 'd'
                if (permissions[0] == 'd') {
                    printf("%s\n", filename);
                }
            }
        }

        fclose(pipe_stream); // This also closes fd[P_READ]
        printf("----------------------------------\n");
        printf("Child finished.\n");
        exit(EXIT_SUCCESS);
    }

    // --- Parent Process: Executes 'ls -l' and sends output to child ---

    // 1. Prompt user for directory
    printf("Please enter the system directory for listing content:\n");
    char user_dir[256];
    fgets(user_dir, sizeof(user_dir), stdin);
    user_dir[strcspn(user_dir, "\n")] = '\0'; // Remove newline

    close(fd[P_READ]);
    dup2(fd[P_WRITE], 1);
    close(fd[P_WRITE]);

    execlp("ls", "ls", "-l", user_dir, NULL);

    perror("execlp failed");
    exit(EXIT_FAILURE);
} 