#ifndef _VPOS_MESSAGEQUEUE_H_
#define _VPOS_MESSAGEQUEUE_H_

#define MAXMSGSIZE  	20	//Max size of message
#define MQ_NUM 		3	//maximum num of mq
#define MAXTBLSIZE	7	//length of msgQueue table
#define MQ_SIZE 	10

typedef struct mq_tbl{
	unsigned char msg_q_index;
	unsigned char msg_q_flag;
	int tblIn;
	int tblOut;
	int QueueSize;
	int EntrySize;
	int MsgSize;
	int tbl_flag;
}vk_mqtbl_t;

typedef struct mq {
	int MsgIn;
	int MsgOut;
	int MsgSize;
	int MsgEntry;
#ifndef HSKIM // MPOS
	unsigned char thread_id;
	unsigned char thread_prio;
#endif
	char msg[MAXMSGSIZE];
}vk_mq_t;

typedef struct mq_attr {
//	long 	mq_flags;
	long 	mq_maxmsg; 	//Maxmum nmber of messages
	long 	mq_msgsize;	//Maximum message size
	long 	mq_curmsgs;	//Number of messages currently queued
}vk_mq_attr_t;

typedef int mqd_t;
vk_mq_attr_t vk_master_getattr;

#ifndef HSKIM // MPOS
//For Local Message Queue
vk_mqtbl_t msgQtbl[MQ_SIZE];
vk_mq_t msgQueue[MQ_SIZE][MAXTBLSIZE];
vk_mq_attr_t mq_attr[MQ_SIZE];

//For Kernel Call Handler
vk_mqtbl_t vk_master_msgQtbl[MQ_SIZE];
vk_mq_t vk_master_msgQueue[MQ_SIZE][MAXTBLSIZE];
vk_mq_attr_t vk_master_mq_attr[MQ_SIZE];
#endif

//For Local Message Queue
int mq_getattr(mqd_t mqdes, struct mq_attr *mqstat);
//int mq_notify(mqd_t mqdes, const struct sigevent *notification);
ssize_t mq_receive(mqd_t mqdes, char *msg_ptr, size_t msg_len, unsigned *msg_prio);
int mq_send(mqd_t mqdes, const char *msg_ptr, size_t msg_len, unsigned msg_prio);
//int mq_setattr(mqd_t mqdes, const struct mq_attr *restrict mqstat, struct mq_attr *restrict omqstat);
int mq_close(mqd_t mqdes);
mqd_t mq_open(const char *name, int oflag);
//int mq_unlink(const char *name);

#ifndef HSKIM // MPOS
//For Kernel Call Handler
int vk_kcall_mq_getattr(mqd_t mqdes, struct mq_attr *mqstat);
ssize_t vk_kcall_mq_receive(mqd_t mqdes, char *msg_ptr, size_t msg_len, unsigned *msg_prio);
int vk_kcall_mq_send(mqd_t mqdes, const char *msg_ptr, size_t msg_len, unsigned msg_prio);
int vk_kcall_mq_close(mqd_t mqdes);
mqd_t vk_kcall_mq_open(const char *name, int oflag);
#endif
#endif //_VPOS_MESSAGEQUEUE_H_
