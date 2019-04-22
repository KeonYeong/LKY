
#ifndef _VPOS_RTC_H_
#define _VPOS_RTC_H_

#include "thread.h"
#include "linux/types.h"

#define max_sleepQ_buf 	31
#define Use		0x02	
#define Not_use		0x00

typedef struct tm {
	int tm_year;
	int tm_mon;
	int tm_mday;
	int tm_weekday;
	int tm_hour;
	int tm_min;
	int tm_sec;
}vk_time_t;

typedef struct sleep_Q {
	unsigned int sectime;
	unsigned int flags ;
	vk_thread_t *sleepThread;
}vk_sleep_queue_t;

vk_sleep_queue_t vk_sleep_queue[max_sleepQ_buf];

void vh_rtc_init(void);
void vh_timeInit(void);
int vh_SecTime_interrupt_handler(void);

void vh_gettime(vk_time_t *time);
void vh_settime(vk_time_t *time);

/*
char* ctime(const time_t *timep);
time_t mktime(vk_time_t *timeptr);
void sleep(int sec);
*/
#endif //_VPOS_RTC_H_
