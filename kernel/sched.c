#include <linux/head.h>
#include <linux/sched.h>
#include <asm/system.h>

struct desc_struct ldt0[3] = {
    {0,0}, 
    {0x9f,0xc0fa00}, 
    {0x9f,0xc0f200}, 
};

#define PAGE_SIZE 4096
char stack0[PAGE_SIZE];
struct tss_struct tss0 = {
    0,PAGE_SIZE+(long)stack0,0x10,0,0,0,0,(long)pg_dir,
    0,0,0,0,0,0,0,0, 
    0,0,0x17,0x0f,0x17,0x17,0x17,0x17, 
    _LDT(0),0x80000000,
};

struct desc_struct ldt1[3] = {
    {0,0}, 
    {0x9f,0xc0fa00}, 
    {0x9f,0xc0f200}, 
};

static int task1(void)
{
    while(1){
	task_int(1);
    }
}

char stack1[PAGE_SIZE];
char usr_stack[PAGE_SIZE];
struct tss_struct tss1 = {
    0,PAGE_SIZE+(long)stack1,0x10,0,0,0,0,(long)pg_dir,
    &task1,0x200,0,0,0,0,PAGE_SIZE+(long)usr_stack,0, 
    0,0,0x17,0x0f,0x17,0x17,0x17,0x17, 
    _LDT(1),0x80000000,
};
extern int system_call(void);
void sched_init(void)
{
    set_tss_desc(gdt+FIRST_TSS_ENTRY,(long)&tss0);
    set_ldt_desc(gdt+FIRST_LDT_ENTRY,ldt0);

    set_tss_desc(gdt+FIRST_TSS_ENTRY+2,(long)&tss1);
    set_ldt_desc(gdt+FIRST_LDT_ENTRY+2,ldt1);
    __asm__("pushfl ; andl $0xffffbfff,(%esp) ; popfl");
    ltr(0);
    lldt(0);

    set_system_gate(0x80, &system_call);
}

