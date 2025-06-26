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

#define READERS 5
#define PRODUCTS 1000

// Struct for the product database
typedef struct{
    char name[50];
    double price;
} Product;

// Struct for the request sent from child to parent
typedef struct {
    int reader_id; // To identify which child is asking
    char product_name[50];
} Request;

int main() {
    Product database[PRODUCTS];

    // Populate the database with product names and prices
    for (int i = 0; i < PRODUCTS; i++) {
        snprintf(database[i].name, sizeof(database[i].name), "product%d", i);
        database[i].price = (i + 1) * 1.5;
    }

    // --- Pipe Setup ---
    int response_pipes[READERS][2]; // Pipes from parent back to each child
    int request_pipe[2];           // One pipe from all children to parent

    // Create the single request pipe
    if (pipe(request_pipe) == -1) {
        perror("Request pipe failed");
        exit(EXIT_FAILURE);
    }

    // Create a unique response pipe for each reader/child
    for (int i = 0; i < READERS; i++) {
        if (pipe(response_pipes[i]) == -1) {
            perror("Response pipe failed");
            exit(EXIT_FAILURE);
        }
    }

    // --- Fork Children ---
    for (int i = 0; i < READERS; i++){
        pid_t pid = fork();

        if (pid < 0) {
            perror("fork failed");
            exit(EXIT_FAILURE);
        }

        if (pid == 0){
            // --- CHILD (READER) PROCESS ---

            // 1. Close all unused pipe ends
            close(request_pipe[P_READ]); // Child only writes to the request pipe
            for (int j = 0; j < READERS; j++) {
                close(response_pipes[j][P_WRITE]); // Child only reads from its own response pipe
                if (i != j) {
                    close(response_pipes[j][P_READ]); // Close other children's read pipes
                    close(response_pipes[j][P_WRITE]); // Close other children's write pipes
                }
            }

            // 2. Simulate scanning a product and create a request
            Request req;
            req.reader_id = i; // Identify which reader this is
            // For this example, each reader requests a different product
            snprintf(req.product_name, sizeof(req.product_name), "product%d", i * 10);

            // 3. Send the request to the parent
            printf("[Reader %d] Requesting price for: %s\n", i, req.product_name);
            write(request_pipe[P_WRITE], &req, sizeof(req));
            close(request_pipe[P_WRITE]); // Done writing request

            // 4. Wait for and read the price from the parent
            double received_price;
            read(response_pipes[i][P_READ], &received_price, sizeof(received_price));
            close(response_pipes[i][P_READ]); // Done reading response

            // 5. Print the final result
            printf("[Reader %d] Display: Product '%s' costs %.2f $\n", i, req.product_name, received_price);

            exit(EXIT_SUCCESS);
        }
    }

    // --- PARENT PROCESS ---

    // 1. Close all unused pipe ends
    close(request_pipe[P_WRITE]); // Parent only reads from the request pipe
    for (int i = 0; i < READERS; i++) {
        close(response_pipes[i][P_READ]); // Parent only writes to response pipes
    }

    // 2. Loop to handle one request from each of the 5 readers
    for (int i = 0; i < READERS; i++) {
        Request received_req;
        double price_to_send = -1.0; // Default price if not found

        // 3. Read a request from any child
        read(request_pipe[P_READ], &received_req, sizeof(received_req));

        // 4. Find the product in the database
        for (int j = 0; j < PRODUCTS; j++) {
            if (strcmp(database[j].name, received_req.product_name) == 0) {
                price_to_send = database[j].price;
                break;
            }
        }

        // 5. Send the price back to the specific child that requested it
        int target_reader = received_req.reader_id;
        printf("[Parent] Replying to Reader %d for product '%s' with price %.2f\n",
               target_reader, received_req.product_name, price_to_send);
        write(response_pipes[target_reader][P_WRITE], &price_to_send, sizeof(price_to_send));
    }

    // --- Cleanup ---
    close(request_pipe[P_READ]);
    for (int i = 0; i < READERS; i++) {
        close(response_pipes[i][P_WRITE]);
        wait(NULL); // Wait for each child to terminate
    }

    printf("[Parent] All readers have finished. Shutting down.\n");
    exit(EXIT_SUCCESS);
}