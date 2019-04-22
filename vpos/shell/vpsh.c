#include "swi_handler.h"
#include "vpos_shell.h"
#include "malloc.h"
#include "parser.h"
#include "printf.h"
#include "thread.h"
#include "timer.h"
#include "recoplay.h"
#include "pmu.h"
#include "serial.h"
#include "led.h"
#define	RACE_NO	300000

void *log_print_x(void *arg);
void *race_log(void *arg);
void *vu_debug(void *arg);
void *thread(void *arg);
void *thread2(void *arg);
void *thread3(void *arg);
void *thread4(void *arg);


VPOS_Shell commands[] = {
	{ "help" , "", vu_help		}, 
	{ "ls" , "", vu_help		}, 
	{ "debug", "", vu_debug		},
	{ "thread", "", thread		},
	{ "temp","",thread3		},
	{ "NULL" , "", NULL}
};

void *thread(void *arg)
{
	int i=0;
	pthread_t thread2_thread;
	vh_LedInit();
	pthread_create(&thread2_thread, NULL, thread2, (void *)NULL);
	while(1)
	{
		i++;
		if ( i== 5000000) {
			//vh_LedInit();
			printk("thread1 cpsr : %x\n", vt_read());
			printk("thread1 sp   : %x\n", vt_read_sp());
			i=0;
			//break;
		}
	}
	return 0;

}

void *thread2(void *arg)
{
	int j=0;
	while(1)
	{
		j++;
		if ( j== 5000000) {
			//vh_LedInit();
			printk("thread2 cpsr : %x\n", vt_read());
			printk("thread2 sp   : %x\n", vt_read_sp());
			j = 0;
			//break;

		}
	}
	return 0;
}


void *thread3(void *arg)
{
	int i=0;
	pthread_t thread4_thread;
	vh_LedInit();
	pthread_create(&thread4_thread, NULL, thread4, (void *)NULL);
	
	while(1) {
		i++;
		if ( i% 1000000 == 0) {
			//vh_LedInit();
			printk("thread1 cpsr : %x\n", vt_read());
			printk("thread1 sp   : %x\n", vt_read_sp());
			if ( i == 5000000)
				break;
	
		}
	}
	return 0;
}

void *thread4(void *arg)
{
	int j=0;
	while(1)
	{
		j++;
		if ( j% 1000000 == 0) {
			//vh_LedInit();
			printk("thread2 cpsr : %x\n", vt_read());
			printk("thread2 sp   : %x\n", vt_read_sp());
			if ( j == 5000000)
				break;

		}
	}
	return 0;
}


void *vu_debug(void *arg)
{
	unsigned int *temp, i;
	/*
	printf("Instruction counter: 0x%x\n", (unsigned int)read_pmn(0));
	printf("Misprediction counter: 0x%x\n", (unsigned int)read_pmn(1));
	printf("Overflow flags\t: 0x%x\n", (unsigned int)read_flags());
	*/
	printf("Ctx Switch Counter\t: %d\n", context_count);
	printf("HWI Counter\t: %d\n", vk_hwi_count);
	printf("Replay-ICINT counter\t: %d\n", vr_ic_int_cnt);
	printf("Replay-BKPT counter\t: %d\n", vr_bkpt_expt_cnt);
	printf("Replay-BKPT Fail counter\t: %d\n", vr_fail_cnt);
	printf("Replay-Register Fail counter\n");
	for(i=0;i<15;i++)
		printf("[%d] %d : ", i, vr_reg_fail_cnt[i]);
	printf("\n");
	printf("Current recorded ID : %d\n", ev_history.cur_event_id);
	/*
	temp = 0x10490200;
	for(i=0;i<5;i++){
		printf("interrupt pending reg[%d]: 0x%x\n", i, (unsigned int)*temp);
		temp++;
	}
	*/
	/*
	printf("INT group 0 status reg\t: 0x%x\n", vh_ISTR0);
	printf("INT group 0 masked status reg\t: 0x%x\n", vh_IMSR0);
	printf("Combined INT pending status\t: 0x%x\n", vh_CIPSR0);
	printf("vr_replay_count\t: 0x%x\n", vr_replay_count);
	*/
	return 0;
}

void *vu_help(void *arg)
{
	int i = 0;
	
	printk("\n*******************Command_List*******************\n");
	
	while(commands[i].func != NULL)
	{
#ifdef MYSEO
		printk ("%d.  %s %s\n", i, commands[i].command_string, commands[i].command_format);
#else
		printk ("- %s %s\n", commands[i].command_string, commands[i].command_format);
#endif
		++i;
	}
	printk("**************************************************\n");
	
	return 0;
}

unsigned int arr_0;
unsigned int arr_1;
unsigned int arr_2;
void *race_log(void *arg)
{
	printk("race_var = %d\n", race_var);
}
void *race_ex_0(void *arg)
{
	int i;
	for(i=0;i<RACE_NO;i++){
		arr_0 = race_var + 2;
		race_var = arr_0;
	}
	return 0;
}

void *race_ex_1(void *arg)
{
	int i;
	for(i=0;i<RACE_NO;i++){
		arr_1 = race_var - 1;
		race_var = arr_1;
	}
	return 0;
}

void *race_ex_2(void *arg)
{
	int i;
	for(i=0;i<RACE_NO;i++){
		arr_2 = race_var + 3;
		race_var = arr_2;
	}
	return 0;
}

void *VPOS_SHELL(void *arg)
{
	char c_buff[100];
	int argc;
	char *argv[10];
	int cmd;
	pthread_t p_thread;
	int cmd_check;

	printk("\nRace condition value = %u\n", race_var);
	argc = 0;
	while (1)
	{
		cmd_check = 0;
	
		printk("\nShell>");

		get_cmd(c_buff);
		
		argc = parse_args(c_buff, argv);
		if(argc)
		{
			cmd = 0;
			while(commands[cmd].command_string)
			{
				if(strcmp(argv[0], commands[cmd].command_string)==0)
				{
					pthread_create(&p_thread, NULL, commands[cmd].func, (void *)&argv[1]);
					printk("\n");
					vk_swi_scheduler();
					cmd_check = 1;
					break;
				}
				cmd++;
			}
			if (!cmd_check)
				printk ("\n%s: command not found", argv[0]);
		}
	}

	return (void *)0;
}
