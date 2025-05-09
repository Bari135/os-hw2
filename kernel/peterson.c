#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"
#include "peterson.h"

struct {
  struct spinlock lock;                // Protects access to the locks array
  struct petersonlock locks[NPETERSON]; // Array of Peterson locks
} ptable;

void
petersoninit(void)
{
  initlock(&ptable.lock, "petersonlocks");
  
  // Initialize all Peterson locks
  for(int i = 0; i < NPETERSON; i++) {
    ptable.locks[i].active = 0;
    ptable.locks[i].want[0] = 0;
    ptable.locks[i].want[1] = 0;
    ptable.locks[i].turn = 0;
    ptable.locks[i].name = "petersonlock";
  }
}

// Create a new Peterson lock
int
peterson_create_impl(void)
{
  int i;
  
  acquire(&ptable.lock);
  
  // Find an inactive lock
  for(i = 0; i < NPETERSON; i++) {
    if(ptable.locks[i].active == 0) {
      ptable.locks[i].active = 1;
      ptable.locks[i].want[0] = 0;
      ptable.locks[i].want[1] = 0;
      ptable.locks[i].turn = 0;
      release(&ptable.lock);
      return i;  // Return lock ID
    }
  }
  
  release(&ptable.lock);
  return -1;  // No free locks
}

// Acquire the Peterson lock
int
peterson_acquire_impl(int lock_id, int role)
{
  // Validate lock_id and role
  if(lock_id < 0 || lock_id >= NPETERSON || (role != 0 && role != 1))
    return -1;
  
  acquire(&ptable.lock);
  if(ptable.locks[lock_id].active == 0) {
    release(&ptable.lock);
    return -1;  // Lock not active
  }
  release(&ptable.lock);
  
  // Set want flag for this process
  __sync_synchronize();  // Memory barrier before accessing shared state
  ptable.locks[lock_id].want[role] = 1;
  
  // Set turn to other process
  ptable.locks[lock_id].turn = 1 - role;
  __sync_synchronize();  // Memory barrier after updating shared state
  
  // While the other process wants the lock and it's the other process's turn
  while(ptable.locks[lock_id].want[1 - role] && 
        ptable.locks[lock_id].turn == (1 - role)) {
    // Instead of busy-waiting, yield the CPU
    yield();
    
    // After returning from yield, we need to recheck if the lock is still active
    acquire(&ptable.lock);
    if(ptable.locks[lock_id].active == 0) {
      release(&ptable.lock);
      return -1;  // Lock was destroyed while waiting
    }
    release(&ptable.lock);
    
    __sync_synchronize();  // Memory barrier before checking shared state again
  }
  
  return 0;  // Lock acquired
}

// Release the Peterson lock
int
peterson_release_impl(int lock_id, int role)
{
  // Validate lock_id and role
  if(lock_id < 0 || lock_id >= NPETERSON || (role != 0 && role != 1))
    return -1;
  
  acquire(&ptable.lock);
  if(ptable.locks[lock_id].active == 0) {
    release(&ptable.lock);
    return -1;  // Lock not active
  }
  release(&ptable.lock);
  
  // Reset want flag for this process
  __sync_synchronize();  // Memory barrier before modifying shared state
  ptable.locks[lock_id].want[role] = 0;
  __sync_synchronize();  // Memory barrier after updating shared state
  
  return 0;
}

// Destroy the Peterson lock
int
peterson_destroy_impl(int lock_id)
{
  // Validate lock_id
  if(lock_id < 0 || lock_id >= NPETERSON)
    return -1;
  
  acquire(&ptable.lock);
  if(ptable.locks[lock_id].active == 0) {
    release(&ptable.lock);
    return -1;  // Lock not active
  }
  
  // Mark lock as inactive
  ptable.locks[lock_id].active = 0;
  release(&ptable.lock);
  
  return 0;
}