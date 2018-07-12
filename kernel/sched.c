#include <linux/head.h>
#include <linux/sched.h>
#include <linux/sys.h>
#include <asm/system.h>
#include <asm/io.h>

#define LATCH (1193180/HZ)
extern int timer_interrupt(void);
extern int system_call(void);

static unsigned int flag=0;
void do_timer(void)
{
    dispaly_time();
    if(flag == 0){
	flag=1;
        switch_to(1);
    }else{
	flag=0;
        switch_to(0);
    }
}
union task_union {
	struct task_struct task;
	char stack[PAGE_SIZE];
};

static union task_union init_task = {INIT_TASK,};

long volatile jiffies=0;
long startup_time=0;
struct task_struct *current = &(init_task.task);
struct task_struct *last_task_used_math = NULL;

struct task_struct * task[NR_TASKS] = {&(init_task.task), };

long user_stack[PAGE_SIZE>>2] ;

struct {
   long *a;
   short b;
}stack_start = {&user_stack[PAGE_SIZE>>2], 0x10};

void sched_init(void)
{
    int i;
    struct desc_struct *p;
    set_tss_desc(gdt+FIRST_TSS_ENTRY,(long)&(init_task.task.tss));
    set_ldt_desc(gdt+FIRST_LDT_ENTRY,(long)&(init_task.task.ldt));

    p = gdt+2+FIRST_TSS_ENTRY;
    for(i=1;i<NR_TASKS;i++) {
	task[i] = NULL;
	p->a = p->b = 0;
	p++;
	p->a = p->b = 0;
	p++;
    }
    __asm__("pushfl ; andl $0xffffbfff,(%esp) ; popfl");
    ltr(0);
    lldt(0);

    outb_p(0x36,0x43);
    outb_p(LATCH & 0xff , 0x40);
    outb(LATCH >> 8 , 0x40);	/* MSB */
    set_intr_gate(0x20,&timer_interrupt);
    outb(inb_p(0x21)&~0x01,0x21);
    set_system_gate(0x80, &system_call);
}

