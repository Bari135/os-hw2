#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("Usage: tournament <num_processes>\n");
        exit(1);
    }
    
    int n = atoi(argv[1]);
    
    // Create tournament tree
    int id = tournament_create(n);
    if (id < 0) {
        printf("Failed to create tournament\n");
        exit(1);
    }
    
    // Acquire the tournament lock
    if (tournament_acquire() < 0) {
        printf("Process %d failed to acquire lock\n", id);
        exit(1);
    }
    
    // Critical section
    printf("Process %d (PID %d) acquired the lock\n", id, getpid());
    
    // Sleep a bit to demonstrate lock exclusivity
    sleep(1);
    
    // Release the tournament lock
    if (tournament_release() < 0) {
        printf("Process %d failed to release lock\n", id);
        exit(1);
    }
    
    exit(0);
}