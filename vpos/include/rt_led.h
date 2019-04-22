#ifndef _VPOS_LED_H_
#define _VPOS_LED_H_

// Libertron BaseLED BSP

#define _IN	(0x0)
#define _OUT	(0x1)
#define _EINT	(0x2)

#define GPCON7(x) (x<<28)
#define GPCON6(x) (x<<24)
#define GPCON5(x) (x<<20)
#define GPCON4(x) (x<<16)
#define GPCON3(x) (x<<12)
#define GPCON2(x) (x<<8)
#define GPCON1(x) (x<<4)
#define GPCON0(x) (x<<0)

#define LED1OUT GPCON0(_OUT)
#define LED2OUT GPCON1(_OUT)
#define LED3OUT GPCON2(_OUT)
#define LED4OUT GPCON3(_OUT)
#define LED5OUT GPCON4(_OUT)
#define LED6OUT GPCON5(_OUT)
#define LED7OUT GPCON6(_OUT)
#define LED8OUT GPCON2(_OUT)

#define LED_CR 0xa800c0c0
#define LED0 0xA800c0c6

#define LED1	(1<<0)
#define LED2	(1<<1)
#define LED3	(1<<2)
#define LED4	(1<<3)
#define LED5    (1<<4)
#define LED6    (1<<5)
#define LED7    (1<<6)
#define LED8    (1<<2)

void vh_LedInit(void);
void vh_LedSet(unsigned char data);
void vh_LedOn(unsigned char data);
void vh_LedOff(unsigned char data);

#endif // _VPOS_LED_H_
