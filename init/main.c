/*
     init/main.c		Reference to Linus Torvalds Linux-0.11
*/
#define __LIBRARY__
#include <unistd.h>
#include <time.h>

inline _syscall0(int,fork) 
inline _syscall0(int,pause)

#include <linux/head.h>
#include <linux/tty.h>
//#include <linux/kernel.h>
#include <asm/io.h>
#include <asm/system.h>
#include <linux/sched.h>

#include <stdarg.h>

extern void mem_init(long start, long end);

#define CMOS_READ(addr) ({ \
   outb_p(0x80|addr,0x70); \
   inb_p(0x71); \
})

#define BCD_TO_BIN(val) ((val)=((val)&15) + ((val)>>4)*10)

static void time_init(void)
{
	struct tm timenow;

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
	timenow.tm_mon--;
	startup_time = kernel_mktime(&timenow);
}

#define EXT_MEM_K (*(unsigned short *)0x90002)

static int task_demo(int num);

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
    trap_init();
    con_init();
    time_init();
    printk("hello printk %d\n", i);
    sched_init();
    sti();
    move_to_user_mode();
    if (!fork()) {
	task_demo(1);
	while(1);
    }

    for(;;) pause();
}

static int task_demo(int num)
{
    int i=0;
    while(1){
	task_int(num);
	for(i=0; i<1000000;i++);
    }
    return 0;
}

void display_task(int num)
{
    printk("task %d\n", num);
}

