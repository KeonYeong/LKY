#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"
#include "elf.h"
#define NQUEUES 3                            // 큐 개수
#define LARGENO 0xFFFFFFFF                  // minpass의 비교를 위한 큰 수
#define THRSHARE 1000


struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;

typedef struct MLFQ {                          //mlfq 구조체
    int maxtime;
    int front;
    int count;
    struct proc *procarr[NPROC];
}MLFQ;

MLFQ mlfq[NQUEUES];                       //구조체 배열로 3개 선언

void enqueue(int lev, struct proc *proc){           // 큐에 넣는 enqueue함수
    if(mlfq[lev].count == NPROC) {
        panic("full");
    }
    int i = (mlfq[lev].front + mlfq[lev].count) % NPROC;
    mlfq[lev].procarr[i] = proc;
    mlfq[lev].count++;
}

void push(int lev, struct proc *proc){            // 큐의 앞에넣어서 먼저 실행되게끔 하는 push함수
    if(mlfq[lev].count == NPROC){
        panic ("full");
    }
    int i = (mlfq[lev].front + NPROC -1) % NPROC ;
    mlfq[lev].procarr[i] = proc;
    mlfq[lev].count++;
    mlfq[lev].front = i;
}

struct proc* dequeue (int lev){                   // 큐에서 하나 빼는 dequeue함수
    if (mlfq[lev].count == 0){
        panic ("empty");
    }
    struct proc *temp = mlfq[lev].procarr[mlfq[lev].front];
    mlfq[lev].front = (mlfq[lev].front + 1) % NPROC;
    mlfq[lev].count--;
    return temp;
}

void init_queues(){          //큐 초기화 함수
    mlfq[0].maxtime = 5;
    mlfq[1].maxtime = 10;
    mlfq[2].maxtime = 20;
    int i;
    for(i=0; i < NQUEUES; i ++){
        mlfq[i].count = 0;
        mlfq[i].front = 0;
    }
}

static struct proc *initproc;

int nextpid = 1;
int threadCreating = 0;
extern void forkret(void);
extern void trapret(void);

static void wakeup1(void *chan);

void
pinit(void)
{
  initlock(&ptable.lock, "ptable");
}

//PAGEBREAK: 32
// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.
static struct proc*
allocproc(void)
{
  struct proc *p;
  char *sp;
  int i;
  acquire(&ptable.lock);

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == UNUSED)
      goto found;

  release(&ptable.lock);
  return 0;

found:
  p->state = EMBRYO;
  if(!threadCreating){
      p->pid = nextpid++;
      p->isThread = 0;
      p->thrFork = 0;
      p->numThread = 0;
      for(i=0;i<64;i++){
          p->spaceValid[i] = 0;
      }
  }
  p->num_ticks = 0;
  p->priority_no = 0;               // 프로세스가 새로 할당될 때마다 필요한 변수들의 초기화 부분
  p->stride = 0;
  p->stdsz = 0;
  p->passvalue = 0;
  p->isStride = 0;
  p->selfYield = 1;
  threadCreating = 0;
  release(&ptable.lock);

  // Allocate kernel stack.
  if((p->kstack = kalloc()) == 0){
    p->state = UNUSED;
    return 0;
  }
  sp = p->kstack + KSTACKSIZE;

  // Leave room for trap frame.
  sp -= sizeof *p->tf;
  p->tf = (struct trapframe*)sp;

  // Set up new context to start executing at forkret,
  // which returns to trapret.
  sp -= 4;
  *(uint*)sp = (uint)trapret;

  sp -= sizeof *p->context;
  p->context = (struct context*)sp;
  memset(p->context, 0, sizeof *p->context);
  p->context->eip = (uint)forkret;

  return p;
}

//PAGEBREAK: 32
// Set up first user process.
void
userinit(void)
{
  struct proc *p;
  extern char _binary_initcode_start[], _binary_initcode_size[];
  init_queues();
  mlfqStride = 0;                       // 최초 xv6의 초기화 시 필요한 변수들의 초기화
  mlfqPassvalue = 0;
  modeStride = 0;
  fromStride = 0;
  p = allocproc();
  acquire(&ptable.lock);
  initproc = p;
  if((p->pgdir = setupkvm()) == 0)
    panic("userinit: out of memory?");
  inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
  p->sz = PGSIZE;
  memset(p->tf, 0, sizeof(*p->tf));
  p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
  p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
  p->tf->es = p->tf->ds;
  p->tf->ss = p->tf->ds;
  p->tf->eflags = FL_IF;
  p->tf->esp = PGSIZE;
  p->tf->eip = 0;  // beginning of initcode.S

  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = namei("/");

  // this assignment to p->state lets other cores
  // run this process. the acquire forces the above
  // writes to be visible, and the lock is also needed
  // because the assignment might not be atomic
  p->state = RUNNABLE;
  enqueue(0, p);            //일단 큐에 넣어주기
  release(&ptable.lock);
}

// Grow current process's memory by n bytes.
// Return 0 on success, -1 on failure.
int
growproc(int n)
{
  uint sz;
//  uint i;
  acquire(&ptable.lock);
  if(proc->isThread) sz = proc->parent->sz;
  else sz = proc->sz;
  if(proc->isThread){
      if(n > 0){
    if((sz = allocuvm(proc->pgdir, sz + PGSIZE * 64, sz + PGSIZE * 64 +  n)) == 0)
      return -1;
  } else if(n < 0){
    if((sz = deallocuvm(proc->pgdir, sz + PGSIZE * 64, sz + PGSIZE * 64 +  n)) == 0)
      return -1;
  }
  proc->parent->sz = sz - PGSIZE * 64;
  }
  else if ((proc->numThread == 0) && (proc->stdsz == proc->sz)){
        if(n > 0){
    if((sz = allocuvm(proc->pgdir, sz, sz +  n)) == 0)
      return -1;
  } else if(n < 0){
    if((sz = deallocuvm(proc->pgdir, sz, sz +  n)) == 0)
      return -1;
  }
  proc->sz = sz;
  proc->stdsz = sz;
  }
  else {
        if(n > 0){
    if((sz = allocuvm(proc->pgdir, sz + PGSIZE * 64, sz + PGSIZE * 64 +  n)) == 0)
      return -1;
  } else if(n < 0){
    if((sz = deallocuvm(proc->pgdir, sz + PGSIZE * 64, sz + PGSIZE * 64 +  n)) == 0)
      return -1;
  }
  proc->sz = sz - PGSIZE * 64;
}
  release(&ptable.lock);
  switchuvm(proc);
  return 0;
}

// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
int
fork(void)
{
  int i, pid;
  struct proc *np;
  // Allocate process.
  if((np = allocproc()) == 0){
    return -1;
  }
  if(proc->isThread) np->thrFork = 1;
  // Copy process state from p.
  if((np->pgdir = copyuvm(proc->pgdir, proc->sz)) == 0){
    kfree(np->kstack);
    np->kstack = 0;
    np->state = UNUSED;
    return -1;
  }
  np->sz = proc->sz;
  np->stdsz = proc->sz;
  if(proc->isThread){
      np->parent = proc->parent;
      np->thrParent = proc;
  }
  else np->parent = proc;
  *np->tf = *proc->tf;

  // Clear %eax so that fork returns 0 in the child.
  np->tf->eax = 0;

  for(i = 0; i < NOFILE; i++)
    if(proc->ofile[i])
      np->ofile[i] = filedup(proc->ofile[i]);
  np->cwd = idup(proc->cwd);
  pid = np->pid;
  np->state = RUNNABLE;
  np->num_ticks = 0;
  np->priority_no =0;
  np->selfYield = 0;
  enqueue(0, np);
  safestrcpy(np->name, proc->name, sizeof(proc->name));
  return pid;
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.
void
exit(void)
{
  struct proc *p;
  int fd;
  if(proc == initproc)
    panic("init exiting");
  if(proc->isThread){
        for(fd = 0; fd < NOFILE; fd++){
         if(proc->parent->ofile[fd]){
             fileclose(proc->parent->ofile[fd]);
             proc->parent->ofile[fd] = 0;
              }
            }
         begin_op();
         iput(proc->parent->cwd);
         end_op();
         proc->parent->cwd = 0;
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if((p->parent == proc->parent) && p->isThread && p!=proc){
             for(fd = 0; fd < NOFILE; fd++){
                if(p->ofile[fd]){
                  fileclose(p->ofile[fd]);
                  p->ofile[fd] = 0;
                }
              }
              begin_op();
              iput(p->cwd);
              end_op();
              p->cwd = 0;
      }
  }
  }
  // Close all open files.
  else{
         for(p=ptable.proc; p < &ptable.proc[NPROC]; p++){
             if((p->parent == proc) && p->isThread){
         for(fd = 0; fd < NOFILE; fd++){
            if(p->ofile[fd]){
              fileclose(p->ofile[fd]);
              p->ofile[fd] = 0;
            }
          }
          begin_op();
          iput(p->cwd);
          end_op();
          p->cwd = 0;
             }
         }
  }
  for(fd = 0; fd < NOFILE; fd++){
    if(proc->ofile[fd]){
      fileclose(proc->ofile[fd]);
      proc->ofile[fd] = 0;
    }
  }

  begin_op();
  iput(proc->cwd);
  end_op();
  proc->cwd = 0;
  acquire(&ptable.lock);
  if(proc->isThread) {
      wakeup1(proc->parent->parent);}
  else if (proc->thrFork) wakeup1(proc->thrParent);
  else wakeup1(proc->parent);

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(proc->isThread){
        if((p->parent == proc->parent)&&p->isThread &&p!=proc){
            p->parent->numThread--;
            p->state = ZOMBIE;
            p->isZombie = 1;
        }
    }
    if(p->parent == proc){
      if(p->isThread){
              p->parent->numThread--;
              p->state = ZOMBIE;
              p->isZombie = 1;
      }
      else {
          p->parent = initproc;
          if(p->state == ZOMBIE) wakeup1(initproc);
      }
    }
  }

  // Jump into the scheduler, never to return.
  if(proc->isThread) {
      proc->parent->state = ZOMBIE;
      proc->isZombie = 1;
  }
  proc->state = ZOMBIE;
  proc->num_ticks = 0;
  proc->priority_no = 0;               // 프로세스가 새로 할당될 때마다 필요한 변수들의 초기화 부분
  proc->stride = 0;
  proc->passvalue = 0;
  proc->isStride = 0;
  proc->selfYield = 1;
  sched();
  panic("zombie exit");
}

void orphanMake(){
    struct proc * p;
    acquire(&ptable.lock);
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
        if(proc->isThread){
            if((p->parent == proc->parent)&&p->isThread &&p!=proc){
                p->state = ZOMBIE;
                p->isZombie = 1;
            }
        }
    }
    release(&ptable.lock);
}
// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int
wait(void)
{
  struct proc *p;
  int havekids, pid;
  //int i;
  acquire(&ptable.lock);
  for(;;){
    // Scan through table looking for exited children.
    havekids = 0;
    for(p=ptable.proc; p<&ptable.proc[NPROC]; p++){
         if(p->isZombie && p->isThread && p->state == ZOMBIE){
              deallocuvm(p->pgdir, p->stack, p->stack - PGSIZE);
              p->parent->spaceValid[p->threadID] = 0;
              p->parent->numThread --;
              kfree(p->kstack);
              p->kstack = 0;
              p->isZombie = 0;
              p->name[0] = 0;
              p->killed = 0;
              p->parent = 0;
              p->num_ticks = 0;
              p->priority_no = 0;               // 프로세스가 새로 할당될 때마다 필요한 변수들의 초기화 부분
              p->stride = 0;
              p->passvalue = 0;
              p->isStride = 0;
              p->selfYield = 1;
              p->state = UNUSED;
         }
        }
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->parent != proc && !p->thrFork)
          continue;
      havekids = 1;
      if(p->state == ZOMBIE && !p->isThread){
        pid = p->pid;
        kfree(p->kstack);
        p->kstack = 0;
        freevm(p->pgdir);
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        p->num_ticks = 0;
        p->priority_no = 0;               // 프로세스가 새로 할당될 때마다 필요한 변수들의 초기화 부분
        p->stride = 0;
        p->passvalue = 0;
        p->isStride = 0;
        p->selfYield = 1;
        p->state = UNUSED;
        release(&ptable.lock);
        return pid;
      }

    }

    // No point waiting if we don't have any children.
    if(!havekids || proc->killed){
        release(&ptable.lock);
      return -1;
    }
    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
    sleep(proc, &ptable.lock);  //DOC: wait-sleep
  }
}

//PAGEBREAK: 42
// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.
void
scheduler(void)
{ 
  struct proc *p;        // 필요한 변수들의 초기화 부분
  double minpass;
  int minindex;
  int finder;
  double stridesum;
  int level;
  double sharesum;
  for(;;){
    sti();
    acquire(&ptable.lock);
    if(modeStride) {    // set_cpu_share가 발생했을 경우 stride로 전환되어 아래 코드가 실행하게 된다.
    minpass = LARGENO;
    stridesum = 0;
    sharesum = 0;
    minindex = 0;
    fromStride = 1; // 이 변수는 타임인터럽트 트랩에서 mlfq의 ticks의 증가 여부를 통제해준다.
    for (finder = 0 ; finder < NPROC ; finder++){ 
        if(ptable.proc[finder].state != RUNNABLE)  //ptable에서 아직 실행 가능한 프로세스들을 탐색하여
            continue;
        else{
            if(ptable.proc[finder].isStride){   // 내부에 stride가 존재할 경우 최소 pass value를 찾는다.
            p = &ptable.proc[finder];
            if(p->passvalue < minpass){
                minpass = p->passvalue;
                minindex = finder;
            }
            stridesum = stridesum + p->stride;                                  //동시에 mlfq로의 전환이나 mlfq의 stride도 구해주기 위하여
            if (p->stride != 0) sharesum = sharesum + (STRIDENO / p->stride);    //share과 stride의 총합을 각각 구한다.
        }
        }
    }

    if(stridesum == 0){   // 만일 stride로 돌아야 할 프로세스가 없을 시 stride를 멈추고 계속 mlfq로 돌리게 된다.i
        mlfqStride = 0 ;
        mlfqPassvalue = 0;
        modeStride = 0;
        goto MLFQ;     // mlfq라벨을 만들어서 필요 시마다 점프를 해준다.
    }
    if(mlfqPassvalue < minpass){     // mlfq의 passvalue가 더 낮을 시 mlfq를 실행
        mlfqStride = STRIDENO / (100 - sharesum);
        goto MLFQ;
    }
    else {                              // 무난히 pass value가 낮은 프로세스를 찾으면 그 프로세스를 실행
        p = &ptable.proc[minindex];
        proc = p;
        switchuvm(p);
        p->state = RUNNING;
        swtch(&cpu->scheduler, p->context);
        switchkvm();

        p->passvalue = p->passvalue + p->stride;
        proc = 0;
    }
    }
    else {                          //modeStride가 0일 경우에(stride scheduling이 아니면) 계속 mlfq로 돌리게 된다.
MLFQ:                           //stride와의 연계 때문에 MLFQ: 라벨을 구성했다.
    fromStride = 0;             //mlfq 실행할 때 마다 trap의 ticks를 증가시켜야 하기 때문에 fromStride가 아니라는 뜻으로 0을 준다.
    for(level =0; level < NQUEUES; level++){      // 큐의 레벨별로 반복문을 돌린다.
        if (mlfq[level].count == 0){            // 큐가 빌 경우 다음 레벨 큐로 간다.
            continue;
        }
        p = dequeue(level);              //실행시킬 프로세스를 찾았을 경우 
        if (p->state != RUNNABLE)
            continue;
        if (p->isStride)                // 실행가능한지 여부와 stride를 쓰겠다고 선언한 프로세스인지 검사하여 맞다면 다음 프로세스를 실행시키게 된다.
            continue;           
      p->selfYield = 1;
      proc = p;                     // 그 후 해당 프로세스를 실행!
      switchuvm(p);
      p->state = RUNNING;
      swtch(&cpu->scheduler, p->context);
      switchkvm();
      if (p->num_ticks >= mlfq[level].maxtime){             // 실행이 완료되고 scheduler로 돌아오면 이제 프로세스의 tick을 보고 큐의 타임 슬라이스를 초과했는지
          p->num_ticks = 0;                                 // 여부를 따지게 된다 그리하여 만약 초과하게 됬다면 priority를 낮추게 된다.
          if (p->priority_no != 2){
              p->priority_no++;
          }
          if(p->state == RUNNABLE) {                    // 아직 실행 가능한 경우 다시 큐의 뒤에 넣어준다.
              enqueue(p->priority_no, p);
          }
      }
      else {
          if (p->state == RUNNABLE){
              if (p->selfYield){                          // time slice가 끝나기 전에
                  enqueue(p->priority_no, p);        //   time interrupt 때문이 아닌 자체적으로 yield한 경우, 큐의 맨뒤에 다시 프로세스를 넣어준다.
              }
              else {
                  push(p->priority_no, p);              // 그게 아닌 그냥 time interrupt때문에 yield한 경우, 그 프로세스는 아직 주어진 타임슬라이스를 돌고 있는것이기에
              }                                         // 큐의 맨 앞에 넣어서 바로 프로세스를 다시 돌리게 된다.
              }                                  
      }
      if (boost_timer >= 100){                          // boosting부분 : tick이 100이 되면 모든 레벨 큐의 모든 프로세스를 0레벨 까지 끌어올린다.
          boost_timer = 0;
          int i;
          for (i=0; i < NQUEUES; i++){
                  int j;
                  for(j = 0 ; j < mlfq[i].count ; j++){
                      struct proc *temp2 = dequeue(i);
                      temp2->priority_no = 0;
                      temp2->num_ticks = 0;
                      enqueue(0, temp2);
                  }
          }
      }
      proc = 0;
    }
    if(modeStride) {
        mlfqPassvalue = mlfqPassvalue + mlfqStride;             //MLFQ가 stride scheduling중 실행 된 경우, mlfq의 passvalue 를 증가시켜 준다.
    }
    }
    release(&ptable.lock);
  }
}

void
sched(void)
{
  int intena;

  if(!holding(&ptable.lock))
    panic("sched ptable.lock");
  if(cpu->ncli != 1)
    panic("sched locks");
  if(proc->state == RUNNING)
    panic("sched running");
  if(readeflags()&FL_IF)
    panic("sched interruptible");
  intena = cpu->intena;
  swtch(&proc->context, cpu->scheduler);
  cpu->intena = intena;
}

// Give up the CPU for one scheduling round.
void
yield(void)
{
  acquire(&ptable.lock);  //DOC: yieldlock
  proc->state = RUNNABLE;
  sched();
  release(&ptable.lock);
}
int
yield2(void)                 // 자체적으로 만든 yield 시스템콜
{
  acquire(&ptable.lock);  //DOC: yieldlock
  proc->state = RUNNABLE;
  sched();
  release(&ptable.lock);
  return 0xABCDABCD;
}


int
sys_yield(void)                 // 위에 함수의 wrapper function
{
   return yield2();
}
// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void
forkret(void)
{
  static int first = 1;
  // Still holding ptable.lock from scheduler.
  release(&ptable.lock);

  if (first) {
    // Some initialization functions must be run in the context
    // of a regular process (e.g., they call sleep), and thus cannot
    // be run from main().
    first = 0;
    iinit(ROOTDEV);
    initlog(ROOTDEV);
  }

  // Return to "caller", actually trapret (see allocproc).
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void
sleep(void *chan, struct spinlock *lk)
{
  if(proc == 0)
    panic("sleep");

  if(lk == 0)
    panic("sleep without lk");

  // Must acquire ptable.lock in order to
  // change p->state and then call sched.
  // Once we hold ptable.lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup runs with ptable.lock locked),
  // so it's okay to release lk.
  if(lk != &ptable.lock){  //DOC: sleeplock0
    acquire(&ptable.lock);  //DOC: sleeplock1
    release(lk);
  }

  // Go to sleep.
  proc->chan = chan;
  proc->state = SLEEPING;
  sched();

  // Tidy up.
  proc->chan = 0;

  // Reacquire original lock.
  if(lk != &ptable.lock){  //DOC: sleeplock2
    release(&ptable.lock);
    acquire(lk);
  }
}

//PAGEBREAK!
// Wake up all processes sleeping on chan.
// The ptable lock must be held.
static void
wakeup1(void *chan)
{
  struct proc *p;

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == SLEEPING && p->chan == chan){
      p->state = RUNNABLE;
      p->priority_no = 0;
      p->num_ticks = 0;
      p->passvalue = 0;
      p->stride = 0;
      p->isStride = 0;
      enqueue(p->priority_no, p);
    }
}

// Wake up all processes sleeping on chan.
void
wakeup(void *chan)
{
  acquire(&ptable.lock);
  wakeup1(chan);
  release(&ptable.lock);
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
int
kill(int pid)
{
  struct proc *p;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->pid == pid && !p->isThread){
        p->killed = 1;
      // Wake process from sleep if necessary.
      if(p->state == SLEEPING){
        p->state = RUNNABLE;
        p->passvalue = 0;
        p->stride = 0;
        p->isStride = 0;
        enqueue(p->priority_no, p);
        }
      release(&ptable.lock);
      return 0;
    }
  }
  release(&ptable.lock);
  return -1;
}

//PAGEBREAK: 36
// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void
procdump(void)
{
  static char *states[] = {
  [UNUSED]    "unused",
  [EMBRYO]    "embryo",
  [SLEEPING]  "sleep ",
  [RUNNABLE]  "runble",
  [RUNNING]   "run   ",
  [ZOMBIE]    "zombie",
  };
  int i;
  struct proc *p;
  char *state;
  uint pc[10];
  for (p = ptable.proc ; p < &ptable.proc[NPROC]; p++){
      if(p->state == UNUSED)
          continue;
      if(p->state >= 0 && p -> state < NELEM(states) && states[p->state])
          state = states[p->state];
      else
          state = "???";
      cprintf("%d %s %s", p->pid, state, p->name);
      if(p->state == SLEEPING){
          getcallerpcs((uint*)p->context->ebp+2, pc);
          for(i=0; i<10 && pc[i] != 0; i++)
              cprintf(" %p", pc[i]);
      }
      cprintf("\n");
  }
}

int
set_cpu_share (int share)
{
    int i;
    int sharetotal = 0;
    double sharingStride;
    proc->shareSum = share;
    acquire(&ptable.lock);
    for(i=0; i <NPROC; i++){
        if(ptable.proc[i].isStride && (ptable.proc[i].state == RUNNABLE)){
            if(ptable.proc[i].stride != 0) sharetotal = sharetotal + (STRIDENO / ptable.proc[i].stride);
        }
    }
    release(&ptable.lock);
    if((sharetotal + share) >80){
        cprintf("Request denied, share exceeding, %d + %d = %d > 80\n", sharetotal, share, sharetotal + share);
        return -1;
    }
    else{
        if(proc->numThread > 0){
            sharingStride = STRIDENO / ((double)share / (proc->numThread + 1));
            acquire(&ptable.lock);
            for(i=0; i < NPROC; i ++){
                if((ptable.proc[i].state == RUNNABLE) && (ptable.proc[i].pid == proc->pid) && (ptable.proc[i].isThread)){
                    ptable.proc[i].stride = sharingStride;
                    ptable.proc[i].passvalue = 0;
                    ptable.proc[i].isStride = 1;
                }
            }
            release(&ptable.lock);
            proc->stride = sharingStride;
        }
        else proc->stride = STRIDENO / (double)share;
        proc->passvalue = 0;
        proc->isStride = 1;
        modeStride = 1;
    }
    return 0;
}

int
sys_set_cpu_share(void)         // 위에 함수의 wrapper function
{
    int share;
    if(argint(0, &share) <0)
        return -1;
    return set_cpu_share(share);
}

int
thread_create(thread_t * thread, void * (*start_routine)(void *), void *arg){
  int i;
  struct proc *np;
  double sharingStride;
  // Allocate process.
  threadCreating = 1;
  if((np = allocproc()) == 0){
    return -1;
  }
  for(i = 0 ; i < 64 ; i++){
      if(proc->spaceValid[i] == 0){
          np->threadID = i;
          proc->spaceValid[i] = np->threadID + 1;
          break;
      }
  }
  *thread = np->threadID;
  np->sz = proc->stdsz;
  np->pid = proc->pid;
  if(proc->isThread) np ->pid = proc->parent->pid;
  if(proc->isThread) np->sz = proc->parent->stdsz;
  np->parent = proc;
  if(proc->isThread) np->parent = proc->parent;
  np->isThread = 1;
  np->isZombie = 0;
  if(proc->isThread) proc->parent->numThread++;
  else proc->numThread ++;
  uint tmpi;
  if(proc->isThread) tmpi = proc->parent->stdsz + PGSIZE * (np->threadID);
  else tmpi = proc->stdsz + PGSIZE * (np->threadID);
  np->stack = tmpi + PGSIZE;
  *np->tf = *proc->tf;
  np->tf->eax = 0;
  np->pgdir = proc->pgdir;
  if(proc->isThread) np->pgdir = proc->parent->pgdir;
  np->tf->eip = (uint)(start_routine); // 함수 시작 조절
  np->sz = allocuvm(np->pgdir, np->stack - PGSIZE, np->stack);
  np->tf->esp = np->stack - PGSIZE + 4092;
  *((uint*)(np->tf->esp)) =  (uint)arg;
  *((uint*)(np->tf->esp-4)) =  0xFFFFFFFF;
  np->tf->esp -= 4;
  // Clear %eax so that fork returns 0 in the child.
  for(i = 0; i < NOFILE; i++)
    if(proc->ofile[i])
      np->ofile[i] = filedup(proc->ofile[i]);
  np->cwd = idup(proc->cwd);
  acquire(&ptable.lock);
  np->state = RUNNABLE;
  release(&ptable.lock);
  np->num_ticks = 0;
  np->priority_no =0;
  np->selfYield = 0;
  if(proc->isStride){
         sharingStride = STRIDENO / ((double)proc->shareSum / (proc->numThread + 1));
            acquire(&ptable.lock);
            for(i=0; i < NPROC; i ++){
                if((ptable.proc[i].state == RUNNABLE) && (ptable.proc[i].pid == proc->pid) && (ptable.proc[i].isThread)){
                    ptable.proc[i].stride = sharingStride;
                    ptable.proc[i].passvalue = 0;
                    ptable.proc[i].isStride = 1;
                }
            }
            release(&ptable.lock);
            proc->stride = sharingStride;
      }
  enqueue(0, np);
  safestrcpy(np->name, proc->name, sizeof(proc->name));
  return 0;
}

int
sys_thread_create(void)         // 위에 함수의 wrapper function
{
    thread_t* procptr;
    void *funcptr;
    void * arptr;
    int tmp;
    if(argint(0, &tmp) <0)
        return -1;
    procptr = (thread_t*) tmp;
    if(argint(1, &tmp)<0)
        return -1;
    funcptr = (void*)tmp;
    if(argint(2, &tmp)<0)
        return -1;
    arptr = (void*)tmp;
    return thread_create(procptr, funcptr, arptr);
}

int
thread_join(thread_t thread, void **retval){
    struct proc *p;
  int havekids ;
  //int i;
  acquire(&ptable.lock);
  for(;;){
    // Scan through table looking for exited children.
    havekids = 0;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->parent != proc || p->threadID != thread)
        continue;
      havekids = 1;
      if(p->state == ZOMBIE && p->isThread){
        deallocuvm(p->pgdir, p->stack, p->stack - PGSIZE);
        proc->spaceValid[thread] = 0;
        *retval = proc->ret[thread];
        kfree(p->kstack);
        p->kstack = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        p->isThread = 0;
        p->num_ticks = 0;
        p->priority_no = 0;               // 프로세스가 새로 할당될 때마다 필요한 변수들의 초기화 부분
        p->stride = 0;
        p->passvalue = 0;
        p->isStride = 0;
        p->selfYield = 1;
        p->state = UNUSED;
        release(&ptable.lock);
        return 0;
      }
    }

    // No point waiting if we don't have any children.
    if(!havekids || proc->killed){
        release(&ptable.lock);
      return -1;
    }

    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
    sleep(proc, &ptable.lock);  //DOC: wait-sleep
  }
}


int
sys_thread_join(void)
{
    thread_t thread;
    int tmp;
    void **retval;
    if(argint(0, &tmp) <0)
        return -1;
    thread = tmp;
    if(argint(1, &tmp)<0)
        return -1; 
    retval = (void**)tmp;
    return thread_join(thread, retval);
}
void
thread_exit(void *retval){
  int fd, i;
  double sharingStride;
  for(fd = 0; fd < NOFILE; fd++){
    if(proc->ofile[fd]){
      fileclose(proc->ofile[fd]);
      proc->ofile[fd] = 0;
    }
  }

  begin_op();
  iput(proc->cwd);
  end_op();
  proc->cwd = 0;
  proc->parent->numThread--;
  proc->state = ZOMBIE;
   if(proc->parent->isStride){
            sharingStride = STRIDENO / ((double)proc->parent->shareSum / (proc->parent->numThread + 1));
            acquire(&ptable.lock);
            for(i=0; i < NPROC; i ++){
                if((ptable.proc[i].state == RUNNABLE) && (ptable.proc[i].pid == proc->parent->pid) && (ptable.proc[i].isThread)){
                    ptable.proc[i].stride = sharingStride;
                    ptable.proc[i].passvalue = 0;
                    ptable.proc[i].isStride = 1;
                }
            }
            release(&ptable.lock);
            proc->parent->stride = sharingStride;
   }
  proc->parent->ret[proc->threadID] = retval;
  proc->num_ticks = 0;
  proc->priority_no = 0;               // 프로세스가 새로 할당될 때마다 필요한 변수들의 초기화 부분
  proc->stride = 0;
  proc->passvalue = 0;
  proc->isStride = 0;
  proc->selfYield = 1;
  acquire(&ptable.lock);
  wakeup1(proc->parent);
  sched();
  panic("thr zombie exit"); 
}

int
sys_thread_exit(void)
{
    void * retval;
    int tmp;
    if(argint(0, &tmp)<0)
        return -1;
    retval = (void*)tmp;
    thread_exit(retval);
    return 0xFFFFFFFF;
}
