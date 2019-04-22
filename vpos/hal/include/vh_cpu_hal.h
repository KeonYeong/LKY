
#ifndef _vh_CPU_HAL_H_
#define _vh_CPU_HAL_H_

/*****************************************************************
   CPU
 *****************************************************************/
#define vh_WTCON				0x53000000
#define vh_WT_Disable				0x0
#define vh_Disable_Interrupt			0xc0
#define vh_INT_CTL_BASE				0x4a000000
#define vh_oCLKDIVN				0x14
#define vh_oMPLLCON				0x04
#define vh_vLOCKTIME				0x00ffffff        
#define vh_vCLKCON				0x0000fff8        
#define vh_vCLKDIVN				0x5             
#define vh_MDIV_50				0x5c
#define vh_PDIV_50				0x4
#define vh_SDIV_50				0x2
#define vh_MDIV_200				0x6e
#define vh_PDIV_200				0x3
#define vh_SDIV_200				0x1
#define vh_vMPLLCON_50				((vh_MDIV_50 << 12) | (vh_PDIV_50 << 4) | (vh_SDIV_50)) 
#define vh_vMPLLCON_200				((vh_MDIV_200 << 12) | (vh_PDIV_200 << 4) | (vh_SDIV_200)) 

#define vh_CLK_CTL_BASE				0x4c000000


/*****************************************************************
	ISR, GPIO
 *****************************************************************/
#define vh_rSRCPND          (*(volatile unsigned *)0x4a000000) 
#define vh_rINTMSK          (*(volatile unsigned *)0x4a000008) 
#define vh_rINTOFFSET       (*(volatile unsigned *)0x4a000014) 
#define vh_rINTPND          (*(volatile unsigned *)0x4a000010) 
#define vh_rSUBSRCPND       (*(volatile unsigned *)0x4a000018) //Sub source pending
#define vh_rINTSUBMSK       (*(volatile unsigned *)0x4a00001c) //Interrupt sub mask
#define ClearPending(bit)   {vh_rSRCPND = bit; vh_rINTPND = bit; vh_rINTPND;}


#define vh_Interrupt_All_Disable		0xffffffff
#if 1 // MYSEO
#define vh_oINTMSK					0x08
#define vh_oCLKDIVN					0x14
#define vh_oINTSUBMSK         				0x1c
#endif

//SWLEE
#define rGPFCON				(*(volatile unsigned *)0x56000050)
#define rGPFDAT				(*(volatile unsigned *)0x56000054)
#define rGPFUP				(*(volatile unsigned *)0x56000058)
#define rEXTINT0			(*(volatile unsigned *)0x56000088)
//SWLEE


#define GPIO_bit(x) (1 << ((x) & 0x1f))
#define SW1 ((*(volatile unsigned char *)0x60000013) & 0x10)
#define SW2 ((*(volatile unsigned char *)0x60000013) & 0x20)
#define SW3 ((*(volatile unsigned char *)0x60000013) & 0x40)
#define SW4 ((*(volatile unsigned char *)0x60000013) & 0x80)

#define IER         __REG(0x60002000)
#define IS          __REG(0x60002004)
#define IR          __REG(0x60002008)


/*****************************************************************
   SDRAM
 *****************************************************************/
#define vh_MEM_CTL_BASE			0x48000000
#define vh_vBWSCON			0x22111110
#define vh_vBANKCON0			0x00000700
#define vh_vBANKCON1			0x00000700
#define vh_vBANKCON2			0x00000700
#define vh_vBANKCON3			0x00000700
#define vh_vBANKCON4			0x00000700
#define vh_vBANKCON5			0x00000700
#define vh_vBANKCON6			0x00018005
#define vh_vBANKCON7			0x00018005
#define vh_vREFRESH			0x008e0459
#define vh_vBANKSIZE			0xb2
#define vh_vMRSRB6			0x30
#define vh_vMRSRB7			0x30

#define vh_SDRAM_BASE 			0x40008000
#define vh_SDRAM_Remapped_reset 	0x40008000
#define vh_SDRAM_Remapped_undef 	(vh_SDRAM_Remapped_reset + 0x04)
#define vh_SDRAM_Remapped_swi 		(vh_SDRAM_Remapped_reset + 0x08)
#define vh_SDRAM_Remapped_prefetch_abort (vh_SDRAM_Remapped_reset + 0x0c)
#define vh_SDRAM_Remapped_data_abort 	(vh_SDRAM_Remapped_reset + 0x10)
#define vh_SDRAM_Remapped_not_used 	(vh_SDRAM_Remapped_reset + 0x14)
#define vh_SDRAM_Remapped_irq 		(vh_SDRAM_Remapped_reset + 0x18)
#define vh_SDRAM_Remapped_fiq 		(vh_SDRAM_Remapped_reset + 0x1c)

#define vh_VPOS_BIN_SIZE 		(vh_SDRAM_BASE+0x80000)
#define vh_VPOS_FLASH_BASE 		0x00000000
#define vh_VPOS_BASE 			0x400

#endif  //_vh_CPU_HAL_H_
