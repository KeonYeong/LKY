#include "linux/string.h"

#define NULL ((void *)0)

typedef struct {
	char *command_string;
	char *command_format;
	void *(*func)(void *arg);
} VPOS_Shell;

void *VPOS_SHELL(void *arg);
void *vu_help(void *arg);

void *race_ex_2(void *arg);
void *race_ex_0(void*arg);
void *race_ex_1(void*arg);

