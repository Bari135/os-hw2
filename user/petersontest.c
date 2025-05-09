#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int lock_id = peterson_create();
  if (lock_id < 0) {
    printf("Failed to create lock\n");
    exit(1);
  }
  
  printf("Created lock with ID: %d\n", lock_id);
  
  int fork_ret = fork();
  int role = fork_ret > 0 ? 0 : 1;
  
  for (int i = 0; i < 10; i++) {
    if (peterson_acquire(lock_id, role) < 0) {
      printf("Failed to acquire lock\n");
      exit(1);
    }
    
    // Critical section
    printf("Process %d (role %d) entered critical section, iteration %d\n", 
           getpid(), role, i);
    
    // Sleep to give the other process a chance to run
    sleep(1);
    
    printf("Process %d (role %d) leaving critical section, iteration %d\n", 
           getpid(), role, i);
    
    if (peterson_release(lock_id, role) < 0) {
      printf("Failed to release lock\n");
      exit(1);
    }
    
    // Sleep a bit before trying to acquire the lock again
    sleep(1);
  }
  
  if (fork_ret > 0) {
    wait(0);
    printf("Parent process destroying lock\n");
    if (peterson_destroy(lock_id) < 0) {
      printf("Failed to destroy lock\n");
      exit(1);
    }
  }
  
  exit(0);
}