#include "debug.h"
#include "printk.h"
#include "exception_handler.h"
#include "vh_cpu_hal.h"
#include "swi_handler.h"
extern unsigned int vk_event_em;
extern unsigned int vk_error_addr;
extern unsigned int *vk_save_pabort_current_tcb_bottom;
extern unsigned int *vk_save_irq_mode_stack_ptr;
extern unsigned int *vk_save_swi_current_tcb_bottom;

void vk_undef_handler(void)
{
	printk("Undefined instruction exception. Location [0x%x]\n", vk_error_addr);
	while(1);
}

void vk_pabort_handler(void)
{
	printk("Prefetch abort exception. Location [0x%x]\n", *(vk_save_pabort_current_tcb_bottom+1));
	while(1);
}
void vk_dabort_handler(void)
{
	printk("Data abort exception. Location [0x%x]\n", vk_error_addr);
	while(1);
}

void vk_fiq_handler(void)
{
	printk("FIQ abort exception.\n");
	while(1);
}

void vk_not_used_handler(void)
{
	printk("NOT_USED exception.\n");
}

