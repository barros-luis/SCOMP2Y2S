# EXAM QUICK REFERENCE SHEET

## 1. MOST USED FUNCTIONS

### PROCESSES & EXEC
```c
// Process Creation
pid_t fork(void);                                    // Returns: child_pid (parent), 0 (child), -1 (error)

// Process Waiting
pid_t wait(int *status);                             // status: pointer to store exit status
pid_t waitpid(pid_t pid, int *status, 0);            // pid: specific child, 0: wait for child

// Process Execution
int execl(const char *path, const char *arg0, ...);  // path: executable path, arg0: program name, ...: args, NULL
int execlp(const char *file, const char *arg0, ...); // file: executable name (searches PATH)
int execv(const char *path, char *const argv[]);     // path: executable path, argv: array of args
int execvp(const char *file, char *const argv[]);    // file: executable name, argv: array of args

// Process Control
pid_t getpid(void);                                  // Returns current process ID
pid_t getppid(void);                                 // Returns parent process ID
void exit(int status);                               // status: exit code (EXIT_SUCCESS, EXIT_FAILURE)
```

### PIPES
```c
// Pipe Creation
int pipe(int fd[2]);                                 // fd[0]: read end, fd[1]: write end

// File Descriptor Operations
int dup2(int oldfd, int newfd);                      // oldfd: source, newfd: destination
int close(int fd);                                   // fd: file descriptor to close
ssize_t read(int fd, void *buf, size_t count);      // fd: file descriptor, buf: buffer, count: bytes to read
ssize_t write(int fd, const void *buf, size_t count); // fd: file descriptor, buf: data, count: bytes to write
```

### SEMAPHORES
```c
// Named Semaphores (POSIX)
sem_t *sem_open(const char *name, int oflag, mode_t mode, unsigned int value);
// name: semaphore name, oflag: O_CREAT|O_EXCL|O_RDWR, mode: S_IRUSR|S_IWUSR, value: initial value

int sem_wait(sem_t *sem);                            // sem: semaphore to wait on
int sem_post(sem_t *sem);                            // sem: semaphore to signal
int sem_close(sem_t *sem);                           // sem: semaphore to close
int sem_unlink(const char *name);                    // name: semaphore name to remove

// Unnamed Semaphores (POSIX)
int sem_init(sem_t *sem, int pshared, unsigned int value); // pshared: 0 (threads), value: initial value
int sem_destroy(sem_t *sem);                         // sem: semaphore to destroy
```

### THREADS
```c
// Thread Creation
int pthread_create(pthread_t *thread, NULL, void *(*start_routine)(void*), void *arg);
// thread: pointer to thread ID, NULL: default attributes, start_routine: function to run, arg: function argument

// Thread Synchronization
int pthread_join(pthread_t thread, NULL);             // thread: thread to wait for, NULL: ignore return value
void pthread_exit(void *retval);                     // retval: return value

// Mutexes
int pthread_mutex_init(pthread_mutex_t *mutex, NULL); // NULL: default attributes
int pthread_mutex_lock(pthread_mutex_t *mutex);      // mutex: mutex to lock
int pthread_mutex_unlock(pthread_mutex_t *mutex);    // mutex: mutex to unlock
int pthread_mutex_destroy(pthread_mutex_t *mutex);   // mutex: mutex to destroy

// Condition Variables
int pthread_cond_init(pthread_cond_t *cond, NULL);   // NULL: default attributes
int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex); // cond: condition, mutex: locked mutex
int pthread_cond_signal(pthread_cond_t *cond);       // cond: condition to signal one thread
int pthread_cond_broadcast(pthread_cond_t *cond);    // cond: condition to signal all threads
int pthread_cond_destroy(pthread_cond_t *cond);      // cond: condition to destroy
```

### SIGNALS
```c
// Signal Handling
int sigaction(int sig, const struct sigaction *act, struct sigaction *oact);
// sig: signal number (SIGUSR1, SIGTERM, etc.), act: new handler, oact: old handler

// Signal Sending
int kill(pid_t pid, int sig);                        // pid: target process, sig: signal to send
int raise(int sig);                                  // sig: signal to send to self

// Signal Waiting
int pause(void);                                     // Wait for any signal
```

### SHARED MEMORY
```c
// Shared Memory Operations
int shm_open(const char *name, int oflag, mode_t mode); // name: memory name, oflag: O_CREAT|O_EXCL|O_RDWR, mode: S_IRUSR|S_IWUSR
int ftruncate(int fd, off_t length);                 // fd: file descriptor, length: size in bytes
void *mmap(NULL, size_t length, PROT_READ|PROT_WRITE, MAP_SHARED, int fd, 0);
// NULL: let system choose address, length: size, PROT_READ|PROT_WRITE: permissions, MAP_SHARED: shared mapping, fd: file descriptor, 0: offset
int munmap(void *addr, size_t length);               // addr: mapped address, length: size
int shm_unlink(const char *name);                    // name: shared memory name
```

### STRING & I/O FUNCTIONS
```c
// String Operations
char *strcpy(char *dest, const char *src);           // dest: destination, src: source
char *strncpy(char *dest, const char *src, size_t n); // n: max characters to copy
char *strcat(char *dest, const char *src);           // dest: destination, src: source to append
int strcmp(const char *s1, const char *s2);          // s1, s2: strings to compare
size_t strlen(const char *s);                        // s: string to measure
char *strchr(const char *s, int c);                  // s: string, c: character to find
char *strcspn(const char *s, const char *reject);    // s: string, reject: characters to avoid

// Formatted I/O
int sprintf(char *str, const char *format, ...);     // str: destination buffer, format: format string
int snprintf(char *str, size_t size, const char *format, ...); // size: buffer size limit
int printf(const char *format, ...);                 // format: format string
int fprintf(FILE *stream, const char *format, ...);  // stream: file pointer, format: format string

// File Operations
FILE *fopen(const char *pathname, const char *mode); // pathname: file path, mode: "r", "w", "a", etc.
int fclose(FILE *stream);                            // stream: file pointer
char *fgets(char *s, int size, FILE *stream);        // s: buffer, size: max chars, stream: file pointer
int fputs(const char *s, FILE *stream);              // s: string, stream: file pointer
int fprintf(FILE *stream, const char *format, ...);  // stream: file pointer, format: format string

// Time Functions
time_t time(NULL);                                   // NULL: current time
struct tm *localtime(const time_t *timep);           // timep: time value
size_t strftime(char *s, size_t max, const char *format, const struct tm *tm);
// s: destination, max: max chars, format: format string, tm: time structure
```

## 2. ERROR CHECKING PATTERNS

### PROCESS CREATION
```c
pid_t pid = fork();
if (pid < 0) {
    perror("fork failed");
    exit(EXIT_FAILURE);
}
```

### PROCESS WAITING
```c
int status;
pid_t child_pid = wait(&status);
if (child_pid < 0) {
    perror("wait failed");
    exit(EXIT_FAILURE);
}

// Check exit status
if (WIFEXITED(status)) {
    printf("Child exited with code: %d\n", WEXITSTATUS(status));
}
```

### EXEC FUNCTIONS
```c
execlp("program", "program", "arg1", "arg2", NULL);
perror("execlp failed");  // Only reached if exec fails
exit(EXIT_FAILURE);
```

### PIPE CREATION
```c
int fd[2];
if (pipe(fd) == -1) {
    perror("pipe failed");
    exit(EXIT_FAILURE);
}
```

### SEMAPHORE OPERATIONS
```c
sem_t *sem = sem_open("/sem_name", O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR, 1);
if (sem == SEM_FAILED) {
    perror("sem_open failed");
    exit(EXIT_FAILURE);
}

if (sem_wait(sem) == -1) {
    perror("sem_wait failed");
    exit(EXIT_FAILURE);
}
```

### THREAD CREATION
```c
pthread_t thread;
int result = pthread_create(&thread, NULL, thread_function, NULL);
if (result != 0) {
    perror("pthread_create failed");
    exit(EXIT_FAILURE);
}
```

### MUTEX OPERATIONS
```c
pthread_mutex_t mutex;
if (pthread_mutex_init(&mutex, NULL) != 0) {
    perror("pthread_mutex_init failed");
    exit(EXIT_FAILURE);
}

if (pthread_mutex_lock(&mutex) != 0) {
    perror("pthread_mutex_lock failed");
    exit(EXIT_FAILURE);
}
```

### SHARED MEMORY
```c
int fd = shm_open("/shm_name", O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
if (fd == -1) {
    perror("shm_open failed");
    exit(EXIT_FAILURE);
}

void *ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
if (ptr == MAP_FAILED) {
    perror("mmap failed");
    close(fd);
    exit(EXIT_FAILURE);
}
```

### MEMORY ALLOCATION
```c
void *ptr = malloc(size);
if (ptr == NULL) {
    perror("malloc failed");
    exit(EXIT_FAILURE);
}
```

### FILE OPERATIONS
```c
FILE *file = fopen("filename.txt", "r");
if (file == NULL) {
    perror("fopen failed");
    exit(EXIT_FAILURE);
}
```

## 3. COMMON CONSTANTS & MACROS

```c
// Exit codes
EXIT_SUCCESS    // 0
EXIT_FAILURE    // 1

// File descriptors
STDIN_FILENO    // 0
STDOUT_FILENO   // 1
STDERR_FILENO   // 2

// Pipe ends
P_READ          // 0
P_WRITE         // 1

// File permissions
S_IRUSR         // User read
S_IWUSR         // User write
S_IRUSR | S_IWUSR  // User read and write

// Open flags
O_CREAT         // Create if doesn't exist
O_EXCL          // Exclusive creation
O_RDWR          // Read and write
O_RDONLY        // Read only
O_WRONLY        // Write only

// Memory protection
PROT_READ       // Readable
PROT_WRITE      // Writable
PROT_READ | PROT_WRITE  // Readable and writable

// Memory mapping
MAP_SHARED      // Shared between processes
MAP_PRIVATE     // Private copy

// Signal numbers
SIGUSR1         // User signal 1
SIGUSR2         // User signal 2
SIGTERM         // Termination signal
SIGINT          // Interrupt signal
SIGKILL         // Kill signal (cannot be caught)

// Wait options
WNOHANG         // Don't block if no child has exited
```

## 4. TYPICAL PATTERNS

### FORK-WAIT PATTERN
```c
for (int i = 0; i < num_children; i++) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }
    if (pid == 0) {
        // Child code
        exit(EXIT_SUCCESS);
    }
}

// Wait for all children
for (int i = 0; i < num_children; i++) {
    wait(NULL);
}
```

### PIPE PATTERN
```c
int fd[2];
pipe(fd);

pid_t pid = fork();
if (pid == 0) {
    // Child: close unused end
    close(fd[P_WRITE]);
    // Read from fd[P_READ]
    close(fd[P_READ]);
    exit(EXIT_SUCCESS);
}

// Parent: close unused end
close(fd[P_READ]);
// Write to fd[P_WRITE]
close(fd[P_WRITE]);
wait(NULL);
```

### THREAD PATTERN
```c
pthread_t threads[num_threads];
for (int i = 0; i < num_threads; i++) {
    pthread_create(&threads[i], NULL, thread_function, &i);
}

for (int i = 0; i < num_threads; i++) {
    pthread_join(threads[i], NULL);
}
```

### SHARED MEMORY PATTERN
```c
int fd = shm_open("/name", O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
ftruncate(fd, sizeof(DataStructure));
DataStructure *data = mmap(NULL, sizeof(DataStructure), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

// Use data...

munmap(data, sizeof(DataStructure));
close(fd);
shm_unlink("/name");
```

### SEMAPHORE PATTERN
```c
sem_t *sem = sem_open("/sem_name", O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR, 1);

sem_wait(sem);
// Critical section
sem_post(sem);

sem_close(sem);
sem_unlink("/sem_name");
```

## 5. IMPORTANT REMINDERS

- **Always check return values** for system calls
- **Close file descriptors** when done
- **Free allocated memory** (malloc/free)
- **Unmap shared memory** (munmap)
- **Close and unlink semaphores**
- **Destroy mutexes and condition variables**
- **Wait for child processes** to avoid zombies
- **Use snprintf instead of sprintf** for safety
- **Check for NULL pointers** before dereferencing
- **Handle signals properly** with sigaction
- **Use proper synchronization** in multi-threaded code 