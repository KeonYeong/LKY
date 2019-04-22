
#ifndef _VPOS_PTHREAD_H_
#define _VPOS_PHTREAD_H_

#include "thread.h"

#define MUTEX_SIZE	10
#define COND_VARIABLE_SIZE	10
#define BARRIER_SIZE	10

#ifndef HSKIM // MPOS
typedef struct kcall_mutex {
	unsigned char thread_id;
	unsigned char prio;
	unsigned char init_prio;
} vk_kcall_mutex_t;
#endif

typedef struct Mutex{
#ifndef HSKIM // MPOS
	unsigned char mutex_index;
	unsigned char mutex_flag;
	unsigned char current_thread_id;
	unsigned char kcall_thread_prio;
//	unsigned char init_	prio;
	vk_kcall_mutex_t mutex_wait_thread[3];
	vk_kcall_mutex_t mutex_lock_holder;
#endif
	unsigned int m_priority;
	unsigned int m_wait_cnt;
	unsigned int m_val; 		
	vk_thread_t *wait_thread[3];         
	vk_thread_t *lock_holder;	
} vk_mutex_t;

typedef vk_mutex_t *pthread_mutex_t;

#ifndef HSKIM // MPOS
pthread_mutex_t	vk_master_mutex_q[MUTEX_SIZE];
#endif

typedef struct mutexattr{
	void *undefined;
} pthread_mutexattr_t;

//For local Mutex
int pthread_mutex_init(pthread_mutex_t* mutex, const pthread_mutexattr_t* mutex_attr);
int pthread_mutex_destroy(pthread_mutex_t* mutex);
int pthread_mutex_lock(pthread_mutex_t* mutex);
int pthread_mutex_trylock(pthread_mutex_t* mutex);
int pthread_mutex_unlock(pthread_mutex_t* mutex);

//For Kernel Call Handler
int vk_kcall_pthread_mutex_init(pthread_mutex_t* mutex, const pthread_mutexattr_t* mutex_attr);
int vk_kcall_pthread_mutex_destroy(pthread_mutex_t* mutex);
int vk_kcall_pthread_mutex_lock(pthread_mutex_t* mutex);
int vk_kcall_pthread_mutex_trylock(pthread_mutex_t* mutex);
int vk_kcall_pthread_mutex_unlock(pthread_mutex_t* mutex);
//-------------------------------------------------------------------------------------

#ifndef HSKIM // MPOS
typedef struct kcall_condition_variable{
	unsigned char stub_signal_waiter;
	unsigned char waiter_thread_prio;
} vk_kcall_cond_t;
#endif

typedef struct condition_variable{
#ifndef HSKIM // MPOS
	unsigned char cond_variable_index;
	unsigned char cond_variable_flag;
	unsigned char current_thread_id;
	unsigned char current_thread_priority;
	vk_kcall_cond_t kcall_waiter[3];
#endif
	vk_thread_t *signal_waiter[3];
} vk_cond_t;

typedef vk_cond_t *pthread_cond_t;
pthread_cond_t vk_master_cond_variable_q[COND_VARIABLE_SIZE];
pthread_mutex_t vk_master_cond_mutex;

typedef struct cond_attr{
	void *undefined;
} pthread_condattr_t;

char broadcast_thread_id[3];

//For Local condition variable
int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr);
int pthread_cond_destroy(pthread_cond_t *cond);
int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mu);
int pthread_cond_signal(pthread_cond_t *cond);
int pthread_cond_broadcast(pthread_cond_t* cond);

//For Kernel Call Handler
int vk_kcall_pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr);
int vk_kcall_pthread_cond_destroy(pthread_cond_t *cond);
int vk_kcall_pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mu);
int vk_kcall_pthread_cond_signal(pthread_cond_t *cond);
int vk_kcall_pthread_cond_broadcast(pthread_cond_t* cond);

//-------------------------------------------------------------------------------------
typedef struct barrier{
#ifndef	HSKIM // MPOS
	unsigned char barrier_index;
	unsigned char barrier_flag;
	unsigned char wait_barrier_thread[5];
#endif
	unsigned int thread_num;
	vk_thread_t *waiter[5];
} vk_barrier_t;

typedef vk_barrier_t *pthread_barrier_t;
pthread_barrier_t vk_master_barrier_q[BARRIER_SIZE];

typedef struct barrier_attr{
	void *undefined;
} pthread_barrierattr_t;

//For local barrier
int pthread_barrier_init(pthread_barrier_t *barrier, const pthread_barrierattr_t *attr, unsigned count);
int pthread_barrier_destroy(pthread_barrier_t *barrier);
int pthread_barrier_wait(pthread_barrier_t *barrier);

//For kernel Call handler
int vk_kcall_pthread_barrier_init(pthread_barrier_t *barrier, const pthread_barrierattr_t *attr, unsigned count);
int vk_kcall_pthread_barrier_destroy(pthread_barrier_t *barrier);
int vk_kcall_pthread_barrier_wait(pthread_barrier_t *barrier);
//--------------------------------------------------------------------------------------

extern vk_thread_t *vk_current_thread;

#endif //_VPOS_PTHREAD_H_
