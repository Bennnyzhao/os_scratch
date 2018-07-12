#include <linux/head.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/sys.h>
#include <asm/system.h>
#include <asm/io.h>

#include <signal.h>

void show_task(int nr, struct task_struct *p)
{
    int i,j=4096-sizeof(struct task_struct);
    printk("%d: pid=%d, state=%d, ",nr, p->pid,p->state);
    i=0;
    while(i<j && !((char*)(p+1))[i])
	i++;
    printk("%d (of %d) chars free in kernel stack\n\r",i,j);
}

void show_stat(void)
{
    int i;
    for (i=0; i<NR_TASKS; i++) {
	if (task[i])
	    show_task(i, task[i]);
    }
}

#define LATCH (1193180/HZ)

extern int timer_interrupt(void);
extern int system_call(void);

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

void math_state_restore()
{
	if (last_task_used_math == current)
		return;
	__asm__("fwait");
	if (last_task_used_math) {
		__asm__("fnsave %0"::"m" (last_task_used_math->tss.i387));
	}
	last_task_used_math=current;
	if (current->used_math) {
		__asm__("frstor %0"::"m" (current->tss.i387));
	} else {
		__asm__("fninit"::);
		current->used_math=1;
	}
}

void schedule(void)
{
    int i, next, c;
    struct task_struct **p;

    for (p=&LAST_TASK; p>&FIRST_TASK;--p)
	if (*p) {
	    if ((*p)->alarm && (*p)->alarm<jiffies) {
		(*p)->signal |= (1<<(SIGALRM-1));
		(*p)->alarm = 0;
	    }
	    if (((*p)->signal & ~(_BLOCKABLE & (*p)->blocked)) &&
		(*p)->state==TASK_INTERRUPTIBLE)
		(*p)->state=TASK_RUNNING;
	}

    while(1) {
	c = -1;
	next = 0;
	i = NR_TASKS;
	p = &task[NR_TASKS];
	while(--i) {
	    if (!*--p)
		continue;
	    if ((*p)->state == TASK_RUNNING && (*p)->counter > c)
		c = (*p)->counter, next = i;
	}
	if (c) break;
	for(p = &LAST_TASK ; p > &FIRST_TASK ; --p)
	    if (*p)
		(*p)->counter = ((*p)->counter >> 1) + (*p)->priority;
    }
    switch_to(next);
}


int sys_pause(void)
{
	current->state = TASK_INTERRUPTIBLE;
	schedule();
	return 0;
}

void sleep_on(struct task_struct **p)
{
	struct task_struct *tmp;

	if (!p)
		return;
	if (current == &(init_task.task))
		panic("task[0] trying to sleep");
	tmp = *p;
	*p = current;
	current->state = TASK_UNINTERRUPTIBLE;
	schedule();
	if (tmp)
		tmp->state=0;
}

void interruptible_sleep_on(struct task_struct **p)
{
	struct task_struct *tmp;

	if (!p)
		return;
	if (current == &(init_task.task))
		panic("task[0] trying to sleep");
	tmp=*p;
	*p=current;
repeat:	current->state = TASK_INTERRUPTIBLE;
	schedule();
	if (*p && *p != current) {
		(**p).state=0;
		goto repeat;
	}
	*p=NULL;
	if (tmp)
		tmp->state=0;
}

void wake_up(struct task_struct **p)
{
	if (p && *p) {
		(**p).state=0;
		*p=NULL;
	}
}

static int flag =1;
void do_timer(int cpl)
{
    if (cpl)
	current->utime++;
    else
	current->stime++;
    if ((--current->counter)>0) return;
	current->counter=0;
    if (!cpl) return;
    if(flag==0) {
	flag = 1;
	switch_to(0);
    }else{
	flag = 0;
	schedule();
    }
}

int sys_alarm(long seconds)
{
	int old = current->alarm;

	if (old)
		old = (old - jiffies) / HZ;
	current->alarm = (seconds>0)?(jiffies+HZ*seconds):0;
	return (old);
}

int sys_getpid(void)
{
	return current->pid;
}

int sys_getppid(void)
{
	return current->father;
}

int sys_getuid(void)
{
	return current->uid;
}

int sys_geteuid(void)
{
	return current->euid;
}

int sys_getgid(void)
{
	return current->gid;
}

int sys_getegid(void)
{
	return current->egid;
}

int sys_nice(long increment)
{
	if (current->priority-increment>0)
		current->priority -= increment;
	return 0;
}

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

