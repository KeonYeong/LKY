
#ifndef _VPOS_SEMAPHORE_H__
#define _VPOS_SEMAPHORE_H_

#include "thread.h"

#ifndef HSKIM // MPOS
#define SEM_SIZE 10
#endif

typedef struct kcall_sem {
	unsigned char thread_id;
	unsigned char prio;
	unsigned char init_prio;
} vk_kcall_sem_t;

typedef struct Semaphore{
#ifndef HSKIM // MPOS
	unsigned char sem_desc;
	unsigned char sem_flag;
	unsigned char current_thread_id;
	unsigned char kcall_thread_prio;
#endif
	unsigned int sem_priority;
	unsigned int sem_wait_cnt;
	//unsigned int sem_val; 		
	int 	     sem_val;
	unsigned int sem_init_val;
#ifndef HSKIM // MPOS
	vk_kcall_sem_t stub_wait_thread[3];
	vk_kcall_sem_t stub_sem_holder[3];
#endif
	vk_thread_t *wait_thread[3];         
	vk_thread_t *sem_holder[3];
} vk_sem_t;

typedef vk_sem_t *sem_t;

sem_t vk_master_sem_q[SEM_SIZE];

int sem_init(sem_t* sem, int pshared, unsigned int value);
int sem_destory(sem_t* sem);
int sem_trywait(sem_t *sem);
int sem_wait(sem_t* sem);
int vk_sem_wait(sem_t* sem);
int sem_post(sem_t* sem);
int vk_sem_post(sem_t* sem);
int sem_getvalue(sem_t *sem, int *sval);

//For Kernel Call Server
int vk_kcall_sem_init(sem_t* sem, int pshared, unsigned int value);
int vk_kcall_sem_destroy(sem_t* sem);
int vk_kcall_sem_trywait(sem_t *sem);
int vk_kcall_sem_wait(sem_t* sem);
int vk_kcall_vk_sem_wait(sem_t* sem);
int vk_kcall_sem_post(sem_t* sem);
int vk_kcall_vk_sem_post(sem_t* sem);
int vk_kcall_sem_getvalue(sem_t *sem, int *sval);

#endif //_VPOS_SEMAPHORE_H__
