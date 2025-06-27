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
#define NR_STUDENTS 20

typedef struct{
    int number;
    char name[STR_SIZE];
    int grade[NR_COURSES];
} student_t;

typedef struct{
    student_t students[NR_STUDENTS];
    int current_student;      // Which student is being processed
    int data_present;         // Flag to indicate if data is ready
    int child_to_read;        // Which child should process next
    int students_processed;   // How many students have been processed
    int writing;              // Flag to prevent concurrent writes
} shared_data_t;

int main() {
    shared_data_t *shared_data;

    // Clean up any existing shared memory segment first
    shm_unlink("/students");

    int fd = shm_open("/students", O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }
    
    ftruncate(fd, sizeof(shared_data_t));
    shared_data = mmap(NULL, sizeof(shared_data_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shared_data == MAP_FAILED) {
        perror("mmap");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // Initialize shared memory
    shared_data->current_student = 0;
    shared_data->data_present = 0;
    shared_data->child_to_read = 0;
    shared_data->students_processed = 0;
    shared_data->writing = 0;
    memset(shared_data->students, 0, NR_STUDENTS * sizeof(student_t));

    // Create child processes
    for (int i = 0; i < 2; i++){
        pid_t pid = fork();

        if (pid == 0){
            // Child process
            if (i == 0){
                // Child 0: Calculate highest and lowest grades for each student
                printf("(Child %d) Waiting for data...\n", i);
                
                while (shared_data->students_processed < NR_STUDENTS) {
                    // Wait for data to be present AND it's my turn
                    while (!shared_data->data_present || shared_data->child_to_read != i) {
                        usleep(10000);
                    }
                    
                    if (shared_data->students_processed >= NR_STUDENTS) {
                        break;
                    }
                    
                    int student_idx = shared_data->current_student;
                    student_t *student = &shared_data->students[student_idx];
                    
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

                    printf("(Child %d) Student %s (%d): highest grade was %d and lowest was %d\n", 
                           i, student->name, student->number, highest, lowest);
                    
                    shared_data->child_to_read = i + 1; // Signal next child
                }
                exit(EXIT_SUCCESS);
            }
            if (i == 1){
                // Child 1: Calculate average grade for each student
                printf("(Child %d) Waiting for data...\n", i);
                
                while (shared_data->students_processed < NR_STUDENTS) {
                    // Wait for data to be present AND it's my turn
                    while (!shared_data->data_present || shared_data->child_to_read != i) {
                        usleep(10000);
                    }
                    
                    if (shared_data->students_processed >= NR_STUDENTS) {
                        break;
                    }
                    
                    int student_idx = shared_data->current_student;
                    student_t *student = &shared_data->students[student_idx];
                    
                    int sum = 0;
                    for (int j = 0; j < NR_COURSES; j++){
                        sum += student->grade[j];
                    }
                    int average = sum / NR_COURSES;

                    printf("(Child %d) Student %s (%d): Average grade was %d\n", 
                           i, student->name, student->number, average);
                    
                    shared_data->child_to_read = 0; // Signal parent to process next student
                    shared_data->students_processed++;
                    exit(EXIT_SUCCESS);
                }
                exit(EXIT_SUCCESS);
            }
        }
    }

    // Parent process - collect data for all students
    for (int student_idx = 0; student_idx < NR_STUDENTS; student_idx++) {
        printf("\n=== Processing Student %d/%d ===\n", student_idx + 1, NR_STUDENTS);
        
        // Wait for children to finish processing previous student
        while (shared_data->child_to_read != 0) {
            usleep(10000);
        }
        
        // Set current student index
        shared_data->current_student = student_idx;
        
        // Collect student data
        printf("(Parent) Please enter student %d's name: \n", student_idx + 1);
        char buffer[64];
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = '\0';
        strcpy(shared_data->students[student_idx].name, buffer);

        printf("(Parent) Please enter student %d's number: \n", student_idx + 1);
        char buffer_num[32];
        fgets(buffer_num, sizeof(buffer_num), stdin);
        buffer_num[strcspn(buffer_num, "\n")] = '\0';
        shared_data->students[student_idx].number = atoi(buffer_num);

        for (int course = 0; course < NR_COURSES; course++){
            printf("(Parent) Please enter student %d's grade for course %d: \n", student_idx + 1, course);
            char buffer_grade[32];
            fgets(buffer_grade, sizeof(buffer_grade), stdin);
            buffer_grade[strcspn(buffer_grade, "\n")] = '\0';
            shared_data->students[student_idx].grade[course] = atoi(buffer_grade);
        }

        // Signal that data is ready for children to process
        shared_data->data_present = 1;
        printf("(Parent) Student %d data is ready! Children can now process it.\n", student_idx + 1);
        
        // Wait for both children to finish processing this student
        while (shared_data->students_processed <= student_idx) {
            usleep(10000);
        }
        
        // Reset for next student
        shared_data->data_present = 0;
        shared_data->child_to_read = 0;
    }

    printf("\n(Parent) ALL STUDENTS PROCESSED! :)\n");

    // Wait for children to finish
    for (int i = 0; i < 2; i++){
        wait(NULL);
    }

    munmap(shared_data, sizeof(shared_data_t));
    close(fd);
    shm_unlink("/students");

    exit(EXIT_SUCCESS);
} 