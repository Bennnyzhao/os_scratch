/*
     init/main.c		Reference to Linus Torvalds Linux-0.11
*/
#include <linux/tty.h>
#include <linux/kernel.h>
#include <asm/io.h>
#include <asm/system.h>


#define HZ 100

#define LATCH (1193180/HZ)

void set_int(void);
extern int timer_interrupt(void);

typedef struct desc_struct {
	unsigned long a,b;
} desc_table[256];

extern desc_table idt;

long user_stack[4096>>2] ;

struct {
   long *a;
   short b;
}stack_start = {&user_stack[4096>>2], 0x10};

struct tm {
	int tm_sec;
	int tm_min;
	int tm_hour;
	int tm_mday;
	int tm_mon;
	int tm_year;
	int tm_wday;
	int tm_yday;
	int tm_isdst;
};


#define CMOS_READ(addr) ({ \
   outb_p(0x80|addr,0x70); \
   inb_p(0x71); \
})

#define BCD_TO_BIN(val) ((val)=((val)&15) + ((val)>>4)*10)

struct tm timenow;
static void get_time(void)
{
	//struct tm time;

	do {
		timenow.tm_sec = CMOS_READ(0);
		timenow.tm_min = CMOS_READ(2);
		timenow.tm_hour = CMOS_READ(4);
		timenow.tm_mday = CMOS_READ(7);
		timenow.tm_mon = CMOS_READ(8);
		timenow.tm_year = CMOS_READ(9);
	} while (timenow.tm_sec != CMOS_READ(0));
	BCD_TO_BIN(timenow.tm_sec);
	BCD_TO_BIN(timenow.tm_min);
	BCD_TO_BIN(timenow.tm_hour);
	BCD_TO_BIN(timenow.tm_mday);
	BCD_TO_BIN(timenow.tm_mon);
	BCD_TO_BIN(timenow.tm_year);
	//timenow.tm_mon--;
	//startup_time = kernel_mktime(&timenow);
}

unsigned long pos;
unsigned long x;
unsigned int flag=0;

void main(void)
{
    int i=10;
    con_init();
    set_int();
    sti();
    printk("hello printk %d\n", i);
    printk("time is ");
    pos = get_pos(&x);
    while(1){
      if(flag){
        flag=0;
        get_time();
        set_pos(pos, x);
        printk("%02d:%02d:%02d",timenow.tm_hour, timenow.tm_min, timenow.tm_sec);
      }
    }
}

void do_timer(void)
{
    //outb(0x20,0x20);
    flag=1;
}

void set_int(void)
{
    outb_p(0x36,0x43);
    outb_p(LATCH & 0xff , 0x40);
    outb(LATCH >> 8 , 0x40);	/* MSB */
    set_intr_gate(0x20,&timer_interrupt);
    outb(inb_p(0x21)&~0x01,0x21);
}


