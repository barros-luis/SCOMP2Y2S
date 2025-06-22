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
    printf("Please enter the name of ur file:\n");
    char buffer[256];
    fgets(buffer,sizeof(buffer),stdin);

    buffer[strcspn(buffer,"\n")] = '\0';

    char *file = buffer;

    execlp("cp","cp",file,"backup/",NULL); 

    printf("if this prints it went south\n"); 
    
    exit(EXIT_SUCCESS);
}