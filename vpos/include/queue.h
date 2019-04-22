
#ifndef _VPOS_QUEUE_H_
#define _VPOS_QUEUE_H_

#include "thread.h"

typedef struct vk_Thread_Queue{
	unsigned priority;
	unsigned counter;
	vk_thread_t *list_head;
	vk_thread_t *list_tail;
} vk_threadq_t;

vk_threadq_t vk_ready_queue[32];
vk_threadq_t vk_wait_queue[32];
vk_threadq_t vk_tcb_log_queue[32];		

void vk_enqueue(vk_thread_t *thread, vk_threadq_t *list_t);
vk_thread_t* vk_dequeue(vk_thread_t *thread, vk_threadq_t *list_t);
void vk_thread_enqueue(vk_thread_t *thread, vk_threadq_t *list_t);
vk_thread_t* vk_thread_dequeue(vk_thread_t *thread, vk_threadq_t *list_t);

void vk_init_kdata_struct(void);
void vk_init_ready_queue(void);
void vk_init_tcb_log_queue(void);
void vk_init_wait_queue(void);
void vk_init_thread_join_table(void);

#endif //_VPOS_QUEUE_H_
