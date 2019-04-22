#ifndef _VPOS_DD_H_
#define _VPOS_DD_H_

#include "linux/types.h"

#define NO_INT -1

typedef struct file_operations{
	int (*vk_open)(char *file_name);
	int (*vk_release)(int fd);
	int (*vk_read)(int fd, char *buf, size_t count);
	int (*vk_write)(int fd, char *buf, size_t count);
	int (*vk_ioctl)(int fd, unsigned int cmd, unsigned long arg);
	int (*vk_interrupt)(void);
} vk_file_operations_t;

typedef struct vk_device_driver_register_strcut{
	char *name;
	unsigned int register_dev;
	int interrupt_number;
	vk_file_operations_t *fop_list;
} vk_ddrs_t;

vk_ddrs_t vk_dd_table[55];

unsigned int vk_idt_table[160];

void vk_init_dd_table(void);

int vu_register_dev(unsigned int dev_number, char *dev_name, struct file_operations *fops, int irq_num);
void vu_unregister_dev(unsigned int dev_number, char *dev_name);
int open(char *file_name);
int close(int fd);
int read(int fd, char *buf, size_t count);
int write(int fd, char *buf, size_t count);
int ioctl(int fd, unsigned int cmd, unsigned long arg);

#ifndef MYSEO
int copy_from_user (void *to, const void *from_user, unsigned long len);
int copy_to_user (void *to_user, const void *from, unsigned long len);
#endif

#endif //_VPOS_DD_H_
