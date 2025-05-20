#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// Global variables to store tournament information
static int process_index = -1;     // Index of this process (0 to N-1)
static int num_processes = 0;      // Number of processes in tournament
static int num_levels = 0;         // Number of levels in tree
static int *lock_ids = 0;          // Array of Peterson lock IDs (BFS order)
static int *acquired_locks = 0;    // Track locks this process has acquired
static int *acquired_roles = 0;    // Track roles used for acquired locks
static int num_acquired = 0;       // Number of locks currently acquired

// Check if a number is a power of 2
static int is_power_of_two(int n) {
    return (n > 0) && ((n & (n - 1)) == 0);
}

// Calculate log base 2 of n (assuming n is a power of 2)
static int log2_pow2(int n) {
    int result = 0;
    while (n > 1) {
        n >>= 1;
        result++;
    }
    return result;
}

int tournament_create(int processes) {
    // Validation: processes must be a power of 2 and <= 16
    if (!is_power_of_two(processes) || processes > 16) {
        return -1;
    }
    
    // Calculate the number of levels in the tournament tree
    num_levels = log2_pow2(processes);
    
    // Total number of locks needed = processes - 1 (internal nodes)
    int total_locks = processes - 1;
    
    // Allocate memory for the lock array
    lock_ids = malloc(total_locks * sizeof(int));
    if (lock_ids == 0) {
        return -1;
    }
    
    // Create all Peterson locks needed for the tournament
    for (int i = 0; i < total_locks; i++) {
        lock_ids[i] = peterson_create();
        if (lock_ids[i] < 0) {
            // Failed to create a lock
            return -1;
        }
    }
    
    // Fork the processes
    process_index = 0;  // Start with index 0 for the parent
    num_processes = processes;
    
    // Fork (processes-1) times to create children
    for (int i = 1; i < processes; i++) {
        int pid = fork();
        if (pid < 0) {
            // Fork failed
            return -1;
        } else if (pid == 0) {
            // Child process
            process_index = i;
            break;  // Stop forking in child processes
        }
    }
    
    // Allocate memory for tracking acquired locks and roles
    acquired_locks = malloc(num_levels * sizeof(int));
    acquired_roles = malloc(num_levels * sizeof(int));
    if (acquired_locks == 0 || acquired_roles == 0) {
        return -1;
    }
    
    return process_index;
}

int tournament_acquire(void) {
    if (process_index < 0 || lock_ids == 0 || acquired_locks == 0 || acquired_roles == 0) {
        return -1;  // Tournament not created or invalid state
    }
    
    num_acquired = 0;  // Reset the number of acquired locks
    
    // Go through each level of the tournament, bottom-up
    for (int level = num_levels - 1; level >= 0; level--) {
        // Calculate the role at this level
        // Extract the bit at position (num_levels - level - 1) from process_index
        int role = (process_index & (1 << (num_levels - level - 1))) >> (num_levels - level - 1);
        
        // Calculate the lock index at this level
        int lock_index_in_level = process_index >> (num_levels - level);
        
        // Convert to the actual array index using the formula from assignment
        // index = lock_index_in_level + (2^level - 1)
        int array_index = lock_index_in_level + ((1 << level) - 1);
        
        // Acquire the lock
        if (peterson_acquire(lock_ids[array_index], role) < 0) {
            // Failed to acquire lock, release any we've acquired
            tournament_release();
            return -1;
        }
        
        // Record that we acquired this lock and the role we used
        acquired_locks[num_acquired] = array_index;
        acquired_roles[num_acquired] = role;
        num_acquired++;
    }
    
    return 0;  // Successfully acquired all locks
}

int tournament_release(void) {
    if (process_index < 0 || lock_ids == 0 || acquired_locks == 0 || acquired_roles == 0) {
        return -1;  // Tournament not created or invalid state
    }
    
    // Release locks in reverse order of acquisition (top-down)
    for (int i = num_acquired - 1; i >= 0; i--) {
        int array_index = acquired_locks[i];
        int role = acquired_roles[i];
        
        // Release the lock
        if (peterson_release(lock_ids[array_index], role) < 0) {
            return -1;  // Failed to release lock
        }
    }
    
    num_acquired = 0;  // Reset the counter
    
    return 0;  // Successfully released all locks
}