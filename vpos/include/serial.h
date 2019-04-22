
#define SERIAL_BUFF_SIZE	100

void putc(char c);
char getc(void);
#ifndef MYSEO
int tstc(void);
#endif
void vh_serial_init(void);
void vh_serial_irq_enable(void);

char serial_buff[SERIAL_BUFF_SIZE];
unsigned int push_idx, pop_idx;
