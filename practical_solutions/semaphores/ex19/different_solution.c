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
#include <semaphore.h>
#define P_READ 0
#define P_WRITE 1

typedef struct {
    int cars_waiting_east;
    int cars_waiting_west;
    int current_direction;  // -1 = none, 0 = east, 1 = west
    int cars_on_bridge;
} BridgeState;

sig_atomic_t volatile keep_going;

int main() {
    sem_t *mutex;      // Mutual exclusion for bridge state
    sem_t *east_queue; // Semaphore for east cars waiting
    sem_t *west_queue; // Semaphore for west cars waiting
    
    BridgeState *state;
    
    // Create shared memory for bridge state
    int fd = shm_open("/bridge_state", O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
    ftruncate(fd, sizeof(BridgeState));
    state = mmap(NULL, sizeof(BridgeState), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    
    // Initialize bridge state
    state->cars_waiting_east = 0;
    state->cars_waiting_west = 0;
    state->current_direction = -1;  // No cars on bridge initially
    state->cars_on_bridge = 0;
    
    // Initialize semaphores
    mutex = sem_open("/sem_mutex", O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR, 1);
    east_queue = sem_open("/sem_east", O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR, 0);
    west_queue = sem_open("/sem_west", O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR, 0);

    keep_going = 1;
    srand(time(NULL));
    
    while(keep_going){
        printf("Do you want to keep going? (y/n)\n");
        char buffer[12];
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = '\0';

        if (strcmp(buffer, "n") == 0){
            keep_going = 0;
            break;
        }

        // Create random number of cars (1-3) with random directions
        int num_cars = (rand() % 3) + 1;
        printf("Creating %d cars...\n", num_cars);
        
        for (int i = 0; i < num_cars; i++){
            pid_t pid = fork();

            if (pid == 0){
                // Child process - car
                int car_direction = rand() % 2;  // 0 = east, 1 = west
                int car_id = getpid() % 1000;  // Simple car ID
                
                if (car_direction == 0) {
                    printf("Car %d from EAST arrives\n", car_id);
                    fflush(stdout);
                    
                    // Enter critical section to check bridge state
                    sem_wait(mutex);
                    state->cars_waiting_east++;
                    printf("Car %d from EAST waiting (East: %d, West: %d, Current: %d)\n", 
                           car_id, state->cars_waiting_east, state->cars_waiting_west, state->current_direction);
                    fflush(stdout);
                    
                    // Check if can cross immediately
                    if (state->current_direction == -1 || state->current_direction == 0) {
                        // Can cross immediately
                        if (state->current_direction == -1) {
                            state->current_direction = 0;  // Set direction to east
                        }
                        state->cars_on_bridge++;
                        state->cars_waiting_east--;
                        sem_post(mutex);
                        
                        printf("Car %d from EAST: Traversing bridge towards WEST... VRUUUM\n", car_id);
                        fflush(stdout);
                        sleep(1);  // 1 minute traversal (1 second for testing)
                        
                        // Exit bridge
                        sem_wait(mutex);
                        state->cars_on_bridge--;
                        if (state->cars_on_bridge == 0) {
                            state->current_direction = -1;  // Bridge is free
                            printf("Bridge is now free\n");
                            fflush(stdout);
                            
                            // Signal waiting cars from opposite direction
                            if (state->cars_waiting_west > 0) {
                                sem_post(west_queue);
                            } else if (state->cars_waiting_east > 0) {
                                sem_post(east_queue);
                            }
                        }
                        sem_post(mutex);
                        
                    } else {
                        // Must wait for opposite direction to finish
                        sem_post(mutex);
                        sem_wait(east_queue);  // Wait for permission to cross
                        
                        // Got permission, enter bridge
                        sem_wait(mutex);
                        state->cars_waiting_east--;
                        state->cars_on_bridge++;
                        if (state->current_direction == -1) {
                            state->current_direction = 0;
                        }
                        sem_post(mutex);
                        
                        printf("Car %d from EAST: Traversing bridge towards WEST... VRUUUM\n", car_id);
                        fflush(stdout);
                        sleep(1);  // 1 minute traversal (1 second for testing)
                        
                        // Exit bridge
                        sem_wait(mutex);
                        state->cars_on_bridge--;
                        if (state->cars_on_bridge == 0) {
                            state->current_direction = -1;  // Bridge is free
                            printf("Bridge is now free\n");
                            fflush(stdout);
                            
                            // Signal waiting cars from opposite direction
                            if (state->cars_waiting_west > 0) {
                                sem_post(west_queue);
                            } else if (state->cars_waiting_east > 0) {
                                sem_post(east_queue);
                            }
                        }
                        sem_post(mutex);
                    }
                    
                    printf("Car %d from EAST: Finished crossing\n", car_id);
                    fflush(stdout);
                    
                } else {
                    printf("Car %d from WEST arrives\n", car_id);
                    fflush(stdout);
                    
                    // Enter critical section to check bridge state
                    sem_wait(mutex);
                    state->cars_waiting_west++;
                    printf("Car %d from WEST waiting (East: %d, West: %d, Current: %d)\n", 
                           car_id, state->cars_waiting_east, state->cars_waiting_west, state->current_direction);
                    fflush(stdout);
                    
                    // Check if can cross immediately
                    if (state->current_direction == -1 || state->current_direction == 1) {
                        // Can cross immediately
                        if (state->current_direction == -1) {
                            state->current_direction = 1;  // Set direction to west
                        }
                        state->cars_on_bridge++;
                        state->cars_waiting_west--;
                        sem_post(mutex);
                        
                        printf("Car %d from WEST: Traversing bridge towards EAST... VRUUUM\n", car_id);
                        fflush(stdout);
                        sleep(1);  // 1 minute traversal (1 second for testing)
                        
                        // Exit bridge
                        sem_wait(mutex);
                        state->cars_on_bridge--;
                        if (state->cars_on_bridge == 0) {
                            state->current_direction = -1;  // Bridge is free
                            printf("Bridge is now free\n");
                            fflush(stdout);
                            
                            // Signal waiting cars from opposite direction
                            if (state->cars_waiting_east > 0) {
                                sem_post(east_queue);
                            } else if (state->cars_waiting_west > 0) {
                                sem_post(west_queue);
                            }
                        }
                        sem_post(mutex);
                        
                    } else {
                        // Must wait for opposite direction to finish
                        sem_post(mutex);
                        sem_wait(west_queue);  // Wait for permission to cross
                        
                        // Got permission, enter bridge
                        sem_wait(mutex);
                        state->cars_waiting_west--;
                        state->cars_on_bridge++;
                        if (state->current_direction == -1) {
                            state->current_direction = 1;
                        }
                        sem_post(mutex);
                        
                        printf("Car %d from WEST: Traversing bridge towards EAST... VRUUUM\n", car_id);
                        fflush(stdout);
                        sleep(1);  // 1 minute traversal (1 second for testing)
                        
                        // Exit bridge
                        sem_wait(mutex);
                        state->cars_on_bridge--;
                        if (state->cars_on_bridge == 0) {
                            state->current_direction = -1;  // Bridge is free
                            printf("Bridge is now free\n");
                            fflush(stdout);
                            
                            // Signal waiting cars from opposite direction
                            if (state->cars_waiting_east > 0) {
                                sem_post(east_queue);
                            } else if (state->cars_waiting_west > 0) {
                                sem_post(west_queue);
                            }
                        }
                        sem_post(mutex);
                    }
                    
                    printf("Car %d from WEST: Finished crossing\n", car_id);
                    fflush(stdout);
                }
                
                exit(EXIT_SUCCESS);
            }
        }

        // Wait for all children to finish
        for (int i = 0; i < num_cars; i++){
            wait(NULL);
        }
        
        printf("All cars from this round finished\n\n");
        fflush(stdout);
    }
    
    // Clean up
    sem_close(mutex);
    sem_close(east_queue);
    sem_close(west_queue);
    sem_unlink("/sem_mutex");
    sem_unlink("/sem_east");
    sem_unlink("/sem_west");
    munmap(state, sizeof(BridgeState));
    close(fd);
    shm_unlink("/bridge_state");

    exit(EXIT_SUCCESS);
} 