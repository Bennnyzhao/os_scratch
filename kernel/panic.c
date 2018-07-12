#define PANIC
#include <linux/kernel.h>
#include <linux/sched.h>

volatile void panic(const char* s)
{
    printk("kernel panic: %s\n", s);
    for(;;);
}

