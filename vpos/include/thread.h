
#ifndef _VPOS_THREAD_H_
#define _VPOS_THREAD_H_

#ifndef RECOPLAY
#define RECOPLAY
#endif

#define D_PRIORITY	15

#ifndef RECOPLAY
#define vh_user_mode	0x10
#else 
int vh_user_mode;
#endif

struct vk_item{
	unsigned int item[12];
};

typedef struct vk_thread{
	unsigned int state;	
	unsigned int thread_id;
	unsigned int priority;
	unsigned int init_priority;	
	int saved_sched_lock;
	unsigned int join_value;
	struct vk_thread *join_thread;
	void *return_value;
	unsigned int *tcb_bottom;
	unsigned int *stack;
	void *(*func)(void *arg);		
	unsigned int sched_lock_counter;
	unsigned int cpu_tick;
	unsigned int swi_number;
	unsigned int interrupt_state;
	struct vk_thread *next;
	unsigned int mode;
	unsigned int rtcfunc;
	unsigned int rtcarg1;		
	struct vk_item rtcarg2;		
	unsigned int rtcarg3;		
	unsigned int rtcarg4;		
} vk_thread_t;

typedef struct vk_pthread{
	unsigned int priority;
	vk_thread_t *thread;
} pthread_t;

typedef struct thread_join_struct{
	unsigned int flag;
	vk_thread_t *zombi_thread;
	vk_thread_t *sleep_thread;
} vk_thread_join_struct;	

enum vk_thread_state {RUNNING, READY, SLEEP, ZOMBI, INIT};

vk_thread_join_struct vk_thread_join_table[10];
vk_thread_t init_thread;

int pthread_create(pthread_t *p_thread, void *reserved, void *(*function)(void *), void *arg);
void pthread_exit(void *retval);
int pthread_join(pthread_t pthread, void **thread_return);
void vk_thread_yield(void);
void init_thread_id(void);
void init_thread_pointer(void);
void vh_restore_userm(void);
#endif	//_VPOS_THREAD_H_
