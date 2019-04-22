
#ifndef _VPOS_PRINTK_H_
#define _VPOS_PRINTK_H_

#define FUNCTRACE	1	// function trace flag

#include "linux/types.h"
#include "stdarg.h"

int printk(const char *fmt, ...);
int printf(const char *fmt, ...);
int vsnprintf(char *buf, size_t size, const char *fmt, va_list args);
char * number(char * buf, char * end, long long num, int base, int size, int precision, int type);
int skip_atoi(const char **s);

#endif //_VPOS_PRINTK_H_ 

