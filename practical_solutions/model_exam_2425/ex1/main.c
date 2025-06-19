#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <time.h>

// --- Constants ---
#define N_SCANNER 5      // Number of concurrent child processes (scanners) to create.
#define N_TICKETS 10     // Number of tickets in our simulated database.
#define P_READ 0         // Index for the read-end of a pipe array.
#define P_WRITE 1        // Index for the write-end of a pipe array.

// --- Data Structures ---

// Represents a single ticket in the museum's database.
typedef struct {
    char id[10];
    char info[50];
    char validity[20];
} Ticket;

// Represents a request sent from a scanner (child) to the database (parent).
// It includes the scanner's own ID so the parent knows where to send the response.
typedef struct {
    int scanner_id;
    char ticket_id[10];
} Request;

// --- Function Prototypes ---
void scanner_logic(int id, int read_pipe, int write_pipe, Ticket db[]);
void parent_logic(int read_pipe, int write_pipes[], Ticket db[]);
void init_database(Ticket db[]);

// --- Main Program Entry ---
int main() {
    // 1. SETUP: Initialize the database with some dummy data.
    Ticket database[N_TICKETS];
    init_database(database); // Note: For an exam, you might just assume this is populated.

    // 2. SETUP: Create the shared pipe.
    // All scanners (children) will write their requests to this one pipe.
    int m2p_pipe[2]; // "Many-to-Parent" pipe
    if (pipe(m2p_pipe) == -1) {
        perror("Shared pipe creation failed");
        exit(EXIT_FAILURE);
    }

    // 3. SETUP: Create arrays to hold data for each child process.
    int p2c_pipes[N_SCANNER][2];    // Array of "Parent-to-Child" pipes. One pipe per scanner.
    int p2c_write_pipes[N_SCANNER]; // Array to hold just the write ends for the parent's convenience.
    pid_t pids[N_SCANNER];          // Array to store the process IDs of the children.

    // 4. PROCESS CREATION: Loop to fork N_SCANNER child processes.
    for (int i = 0; i < N_SCANNER; i++) {
        // Create a dedicated, private pipe for this specific child.
        if (pipe(p2c_pipes[i]) == -1) {
            perror("Scanner-specific pipe creation failed");
            exit(EXIT_FAILURE);
        }

        // Fork the current process.
        pids[i] = fork();
        if (pids[i] < 0) {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        }

        // --- CHILD PROCESS LOGIC ---
        if (pids[i] == 0) {  /* Code inside this block is only executed by the child */
            // A. CLOSE UNUSED SHARED PIPE END: The child only WRITES to the shared pipe.
            close(m2p_pipe[P_READ]);

            // B. CLOSE UNUSED DEDICATED PIPE ENDS:
            // This loop ensures the child only keeps its own dedicated pipe open for reading.
            for (int j = 0; j < N_SCANNER; j++) {
                if (i == j) {
                    // This is my dedicated pipe. I only READ from it, so close the write end.
                    close(p2c_pipes[j][P_WRITE]);
                } else {
                    // This pipe belongs to another child. I have no business with it, so close both ends.
                    close(p2c_pipes[j][P_READ]);
                    close(p2c_pipes[j][P_WRITE]);
                }
            }

            // C. EXECUTE THE CORE LOGIC: Run the function that defines a scanner's behavior.
            scanner_logic(i, p2c_pipes[i][P_READ], m2p_pipe[P_WRITE], database);

            // D. FINAL CLEANUP: Close the remaining pipe ends before exiting.
            close(m2p_pipe[P_WRITE]);
            close(p2c_pipes[i][P_READ]);
            exit(EXIT_SUCCESS); // Child process is finished.
        }
    }

    // --- PARENT PROCESS LOGIC ---
    /* Code from here on is only executed by the parent process */

    // A. CLOSE UNUSED SHARED PIPE END: The parent only READS from the shared pipe.
    close(m2p_pipe[P_WRITE]);

    // B. CLOSE UNUSED DEDICATED PIPE ENDS: The parent only WRITES to the dedicated pipes.
    // We also store the write ends in a separate array for easier access later.
    for (int i = 0; i < N_SCANNER; i++) {
        close(p2c_pipes[i][P_READ]);
        p2c_write_pipes[i] = p2c_pipes[i][P_WRITE];
    }

    // C. EXECUTE THE CORE LOGIC: Run the function that defines the parent's server behavior.
    parent_logic(m2p_pipe[P_READ], p2c_write_pipes, database);

    // D. FINAL CLEANUP: Close the remaining pipe ends now that the work is done.
    close(m2p_pipe[P_READ]);
    for (int i = 0; i < N_SCANNER; i++) {
        close(p2c_write_pipes[i]);
    }

    // E. WAIT FOR CHILDREN: The parent waits for all child processes to terminate.
    // This prevents the parent from exiting prematurely and creating zombie processes.
    for (int i = 0; i < N_SCANNER; i++) {
        waitpid(pids[i], NULL, 0);
    }

    printf("Parent: All scanners have shut down.\n");

    return 0;
}

/**
 * @brief The core logic for a scanner process (child).
 * @param id The unique ID of this scanner (0 to N_SCANNER-1).
 * @param read_pipe The file descriptor for READING responses from the parent.
 * @param write_pipe The file descriptor for WRITING requests to the parent.
 * @param db A read-only copy of the database to pick a ticket from.
 */
void scanner_logic(int id, int read_pipe, int write_pipe, Ticket db[]) {
    // Seed the random number generator. XORing with getpid() ensures each child is unique.
    srand(time(NULL) ^ getpid());

    // Simulate scanning by picking a random ticket from the database.
    int ticket_index = rand() % N_TICKETS;

    // Prepare the request packet. Include our own ID so the parent knows who to reply to.
    Request req;
    req.scanner_id = id;
    strcpy(req.ticket_id, db[ticket_index].id);

    printf("[Scanner %d] Requesting info for ticket: %s\n", id, req.ticket_id);
    fflush(stdout);

    // Send the request to the parent through the shared pipe.
    if (write(write_pipe, &req, sizeof(Request)) == -1) {
        perror("Scanner write failed");
        return;
    }

    // Wait for a response from the parent on the dedicated pipe.
    // The read() call will block until the parent sends data.
    Ticket response_ticket;
    if (read(read_pipe, &response_ticket, sizeof(Ticket)) > 0) {
        // Print the information received from the parent.
        printf("\n--- Ticket Info (Scanner %d) ---\n", id);
        printf("  ID: %s\n", response_ticket.id);
        printf("  Info: %s\n", response_ticket.info);
        printf("  Status: %s\n", response_ticket.validity);
        printf("----------------------------------\n\n");
        fflush(stdout);
    } else {
        perror("Scanner read failed");
    }
}

/**
 * @brief The core logic for the database server (parent).
 * @param read_pipe The file descriptor for READING requests from the shared pipe.
 * @param write_pipes An array of file descriptors for WRITING responses to each child.
 * @param db The ticket database.
 */
void parent_logic(int read_pipe, int write_pipes[], Ticket db[]) {
    Request req;

    // The server loop. It continues as long as there are active scanners.
    while (1) {
        // Wait for a request to arrive from any scanner on the shared pipe.
        // This read() blocks until data is available.
        ssize_t bytes_read = read(read_pipe, &req, sizeof(Request));

        if (bytes_read > 0) { // Successfully read a request
            printf("Parent: Received request from scanner %d for ticket %s.\n", req.scanner_id, req.ticket_id);

            // Search for the requested ticket in the database.
            Ticket found_ticket;
            int found = 0;
            for (int i = 0; i < N_TICKETS; i++) {
                if (strcmp(db[i].id, req.ticket_id) == 0) {
                    found_ticket = db[i];
                    found = 1;
                    break;
                }
            }

            // If the ticket ID was not found, prepare an "UNKNOWN" response.
            if (!found) {
                 strcpy(found_ticket.id, req.ticket_id);
                 strcpy(found_ticket.info, "---");
                 strcpy(found_ticket.validity, "UNKNOWN TICKET");
            }

            // Send the response back to the correct scanner using its dedicated pipe.
            // The scanner_id from the request tells us which pipe to use.
            if (write(write_pipes[req.scanner_id], &found_ticket, sizeof(Ticket)) == -1) {
                perror("Parent write failed");
            }
        } else if (bytes_read == 0) {
            // read() returns 0 when all write ends of the pipe have been closed.
            // This means all children have exited. The server can now shut down.
            printf("Parent: All scanners have closed the pipe. Shutting down.\n");
            break;
        } else {
            // An error occurred on read.
            perror("Parent read failed");
            break;
        }
    }
}

/**
 * @brief A helper function to populate the database with dummy ticket data.
 * @param db The Ticket array to fill.
 */
// wouldnt really need this method but its here for testing purposes
void init_database(Ticket db[]) {
    for (int i = 0; i < N_TICKETS; i++) {
        sprintf(db[i].id, "TICKET%03d", i);
        sprintf(db[i].info, "Access to Exhibit #%d", i);
        if (i % 2 == 0) {
            strcpy(db[i].validity, "VALID");
        } else {
            strcpy(db[i].validity, "INVALID");
        }
    }
}    