#ifndef MPOS_HAL_HWI_HANDLER_H
#define MPOS_HAL_HWI_HANDLER_H

#define RECOPLAY

#include "vh_cpu_hal.h"
#include "vh_io_hal.h"
#include "vh_variant_hal.h"
#include "dd.h"
#include "hwi_handler.h"
#include "recoplay.h"

void vh_hwi_classifier(void)
{ 
/*	unsigned int intoffset;
	unsigned int irqstatus;
	int i;
	for(i=0; i<32; i++)
	{
		if(irqstatus & 0x1) {
			intoffset = i;
		}
		irqstatus << 1;
	}
	vk_dd_table[vk_idt_table[intoffset]].fop_list->vk_interrupt();
*/
}

void vh_interrupt_mask(int mask_irq_number)
{
}

void vh_interrupt_unmask(int unmask_irq_number)
{
}

#endif /* MPOS_HAL_HWI_HANDLER_H */
