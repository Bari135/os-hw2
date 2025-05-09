#include "types.h"

struct petersonlock {
  uint active;          // Is the lock active/created?
  uint want[2];         // Want flags (1 means the process wants to enter critical section)
  uint turn;            // Whose turn is it to wait (0 or 1)
  // Debug info
  char *name;           // Name of the lock
};

#define NPETERSON 16    // Number of Peterson locks in the system

void            petersoninit(void);
int             peterson_create_impl(void);
int             peterson_acquire_impl(int lock_id, int role);
int             peterson_release_impl(int lock_id, int role);
int             peterson_destroy_impl(int lock_id);
