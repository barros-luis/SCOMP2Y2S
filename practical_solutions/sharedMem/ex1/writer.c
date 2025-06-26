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


#define P_READ 0
#define P_WRITE 1

typedef struct{
    int number;
    char name[25];
    char address[100];
    int data_ready;  // Flag to indicate if data is ready
} shared_student;

int main() {
    shared_student *student_info;

    int fd = shm_open("/writer", O_CREAT | O_EXCL| O_RDWR, S_IRUSR|S_IWUSR);
    ftruncate(fd,sizeof(shared_student));
    student_info = mmap(NULL,sizeof(shared_student), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

    printf("Please enter the students name :\n");
    char buffer[32];
    fgets(buffer,sizeof(buffer), stdin);
    buffer[strcspn(buffer,"\n")] = '\0';
    strcpy(student_info->name, buffer);

    printf("Please enter the students number:\n");
    char buffer_num[32];
    fgets(buffer_num,sizeof(buffer_num), stdin);
    buffer_num[strcspn(buffer_num,"\n")] = '\0';
    student_info->number = atoi(buffer_num);

    printf("Please enter the students address :\n");
    char buffer_3[256];
    fgets(buffer_3,sizeof(buffer_3), stdin);
    buffer_3[strcspn(buffer_3,"\n")] = '\0';
    strcpy(student_info->address, buffer_3);

    // Signal that data is ready for reading
    student_info->data_ready = 1;
    printf("Data written to shared memory. Reader can now read.\n");

    exit(EXIT_SUCCESS);
}