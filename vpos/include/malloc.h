#ifndef _VPOS_MALLOC_H_
#define _VPOS_MALLOC_H_

#include "thread.h"

void* malloc(unsigned nbytes);
void free(void* ap);

extern vk_thread_t *vk_current_thread;

#endif //_VPOS_MALLOC_H_
