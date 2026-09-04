#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "sysinfo.h"


uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
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
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
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

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
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

  if(argint(0, &pid) < 0)
    return -1;
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

uint64 sys_trace(void){
  int n;
  if(argint(0, &n) < 0)
    return -1;
  myproc()->trace_mask = n;
  return 0;
}

uint64 sys_sysinfo(void){
  //The kernel should fill out the fields of this struct: the freemem field should be set to the number of bytes of free memory, 
  //and the nproc field should be set to the number of processes whose state is not UNUSED.
  //So this is Kernel memory at this point.
  struct sysinfo st_struct;
  uint64 stuservirtual; // user pointer to struct stat, essentially mapping to the supplied userspace pointer.

  if(argaddr(0, &stuservirtual) < 0)
    return -1;

  //where is the user supplied address? 
  st_struct.freemem = freemem();
  st_struct.nproc = nproc();

  //we need to sort of fill out st with the process stuff.
  //we need to copy out st to userspace (specifically to the curr process' page table and virtual address)
  if(copyout(myproc()->pagetable, stuservirtual, (char *)&st_struct, sizeof(st_struct)) < 0)
  return -1;
  return 0;
}