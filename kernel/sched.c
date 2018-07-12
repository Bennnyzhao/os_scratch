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

void sched_init(void)
{
    set_tss_desc(gdt+FIRST_TSS_ENTRY,(long)&tss0);
    set_ldt_desc(gdt+FIRST_LDT_ENTRY,ldt0);

    __asm__("pushfl ; andl $0xffffbfff,(%esp) ; popfl");
    ltr(0);
    lldt(0);
}
