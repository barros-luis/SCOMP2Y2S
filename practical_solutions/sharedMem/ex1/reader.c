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
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

typedef struct{
    int number;
    char name[25];
    char address[100];
    int data_ready;  // Flag to indicate if data is ready
} shared_student;

int main() {
    shared_student *student_info;

    int fd = shm_open("/writer", O_RDONLY, S_IRUSR);
    student_info = mmap(NULL, sizeof(shared_student), PROT_READ, MAP_SHARED, fd, 0);

    printf("Waiting for data to be written...\n");
    
    // Busy waiting - continuously check if data is ready
    while (student_info->data_ready == 0) {
        // Small delay to prevent excessive CPU usage
        usleep(100000); // 100ms delay
    }

    printf("Data received! Student Information:\n");
    printf("Name: %s\n", student_info->name);
    printf("Number: %d\n", student_info->number);
    printf("Address: %s\n", student_info->address);

    // Clean up
    munmap(student_info, sizeof(shared_student));
    close(fd);
    shm_unlink("/writer");

    exit(EXIT_SUCCESS);
} 