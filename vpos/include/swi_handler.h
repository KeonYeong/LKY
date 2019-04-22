
#ifndef VPOS_SWI_HANDLER_H
#define VPOS_SWI_HANDLER_H

#include "thread.h"  

#define read_register(n) HAL_read_register(n)

enum vk_swi_vector_type {EI=40, DI=41, SC=42, RC=43, CS=44, SW=1911};

void vh_swi(unsigned thread);
void vk_swi_classifier(unsigned thread);
void vh_enable_interrupt(vk_thread_t* thread);
void vh_disable_interrupt(vk_thread_t* thread);
void vh_save_thread_ctx(unsigned bottom);
void vh_restore_thread_ctx(unsigned ctx_bottom);

void vk_enable_interrupt(void);	
void vk_disable_interrupt(void);
void vk_save_thread_ctx(vk_thread_t *thread);
void vk_restore_thread_ctx(vk_thread_t *thread);
void vk_swi_scheduler(void);

#endif //VPOS_SWI_HANDLER_H







