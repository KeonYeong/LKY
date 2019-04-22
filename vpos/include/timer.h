
#ifndef VPOS_TIMER_H
#define VPOS_TIMER_H

#include "vh_io_hal.h"

#define	vh_TIMER4	4
#define vh_TIMER1	1
void vh_timer_irq_enable(int timer);

void vh_timer_init(void);
void vh_timer_interrupt_handler(void);
int vr_DeltaT_interrupt_handler(void);
void vh_timer1_interrupt_handler(void);
void vk_wait_for(int time);
unsigned int read_timer(void);
void vh_restart_timer(void);

unsigned int vk_timer_save_stk[17];
unsigned int vk_timer_flag;
// TIMER4 Operations
#define vr_read_time4() 	((unsigned int)0xffffffff - (unsigned int)vh_rTCNTO4)
#define vr_timer4_start(x)	vh_rTCNTB4 =(unsigned int)0xffffffff - x; \
							vh_rTCON = (vh_rTCON & ~0x0700000) | 0x600000; \
							vh_rTCON = (vh_rTCON & ~0x0700000) | 0x500000;
#define vr_timer4_stop() 	vh_rTCON &= ~(0x1<<20);				
#define vr_timer4_reset() 	vh_rTCNTB4 = 159; \
							vh_rTCON = (vh_rTCON & ~0x0700000) | 0x600000; \
							vh_rTCON = (vh_rTCON & ~0x0700000) | 0x500000; 

// (0xf<<20)  (0x6<<20) (0x5<<20)
// SYSTEM TIMER Operation
#define vr_read_time()	((unsigned int)0xffffffff - (unsigned int)vh_rTCNTO)
#define vr_timer_start(x)	vh_rTCNTB = (unsigned int)0xffffffff - x; \
							 vh_rSYSTCON = (vh_rSYSTCON & ~0x07) | 0x06; \
							 vh_rSYSTCON = (vh_rSYSTCON & ~0x07) | 0x05;
#define vr_timer_stop()		vh_rSYSTCON &= ~0x1;
#define vr_timer_reset()	vh_rTCNTB = 0xffffffff; \
							 vh_rSYSTCON = (vh_rSYSTCON & ~0x07) | 0x06; \
							 vh_rSYSTCON = (vh_rSYSTCON & ~0x07) | 0x05;

							
// TIMER1 Operations
#define TIMER1_COUNT		0xffff
#define vr_read_time1() 	((unsigned int)0xffff - (unsigned int)vh_rTCNTO1)
#define vr_timer1_start(x)	vh_rTCNTB1 =(unsigned int)0xffff - x; \
							vh_rTCON = (vh_rTCON & ~(0xf<<8)) |( 0xa<<8); \
							vh_rTCON = (vh_rTCON & ~(0xf<<8)) | (0x9<<8);
#define vr_timer1_stop() 	vh_rTCON &= ~(0x1<<8);				
#define vr_timer1_reset() 	vh_rTCNTB1 = 0xffff; \
							vh_rTCON = (vh_rTCON & ~(0xf<<8)) |( 0xa<<8); \
							vh_rTCON = (vh_rTCON & ~(0xf<<8)) | (0x9<<8);
#endif //VPOS_TIMER_H
