/*
     init/main.c		Reference to Linus Torvalds Linux-0.11
*/
#include <unistd.h>
inline  _syscall0(int,fork) 

#include <linux/head.h>
#include <linux/tty.h>
#include <linux/kernel.h>
#include <asm/io.h>
#include <asm/system.h>
#include <linux/sched.h>

extern void mem_init(long start, long end);

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

#define EXT_MEM_K (*(unsigned short *)0x90002)

unsigned long pos;
unsigned long x;

static int task_demo(int num)
{
    int i=0;
    while(1){
	task_int(num);
	for(i=0; i<1000000;i++);
    }
    return 0;
}

static long memory_end = 0;
static long buffer_memory_end = 0;
static long main_memory_start = 0;

void main(void)
{
    int i=10;
    memory_end = (1<<20) + (EXT_MEM_K<<10);
    memory_end &= 0xfffff000;
    if (memory_end > 16*1024*1024)
	memory_end = 16*1024*1024;
    if (memory_end > 12*1024*1024) 
	buffer_memory_end = 4*1024*1024;
    else if (memory_end > 6*1024*1024)
	buffer_memory_end = 2*1024*1024;
    else
	buffer_memory_end = 1*1024*1024;
    main_memory_start = buffer_memory_end;
    mem_init(main_memory_start, memory_end);
    con_init();
    printk("hello printk %d\n", i);
    sched_init();
    sti();
    move_to_user_mode();
    if (!fork()) {
	task_demo(1);
	while(1);
    }
    task_demo(2);
    while(1);
}

void dispaly_time(void)
{   char i, *p;
    p = display_tm;
    get_time();
    i = timenow.tm_hour/10;
    *p = i+0x30;
    p+=2;
    i = timenow.tm_hour%10;
    *p = i+0x30;
    p+=2;
    *p= ':';
    p+=2;
    i = timenow.tm_min/10;
    *p = i+0x30;
    p+=2;
    i = timenow.tm_min%10;
    *p = i+0x30;
    p+=2;
    *p= ':';
    p+=2;
    i = timenow.tm_sec/10;
    *p = i+0x30;
    p+=2;
    i = timenow.tm_sec%10;
    *p = i+0x30;
}

void display_task(int num)
{
    printk("task %d\n", num);
}

