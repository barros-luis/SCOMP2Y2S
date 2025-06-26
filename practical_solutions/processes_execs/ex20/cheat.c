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
    srand(time(NULL));

    int random_num = (rand() % 5) + 1;
    
    exit(random_num);
}