
#ifndef	_VPOS_RTC_API_H_
#define _VPOS_RTC_API_H_

#include "rtc_init.h"

//API about time
char* ctime(const time_t *timep);
#ifdef MYSEO
time_t mktime(vk_time_t *timeptr);
#else
time_t mk_time(vk_time_t *timeptr);
#endif
void sleep(int sec);

#endif //_VPOS_RTC_API_H_
