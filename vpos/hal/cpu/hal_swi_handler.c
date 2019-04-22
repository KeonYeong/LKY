#include "vh_cpu_hal.h"
#include "vh_io_hal.h"
#include "swi_handler.h"
#include "printk.h"
#include "thread.h"
#include "scheduler.h"
#include "timer.h"
#include "linux/string.h"
#include "recoplay.h"
#include "hwi_handler.h"
#include "queue.h"

#define TRUE 1
#define FALSE 0

extern vk_thread_t *vk_current_thread;
extern unsigned int *vk_save_irq_current_tcb_bottom;
extern unsigned int *vk_save_swi_current_tcb_bottom;

extern unsigned int *vk_save_swi_mode_stack_ptr;
void vk_swi_classifier(unsigned thread)
{
	unsigned number;
	vk_thread_t *vector;
	unsigned temp;
	int i;
	unsigned int *k=vk_save_swi_mode_stack_ptr;
	unsigned int *kk=vk_save_swi_current_tcb_bottom;
	//printk("vk_swi_classifier switch up\n");
	vector=(vk_thread_t *)thread;
	number=vector->swi_number;
	switch(number)
	{
		case EI:
			vector->interrupt_state = FALSE;
			vh_enable_interrupt(vector);
			break;
		case DI:
			vector->interrupt_state = TRUE;
			vh_disable_interrupt(vector);
			break;
		case SC:
			vh_save_thread_ctx((unsigned)vector->tcb_bottom);
			break;
		case RC:
			temp = (unsigned)vector->func;
			vh_restore_thread_ctx((unsigned)vector->tcb_bottom);
			break;
		case CS:
			vk_scheduler();
			break;
	}
}
void vk_enable_interrupt(void)
{
	unsigned temp;

	vk_current_thread->swi_number = EI;
	temp = (unsigned)vk_current_thread;
	vh_swi(temp);
}

void vk_disable_interrupt(void)
{
	unsigned temp;

	vk_current_thread->swi_number = DI;
	temp = (unsigned)vk_current_thread;
	vh_swi(temp);
}

void vk_swi_scheduler(void)
{
	unsigned temp;
	
	vk_current_thread->swi_number = CS;
	temp = (unsigned)vk_current_thread;
	vh_swi(temp);
}


void vk_save_thread_ctx(vk_thread_t *thread)
{
	unsigned temp;

	thread->swi_number = SC;
	temp = (unsigned)thread;
	vh_swi(temp);
}

void vk_restore_thread_ctx(vk_thread_t *thread)
{
	unsigned temp;

	thread->swi_number = RC;
	temp = (unsigned)thread;
	vh_swi(temp);
}

void vh_swi(unsigned thread)
{
	asm volatile("swi 0x00");
}

void vh_enable_interrupt(vk_thread_t* thread)
{
        unsigned long temp;
        __asm__ __volatile__("mrs %0, spsr\n"
                             "bic %0, %0, #0xc0\n"
                             "msr spsr_c, %0\n"
                             : "=r" (temp));
}

void vh_disable_interrupt(vk_thread_t* thread)
{
        unsigned long temp;
	
        __asm__ __volatile__("mrs %0, spsr\n"
                             "orr %0, %0, #0x80\n"
                             "msr spsr_c, %0\n"
                             : "=r" (temp));
}

