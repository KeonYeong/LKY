#define STRIDENO 1000
// Per-CPU state
struct cpu {
  uchar apicid;                // Local APIC ID
  struct context *scheduler;   // swtch() here to enter scheduler
  struct taskstate ts;         // Used by x86 to find stack for interrupt
  struct segdesc gdt[NSEGS];   // x86 global descriptor table
  volatile uint started;       // Has the CPU started?
  int ncli;                    // Depth of pushcli nesting.
  int intena;                  // Were interrupts enabled before pushcli?

  // Cpu-local storage variables; see below
  struct cpu *cpu;
  struct proc *proc;           // The currently-running process.
};

extern struct cpu cpus[NCPU];
extern int ncpu;

// Per-CPU variables, holding pointers to the
// current cpu and to the current process.
// The asm suffix tells gcc to use "%gs:0" to refer to cpu
// and "%gs:4" to refer to proc.  seginit sets up the
// %gs segment register so that %gs refers to the memory
// holding those two variables in the local cpu's struct cpu.
// This is similar to how thread-local variables are implemented
// in thread libraries such as Linux pthreads.
extern struct cpu *cpu asm("%gs:0");       // &cpus[cpunum()]
extern struct proc *proc asm("%gs:4");     // cpus[cpunum()].proc

//PAGEBREAK: 17
// Saved registers for kernel context switches.
// Don't need to save all the segment registers (%cs, etc),
// because they are constant across kernel contexts.
// Don't need to save %eax, %ecx, %edx, because the
// x86 convention is that the caller has saved them.
// Contexts are stored at the bottom of the stack they
// describe; the stack pointer is the address of the context.
// The layout of the context matches the layout of the stack in swtch.S
// at the "Switch stacks" comment. Switch doesn't save eip explicitly,
// but it is on the stack and allocproc() manipulates it.
struct context {
  uint edi;
  uint esi;
  uint ebx;
  uint ebp;
  uint eip;
};

enum procstate { UNUSED, EMBRYO, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };
// Per-process state
struct proc {
  uint sz;                     // Size of process memory (bytes)
  uint stdsz;                  // 스레드의 메모리 할당을 위해 사용되는 프로세스의 사이즈 기준, sz의 최초값이 들어가고 변하는 조건이 까다로우며 거의 고정이다.
  uint numThread;              // 프로세스가 가지고 있는 자식 스레드의 개수
  uint shareSum;               // 최초에 프로세스가 set_cpu_share로 요청한 share의 고정 값
  pde_t* pgdir;                // Page table
  //pde_t* thrpgdir;
  char *kstack;                // Bottom of kernel stack for this process
  uint stack;                 // Stack of Thread
  enum procstate state;        // Process state
  int pid;                     // Process ID
  struct proc *parent;         // Parent process
  struct proc *thrParent;      // 스레드가 fork()를 했을 경우 그 fork된 프로세스의 부모는 스레드의 부모가 되지만, exit에서 효율 있게 처리하기 위해 thrParent로 생성시킨 스레드를 넣는다.
  struct trapframe *tf;        // Trap frame for current syscall
  struct context *context;     // swtch() here to run process
  void *chan;                  // If non-zero, sleeping on chan
  int killed;                  // If non-zero, have been killed
  struct file *ofile[NOFILE];  // Open files
  struct inode *cwd;           // Current directory
  char name[16];               // Process name (debugging)
  unsigned int num_ticks;       // 프로세스의 tick
  unsigned int priority_no;     // 프로세스가 속한 현재 큐의 레벨
  unsigned int selfYield;       // 프로세스가 자체적으로 yield한건지 아님 타임 인터럽트 때문에 yield한건지
  double stride;          // 프로세스의 share 에 따른 stride
  double passvalue;       // 프로세스의 현 pass value
  unsigned int isStride;        // 프로세스가 stride식인지 아닌지 (cpu share을 요청한 프로세스인지 아닌지)
  unsigned int isThread;        // 현 프로세스가 thread인지 아닌지
  unsigned int threadID;        // 스레드에게 주어지는 아이디 0 ~ 63까지 있다.
  void* ret[64];                // thread_exit() 시 부모 프로세스에게 thread_join()을 통하여 전달해주는 인자값이 있는데 그 인자값을 저장하는 배열, 인덱스는 스레드 ID 로 구별한다.
  unsigned int spaceValid[64];  // thread를 무한정 생성하게 되어 페이지 공간이 터지지 않도록 하기 위해, 스레드가 종료될 시 그 아이디 인덱스 위치에 0을 집어넣고 스레드가 존재할 시 그 위치에 1을 집어 넣어서 스레드의 생성을 통제해주는 배열.
  unsigned int isZombie;        // 조건 검사를 위해 추가된 flag이며, 현 스레드의 부모가 종료된 상태인지 확인하는 flag이다.
  unsigned int thrFork;         // 스레드가 fork()를 호출해서 만들어진 프로세스인지 아닌지 판별하는 flag. 
};
typedef uint thread_t;     // 이름을 보기 쉽게 만듬
double mlfqStride;        // mlfq의 stride
double mlfqPassvalue;     // mlfq의 pass value
unsigned int boost_timer;       // mlfq내에서 starvation 방지를 위한 boost timer
unsigned int modeStride;        // 현재 scheduling mode가 stride인지 mlfq인지
unsigned int fromStride;        // 타임 인터럽트 발생 시 그 당시 스케쥴링 모드가 stride 였는지 mlfq였는지
// Process memory is laid out contiguously, low addresses first:
//   text
//   original data and bss
//   fixed-size stack
//   expandable heap
