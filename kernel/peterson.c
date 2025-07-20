/* Added for Task 1 - Peterson Lock Implementation */
#include "types.h"
#include "param.h"   
#include "memlayout.h"
#include "riscv.h" 
#include "spinlock.h"
#include "defs.h"
#include "peterson.h"
#include "proc.h"

struct petlock peterson_locks[NPETLOCK];

static inline int
valid_role(int r){ return r == ROLE0 || r == ROLE1; }

static inline int
valid_lockid(int id){ return id >= 0 && id < NPETLOCK; }

void
petersoninit(void)
{
  for(int i = 0; i < NPETLOCK; i++){
    peterson_locks[i].active   = 0;
    peterson_locks[i].flag[0]  = 0;
    peterson_locks[i].flag[1]  = 0;
    peterson_locks[i].turn     = 0;
  }
}

// Create a new Peterson lock
int
peterson_create_impl(void)
{
  for(int i = 0; i < NPETLOCK; i++){
    // Atomically claim a free slot
    if(__sync_lock_test_and_set(&peterson_locks[i].active, 1) == 0){
      __sync_synchronize();          // Make sure rest of struct is visible
      return i;
    }
  }
  return -1;
}

// Acquire the Peterson lock
int
peterson_acquire_impl(int lock_id, int role)
{
  if(!valid_lockid(lock_id) || !valid_role(role) || !peterson_locks[lock_id].active)
    return -1;

  struct petlock *l = &peterson_locks[lock_id];
  int other = role ^ 1;

  // Peterson protocol with yield
  __sync_lock_test_and_set(&l->flag[role], 1);
  __sync_lock_release(&l->turn);          // Reset turn
  l->turn = other;                        // Set turn to other process
  __sync_synchronize();

  while(l->flag[other] && l->turn == other){
    yield();                              // Give CPU up instead of busy wait
    
    // Check if lock was destroyed while waiting
    if(!peterson_locks[lock_id].active)
      return -1;
    
    __sync_synchronize();                 // Reload shared fields
  }
  return 0;
}

// Release the Peterson lock
int
peterson_release_impl(int lock_id, int role)
{
  if(!valid_lockid(lock_id) || !valid_role(role) || !peterson_locks[lock_id].active)
    return -1;

  struct petlock *l = &peterson_locks[lock_id];
  __sync_lock_release(&l->flag[role]);    // flag[role] = 0 (atomic)
  __sync_synchronize();
  return 0;
}

// Destroy the Peterson lock
int
peterson_destroy_impl(int lock_id)
{
  if(!valid_lockid(lock_id) || !peterson_locks[lock_id].active) return -1;

  struct petlock *l = &peterson_locks[lock_id];
  // Clean up both flags before marking inactive
  __sync_lock_release(&l->flag[0]);
  __sync_lock_release(&l->flag[1]);
  __sync_synchronize();
  l->active = 0;
  return 0;
}