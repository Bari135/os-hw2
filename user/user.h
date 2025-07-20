struct stat;

// system calls
int fork(void);
int exit(int) __attribute__((noreturn));
int wait(int*);
int pipe(int*);
int write(int, const void*, int);
int read(int, void*, int);
int close(int);
int kill(int);
int exec(const char*, char**);
int open(const char*, int);
int mknod(const char*, short, short);
int unlink(const char*);
int fstat(int fd, struct stat*);
int link(const char*, const char*);
int mkdir(const char*);
int chdir(const char*);
int dup(int);
int getpid(void);
char* sbrk(int);
int sleep(int);
int uptime(void);

/* Added for Task 1 - Peterson Lock API */
int peterson_create(void);
int peterson_acquire(int, int);
int peterson_release(int, int);
int peterson_destroy(int);

// ulib.c
int stat(const char*, struct stat*);
char* strcpy(char*, const char*);
void *memmove(void*, const void*, int);
char* strchr(const char*, char c);
int strcmp(const char*, const char*);
void fprintf(int, const char*, ...);
void printf(const char*, ...);
char* gets(char*, int max);
uint strlen(const char*);
void* memset(void*, int, uint);
void* malloc(uint);
void free(void*);
int atoi(const char*);
int memcmp(const void *, const void *, uint);
void *memcpy(void *, const void *, uint);

/* Added for Task 2 - Tournament Lock API */

/**
 * @brief Creates a new tournament tree with the specified number of processes
 * 
 * @param processes Number of processes to participate in the tournament (must be a power of 2, max 16)
 * @return int Returns:
 *         - Process index (0 to processes-1) on success
 *         - -1 on error (invalid number of processes, creation failure, fork failure)
 * 
 * This function:
 * - Validates that processes is a power of 2 and <= 16
 * - Creates all necessary Peterson locks for the tournament tree
 * - Forks the specified number of child processes
 * - Assigns each process its unique index and roles in the tree
 * - Does not clean up on fork failure
 */
int tournament_create(int processes);

/**
 * @brief Attempts to acquire the tournament lock for the calling process
 * 
 * @return int Returns:
 *         - 0 on successful lock acquisition
 *         - -1 on error
 * 
 * This function:
 * - Uses the process's assigned index from tournament_create
 * - Traverses the tournament tree bottom-up
 * - Acquires necessary Peterson locks at each level
 * - Must be called after a successful tournament_create
 */
int tournament_acquire(void);

/**
 * @brief Releases all locks held by the calling process
 * 
 * @return int Returns:
 *         - 0 on successful release of all locks
 *         - -1 on error
 * 
 * This function:
 * - Releases all acquired locks in reverse order (top-down)
 * - Must be called after a successful tournament_acquire
 * - Resets the process's lock acquisition state
 */
int tournament_release(void);