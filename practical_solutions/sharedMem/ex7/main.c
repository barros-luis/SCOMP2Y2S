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
#define STR_SIZE 50
#define NR_COURSES 10

typedef struct{
    int number;
    char name[STR_SIZE];
    int grade[NR_COURSES];
    int data_present;
    int child_to_read;
}student_t;

int main() {
    student_t *student;

    // Clean up any existing shared memory segment first
    shm_unlink("/students");

    int fd = shm_open("/students", O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }
    
    ftruncate(fd, sizeof(student_t));
    student = mmap(NULL, sizeof(student_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (student == MAP_FAILED) {
        perror("mmap");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // Initialize shared memory
    student->child_to_read = 0;
    student->data_present = 0;
    student->number = 0;
    memset(student->name, 0, STR_SIZE);
    memset(student->grade, 0, NR_COURSES * sizeof(int));

    for (int i = 0; i < 2; i++){
        pid_t pid = fork();

        if (pid == 0){
            // Child process
            if (i == 0){
                // Child 0: Calculate highest and lowest grades
                printf("(Child %d) Waiting for data...\n", i);
                
                // Wait for data to be present AND it's my turn
                while (!student->data_present || student->child_to_read != i) {
                    usleep(10000); // Small delay to prevent busy waiting
                }
                
                int highest = student->grade[0];
                int lowest = student->grade[0];
                
                for (int j = 0; j < NR_COURSES; j++){
                    if (student->grade[j] > highest){
                        highest = student->grade[j];
                    }
                    if (student->grade[j] < lowest){
                        lowest = student->grade[j];
                    }
                }

                printf("(Child %d) highest grade was %d and lowest was %d !\n", i, highest, lowest);
                student->child_to_read = i + 1; // Signal next child
                exit(EXIT_SUCCESS);
            }
            if (i == 1){
                // Child 1: Calculate average grade
                printf("(Child %d) Waiting for data...\n", i);
                
                // Wait for data to be present AND it's my turn
                while (!student->data_present || student->child_to_read != i) {
                    usleep(10000); // Small delay to prevent busy waiting
                }
                
                int sum = 0;
                for (int j = 0; j < NR_COURSES; j++){
                    sum += student->grade[j];
                }
                int average = sum / NR_COURSES;

                printf("(Child %d) Average grade was %d !\n", i, average);
                student->child_to_read = i + 1; // Signal completion
                exit(EXIT_SUCCESS);
            }
        }
    }

    // Parent process - collect student data
    printf("(Parent) Hey, Please enter the students name: \n");
    char buffer[64];
    fgets(buffer, sizeof(buffer), stdin);
    buffer[strcspn(buffer, "\n")] = '\0';
    strcpy(student->name, buffer);

    printf("(Parent) Hey, Please enter the students number: \n");
    char buffer_num[32];
    fgets(buffer_num, sizeof(buffer_num), stdin);
    buffer_num[strcspn(buffer_num, "\n")] = '\0';
    student->number = atoi(buffer_num);

    for (int i = 0; i < NR_COURSES; i++){
        printf("(Parent) Hey, Please enter the students grade for course %d: \n", i);
        char buffer_grade[32];
        fgets(buffer_grade, sizeof(buffer_grade), stdin);
        buffer_grade[strcspn(buffer_grade, "\n")] = '\0';
        student->grade[i] = atoi(buffer_grade);
    }

    // Signal that data is ready for children to process
    student->data_present = 1;
    printf("(Parent) Data is ready! Children can now process it.\n");

    // Wait for both children to finish
    for (int i = 0; i < 2; i++){
        wait(NULL);
    }

    printf("(Parent) ALL DONE ! :)\n");

    munmap(student, sizeof(student_t));
    close(fd);
    shm_unlink("/students");

    exit(EXIT_SUCCESS);
}