#include "vh_cpu_hal.h"
#include "vh_io_hal.h"
#include "timer.h"
#include "printk.h"
#include "dd.h"
#include "hwi_handler.h"
#include "scheduler.h"
#include "thread.h"
#include "queue.h"
#include "semaphore.h"
#include "recoplay.h"

extern int vk_sched_lock;
extern vk_thread_t *vk_current_thread;


void vk_timer_irq_enable(void)
{	
	vh_TCON = (vh_TCON & ~0x700000) | 0x600000;
	vh_TCON = (vh_TCON & ~0x700000) | 0x500000;
	vh_TINT_CSTAT = 0x210;
}

void vk_timer_irq_disable(void)
{
	vh_TCON = (vh_TCON & ~0x700000) | 0x600000;
	vh_TINT_CSTAT = 0x200;
}

void vh_timer_init(void)
{
	
	vk_timer_flag = 0;
	vh_TCFG0 = 0xff00;
	vh_TCFG1 = 0x40000;
	vh_TCNTB4 = 0x3ef1;
}

void vh_timer_irq_enable(int timer)
{
	
	switch(timer){
	case 0: 
		 break;
	case 1: 
		 break;
	case 2:
		 break;
	case 3:	
		 break;
	case 4:
		vh_VIC0VECTADDR25 = (unsigned int) &vh_timer_interrupt_handler;
		vh_VIC0INTENABLE |= vh_VIC_TIMER4_bit;
		vh_VIC0INTSELECT &= ~vh_VIC_TIMER4_bit;
		vh_VIC0SWPRIORITYMASK = 0xffff;
		break;
	default: break;
	}
}
void vh_timer_interrupt_handler(void)
{
	vk_timer_irq_disable();
	vh_save_thread_ctx(vk_timer_save_stk);
	
	// timer interrupt clear & enable
	vh_VIC0INTENCLEAR |= vh_VIC_TIMER4_bit;
	vh_VIC0INTENABLE |= vh_VIC_TIMER4_bit;
	
	vk_sched_save_tcb_ptr = (unsigned int)vk_timer_save_stk;
	vk_timer_flag = 1;

	++(vk_current_thread->cpu_tick);
	if(vk_sched_lock==0) {
		vk_swi_scheduler();
	}
}
