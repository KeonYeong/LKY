#define NUM_EVENT_HISTORY	200

#define MAGIC_NUM 0x77777777
#define MAGIC_BASE ((volatile unsigned*)(0x20300000))
#define MAGIC_SIZE sizeof(unsigned int)
#define LOG_BASE ((volatile unsigned*)((char*)MAGIC_BASE+MAGIC_SIZE))

#define USER_MODE	0x10
#define IRQ_MASK	0x80
#define FIQ_MASK	0x40

#define BASE_DELTA		300
#define MAX_MISPREDIC_DIFF 	0	

#define UBOOT_RESET ((volatile unsigned*)(0xc7e00000))

typedef struct finger {
	unsigned int r0;
	unsigned int r1;
	unsigned int r2;
	unsigned int r3;
	unsigned int r4;
	unsigned int r5;
	unsigned int r6;
	unsigned int r7;
	unsigned int r8;
	unsigned int r9;
	unsigned int r10;
	unsigned int r11;
	unsigned int r12;
	unsigned int r13;
	unsigned int spsr;
} finger_print;

typedef struct vr_log {	
	unsigned int id;	// id
	unsigned int ic;	// instruction counter
	unsigned int mc;	// branch misprediction counter
	unsigned int pc;	// program counter
	unsigned char irq;	// interrupt id
	finger_print fp;	// finger print
} vr_log_t;

// Event history data structure for record/replay
struct event_history {
	unsigned int cur_event_id;
	vr_log_t event[NUM_EVENT_HISTORY];
};

struct event_history ev_history;

int mode;	// 0 for record mode, 1 for replay mode

void init_rec_event_struct(vr_log_t *ev);
void record_event_id(vr_log_t *ev, int int_type);
void vr_set_ic_irq(void);

unsigned int vk_hwi_count;
unsigned int vr_ic_int_cnt;
unsigned int vr_reg_fail_cnt[15];
unsigned int vr_bkpt_expt_cnt;
unsigned int *vr_data_ptr;

vr_log_t *current_log;		// current log structure pointer
unsigned int vr_replay_count;	// instruction counter for reproducing
unsigned int vr_fail_cnt;

unsigned char vr_replay_end_flag;

unsigned int race_var;

int context_count; 
