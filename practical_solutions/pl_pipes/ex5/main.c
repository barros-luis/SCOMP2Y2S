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

    int vec1[1000];
    int vec2[1000];
    srand(time(NULL));

    for (int i = 0; i < 1000;i++){
        vec1[i] = (rand() % 6) + 5;
        vec2[i] = (rand() % 5) + 1; 
    }

    for (int i= 0; i < 5; i++){
        pid_t pid = fork();
        int partial_sum = 0;
        int start = i*200;
        int finish = start + 200;
        if (pid < 0){
            perror("fork failed");
            close(fd[P_READ]);
            close(fd[P_WRITE]);
            exit(EXIT_FAILURE);
        }
        
        if (pid == 0){
            close(fd[P_READ]);
            for (int j = start; j < finish; j++){
                partial_sum += vec1[j] + vec2[j];
            }
            char buffer[32];
            snprintf(buffer,sizeof(buffer),"%d",partial_sum);
            write(fd[P_WRITE],buffer,sizeof(buffer));
            close(fd[P_WRITE]);
            exit(EXIT_SUCCESS);
        }
    }

    close(fd[P_WRITE]);
    int result = 0;
    for (int i = 0; i < 5; i++){
        char buffer2[32];
        read(fd[P_READ],buffer2,sizeof(buffer2));
        int current_sum = atoi(buffer2);
        result += current_sum;
    }


    for (int i = 0; i < 5; i++)
        wait(NULL);


    printf("THE RESULT IS : %d\n",result);
    close(fd[P_READ]);
    exit(EXIT_SUCCESS);
}