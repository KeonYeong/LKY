#include "serial.h"
#include "vh_cpu_hal.h"
#include "vh_variant_hal.h"
#include "vh_io_hal.h"
#include "dd.h"
#include "printk.h"

void vh_serial_interrupt_handler(void);

char getc(void)
{
	char c;
/*	unsigned long rxstat;

	while(!vh_SERIAL_CHAR_READY());

	c = vh_SERIAL_READ_CHAR();
	rxstat = vh_SERIAL_READ_STATUS();
*/

	while(pop_idx == push_idx){
	}
	c = serial_buff[pop_idx++];
	
	return c;
}

void putc(char c)
{
	vh_SERIAL_PUTC(c);
}

int tstc()
{
	return vh_SERIAL_CHAR_READY();
}

void vh_serial_init(void)
{
	
	int i;
	// UART 1 Setting
	// UART 1 GPIO setting
	vh_GPA0CON = vh_vSERIAL_CON;
	vh_GPA0PUD = vh_vSERIAL_PUD;

	// UART register setting
	vh_ULCON = 0x3;
	vh_UCON = 0x245;
	vh_UFCON = 0xc7;
	vh_UINTM1 = 0xe;
	vh_UINTP1 = 0xf;
	vh_UBRDIV = ((66000000 / (115200 * 16)) - 1);
	push_idx = 0;
	pop_idx = 0;
	for(i=0;i<SERIAL_BUFF_SIZE;i++)	serial_buff[i] = '\0';

	
}

void vh_serial_irq_enable(void)
{
	/* enable UART1 Interrupt */
	vh_VIC1VECTADDR11 = (unsigned int) &vh_serial_interrupt_handler;
	vh_VIC1INTENABLE |= vh_VIC_UART1_bit;
	vh_VIC1INTSELECT &= ~vh_VIC_UART1_bit;
	vh_VIC1SWPRIORITYMASK = 0xffff;
}

void vk_serial_push(void)
{
	char c;
	int i=0;
	c = vh_URXH1;
	if(push_idx == SERIAL_BUFF_SIZE){	// buffer is full
		push_idx = 0;
	}
	serial_buff[push_idx++] = c;
}

void vh_serial_interrupt_handler(void)
{
	vk_serial_push();
	vh_VIC1INTENCLEAR |= vh_VIC_UART1_bit;
	vh_VIC1INTENABLE |= vh_VIC_UART1_bit;
	vh_UINTP1 = 0xf;
}

