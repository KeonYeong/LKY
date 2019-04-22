#include "machine_init.h"
#include "pthread.h"
#include "scheduler.h"
#include "queue.h"
#include "vpos_shell.h"
#include "version.h"
#include "printk.h"
#include "recoplay.h"
#include "timer.h"
#include "pmu.h"
#include "debug.h"
#include "serial.h"

void __aeabi_unwind_cpp_pr0(void){}

void set_interrupt(void)
{
	// interrupt setting
	vh_serial_irq_enable();
	vh_timer_irq_enable(vh_TIMER4);
}

void TIMER_test(void){
	int timebuffer[10];
	int i;

	vh_TCON = (vh_TCON & ~0x700000) | 0x600000;
	vh_TCON = (vh_TCON & ~0x700000) | 0x500000;

	for (i = 0 ; i < 10; i ++){
		timebuffer[i] = vh_TCNTO4;
		printk("timebuffer[%d] = %d\n", i, timebuffer[i]);
	}
}

void VPOS_kernel_main( void )
{
	pthread_t p_thread, p_thread_0, p_thread_1, p_thread_2;
	
	/* static and global variable initialization */
	vk_scheduler_unlock();
	init_thread_id();
	init_thread_pointer();
	vh_user_mode = USER_MODE;
	vk_init_kdata_struct();
	
	vk_machine_init();
	set_interrupt();

	
	printk("%s\n%s\n%s\n", top_line, version, bottom_line);
	
	//TIMER test
	TIMER_test();

	/* initialization for thread */
	race_var = 0;
	pthread_create(&p_thread, NULL, VPOS_SHELL, (void *)NULL);
	pthread_create(&p_thread_0, NULL, race_ex_1, (void *)NULL);
	pthread_create(&p_thread_1, NULL, race_ex_0, (void *)NULL);
	pthread_create(&p_thread_2, NULL, race_ex_2, (void *)NULL);

	VPOS_start();

	/* cannot reach here */
	printk("OS ERROR: VPOS_kernel_main( void )\n");
	while(1){}
}

