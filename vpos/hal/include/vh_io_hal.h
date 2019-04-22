#ifndef _vh_IO_HAL_H_
#define _vh_IO_HAL_H_
/*******************************************************************
	VIC - S5PC100
*******************************************************************/

#define vh_VIC_UART1	11
#define vh_VIC_UART1_bit (1 << vh_VIC_UART1)

#define vh_VIC_TIMER4	25
#define vh_VIC_TIMER4_bit (1 << vh_VIC_TIMER4)

//VIC0
#define vh_VIC0INTSELECT		(*(volatile unsigned *)0xe400000c)
#define vh_VIC0INTENABLE		(*(volatile unsigned *)0xe4000010)
#define vh_VIC0INTENCLEAR		(*(volatile unsigned *)0xe4000014)
#define vh_VIC0SWPRIORITYMASK	(*(volatile unsigned *)0xe4000024)
#define vh_VIC0VECTADDR25		(*(volatile unsigned *)0xe4000164)

//VIC1
#define vh_VIC1INTSELECT		(*(volatile unsigned *)0xe410000c)
#define vh_VIC1INTENABLE		(*(volatile unsigned *)0xe4100010)
#define vh_VIC1INTENCLEAR		(*(volatile unsigned *)0xe4100014)
#define vh_VIC1SWPRIORITYMASK	(*(volatile unsigned *)0xe4100024)
#define vh_VIC1VECTADDR11		(*(volatile unsigned *)0xe410012c)

/*******************************************************************
   	GPIO
 *******************************************************************/
#define vh_GPA0CON		(*(volatile unsigned *)0xe0300000)
#define vh_GPA0PUD		(*(volatile unsigned *)0xe0300008)
#define vh_GPJ2CON		(*(volatile unsigned *)0xe0300240)
#define vh_GPJ2DAT		(*(volatile unsigned *)0xe0300244)


/*****************************************************************
   Timer address
 *****************************************************************/
#define vh_TCFG0		(*(volatile unsigned *)0xea000000)
#define vh_TCFG1		(*(volatile unsigned *)0xea000004)
#define vh_TCON			(*(volatile unsigned *)0xea000008)
#define vh_TINT_CSTAT	(*(volatile unsigned *)0xea000044)
#define vh_TCNTB4		(*(volatile unsigned *)0xea00003c)
#define vh_TCNTO4		(*(volatile unsigned *)0xea000040)


/******************************************************************
  	UART address
 ******************************************************************/

#define vh_vSERIAL_CON			0x220000
#define vh_vSERIAL_PUD			0xa00

#define vh_UART_CTL_BASE 		0xec000000

#define vh_ULCON	 		(*(volatile unsigned *)(vh_UART_CTL_BASE+0x0400))	// Specifies line control
#define vh_UCON	 			(*(volatile unsigned *)(vh_UART_CTL_BASE+0x0404))	// Specifies control
#define vh_UFCON	 		(*(volatile unsigned *)(vh_UART_CTL_BASE+0x0408))	// Specifies FIFO control
#define vh_UMCON	 		(*(volatile unsigned *)(vh_UART_CTL_BASE+0x040c))	// Specifies modem control
#define vh_UBRDIV	 		(*(volatile unsigned *)(vh_UART_CTL_BASE+0x0428))	// SPecifies baud rate divisor
#define vh_UINTP1			(*(volatile unsigned *)(vh_UART_CTL_BASE+0x0430))
#define vh_UINTSP1			(*(volatile unsigned *)(vh_UART_CTL_BASE+0x0434))
#define vh_UINTM1			(*(volatile unsigned *)(vh_UART_CTL_BASE+0x0438))
#define vh_UTRSTAT1			(*(volatile unsigned *)(vh_UART_CTL_BASE+0x0410))
#define vh_UERSTAT1			(*(volatile unsigned *)(vh_UART_CTL_BASE+0x0414))
#define vh_UFSTAT1			(*(volatile unsigned *)(vh_UART_CTL_BASE+0x0418))
#define	vh_URXH1			(*(volatile unsigned *)(vh_UART_CTL_BASE+0x0424))
#define vh_UMSTAT1			(*(volatile unsigned *)(vh_UART_CTL_BASE+0x041C))
#define vh_vULCON                       0  
#define vh_vUCON                        0
#define vh_UART_BAUD_RATE               115200

#define vh_UART_BRD                     (((200*1000000) / (vh_UART_BAUD_RATE * 16)) - 1)	/* Baud rate division value
											DIV_VAL = (SCLK_UART / (bps * 16)) - 1
											115200 bps, SCLK_UART 40MHz */
#define vh_oUTRSTAT                    	0x10	// Specifies Tx/Rx status
#define vh_oUTXH                        0x0420	// Specifies transmit buffer
#define vh_oURXH                        0x24 	// Specifies receive buffer

#define vh_UTRSTAT_TX_EMPTY             (1 << 2)// Transmitter empty. if the transmit buffer has no valid data to transmit, then 1
#define vh_UTRSTAT_RX_READY             (1 << 0)// Receive buffer data ready. 1=buffer has a received data

/* UART Transmit Operation */
#define vh_SERIAL_WRITE_READY() 	((vh_UTRSTAT1) & vh_UTRSTAT_TX_EMPTY)
#define vh_UTXH1                        vk_REGb(vh_UART_CTL_BASE + (vh_oUTXH))
#define vh_SERIAL_WRITE_CHAR(c) 	((vh_UTXH1) = (c))
#define vh_SERIAL_PUTC(c)               ({while (!vh_SERIAL_WRITE_READY());vh_SERIAL_WRITE_CHAR(c);})

/* UART Receive OPeration */
#define vh_SERIAL_CHAR_READY()  	(vh_UTRSTAT1 & vh_UTRSTAT_RX_READY)
//#define vh_URXH1                        vk_REGb(vh_UART_CTL_BASE + (vh_oURXH))
#define vh_SERIAL_READ_CHAR()   	vh_URXH1

/************************************************************************************************/
#define vh_oUERSTAT				0x14	
#define vh_oURXHL				0x24	
#define vh_UERSTAT0				vh_bUART(0, vh_oUERSTAT)
#define vh_SERIAL_READ_STATUS()	(vh_UERSTAT0 & vh_UART_ERR_MASK) 
#define vh_UTXH0				vh_bUARTb(0, vh_oUTXHL)

#define vh_vUFCON				0x0
#define vh_vUMCON				0x0
#define vh_UART_ERR_MASK		0xF 

#define vh_bUART(x, Nb)			vk_REGl(vh_UART_CTL_BASE + (x)*0x4000 + (Nb))
#define vh_bUARTb(x, Nb)		vk_REGb(vh_UART_CTL_BASE + (x)*0x4000 + (Nb))

/*******************************************************************
   	nand flash address 
 *******************************************************************/
#define U8 		unsigned char
#define NAND_BASE 	0xe7200000

#define vh_rNFCONF 		(*(volatile unsigned *)(NAND_BASE+ 0x00))
#define vh_rNFCMD		(*(volatile U8*)(NAND_BASE + 0x04))
#define vh_rNFADDR 		(*(volatile U8*)(NAND_BASE + 0x08))
#define vh_rNFDATA		(*(volatile U8*)(NAND_BASE + 0x0c))
#define vh_rNFSTAT		(*(volatile unsigned *)(NAND_BASE + 0x10))
#define vh_rNFECC 		(*(volatile unsigned *)(NAND_BASE + 0x14))
#define vh_rNFECC0		(*(volatile U8 *)(NAND_BASE + 0x14))
#define vh_rNFECC1		(*(volatile U8 *)(NAND_BASE + 0x15))
#define vh_rNFECC2 		(*(volatile U8 *)(NAND_BASE + 0x16))

//NAND Flash Macro
#define NF_CMD(cmd) 		{vh_rNFCMD = cmd;}
#define NF_ADDR(addr) 		{vh_rNFADDR = addr;}
#define NF_nFCE_L() 		{vh_rNFCONF &=~(1 <<11);}
#define NF_nFCE_H() 		{vh_rNFCONF |= (1 <<11);}
#define NF_RSTECC()		{vh_rNFCONF |= (1 <<12);}
#define NF_RDDATA() 		(vh_rNFDATA)
#define NF_WRDATA(data) 	{vh_rNFDATA = data;}
#define NF_WAITRB() 		{while(!(vh_rNFSTAT&(1<<0)));}

// RnB Signal
#define NF_CLEAR_RB()           {vh_rNFSTAT |= (1<<2);}    // Have write '1' to clear this bit.
#define NF_DETECT_RB()          {while(!(vh_rNFSTAT&(1<<2)));}

#define NAND_CTL_S3C_WAIT 	0x100
/*****************************************************************************/

#endif  //_vh_IO_HAL_H_
