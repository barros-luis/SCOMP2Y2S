#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <time.h>

int main() {
    int current_money = 25;
    while(current_money > 0){
        printf("Please select a number between 1 and 5:\n");
        char buffer[256];
        fgets(buffer,sizeof(buffer),stdin);
        buffer[strcspn(buffer,"\n")] = '\0';
       
        int number = atoi(buffer);

        printf("Please select how much u want to bet - CURRENT %d\n", current_money);
        char buffer_2[256];
        fgets(buffer_2,sizeof(buffer_2),stdin);
        buffer_2[strcspn(buffer_2,"\n")] = '\0';
       
        int bet = atoi(buffer_2);

        if (bet > current_money) {
            printf("Bet should be less or equal to ur balance\n");
            continue;
        }

        pid_t pid = fork();
        
        if (pid < 0) {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        }

        if (pid == 0) {
            execlp("./cheat","cheat",NULL);
            perror("execlp failed");
            exit(EXIT_FAILURE);
        }

        int status;
        waitpid(pid,&status,0);

        if (WIFEXITED(status)) {
            int random_num = WEXITSTATUS(status);
            printf("Random number was: %d\n", random_num);
            
            if (random_num == number){
                current_money = current_money + bet;
                printf("You won! New balance: %d\n", current_money);
            } else {
                current_money = current_money - bet;
                printf("You lost! New balance: %d\n", current_money);
            }
        }
    }

    printf("Game over! You're out of money.\n");
    exit(EXIT_SUCCESS);
}