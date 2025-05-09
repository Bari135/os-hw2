#include "types.h"

// Define roles
#define ROLE0    0
#define ROLE1    1
#define NPETLOCK 15   // Number of Peterson locks

// Renamed struct and fields
struct petlock {
  int active;        // Is the lock active/created?
  int flag[2];       // Flag fields (renamed from want)
  int turn;          // Whose turn is it to wait (0 or 1)
};

// Keep same function prototypes
void            petersoninit(void);
int             peterson_create_impl(void);
int             peterson_acquire_impl(int lock_id, int role);
int             peterson_release_impl(int lock_id, int role);
int             peterson_destroy_impl(int lock_id);
