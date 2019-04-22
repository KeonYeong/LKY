/*
 * Copyright (c) 2001 Advanced Core Tech.
 * All Rights Reserved.
 */

#ifndef _VPOS_STDLIB_H_
#define _VPOS_STDLIB_H_

#include "stdio.h"

#ifndef MYSEO
int strcasecmp(const char* s1, const char* s2);
int strncasecmp(const char* s1, const char* s2, int maxlen);
#endif

int abs(int value);
unsigned long atoi(char* ptr);
unsigned long atol(char* ptr);
void itoa(unsigned N, char str[]);
void itoa16(unsigned N, char str[]);

#endif //_VPOS_STDLIB_H_
