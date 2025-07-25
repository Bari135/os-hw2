#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

/* Added for Task 1 - Peterson Lock System Call Implementations */

uint64
sys_peterson_create(void)
{
  return peterson_create_impl();
}

uint64
sys_peterson_acquire(void)
{
  int lock_id;
  int role;

  argint(0, &lock_id);
  argint(1, &role);
  return peterson_acquire_impl(lock_id, role);
}

uint64
sys_peterson_release(void)
{
  int lock_id;
  int role;

  argint(0, &lock_id);
  argint(1, &role);
  return peterson_release_impl(lock_id, role);
}

uint64
sys_peterson_destroy(void)
{
  int lock_id;

  argint(0, &lock_id);
  return peterson_destroy_impl(lock_id);
}