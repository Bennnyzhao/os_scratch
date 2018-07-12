/*
     init/main.c		Reference to Linus Torvalds Linux-0.11
*/
#include <linux/tty.h>
#include <linux/kernel.h>

long user_stack[4096>>2] ;

struct {
   long *a;
   short b;
}stack_start = {&user_stack[4096>>2], 0x10};

void main(void)
{
    int i=10;
    con_init();
    printk("hello printk %d\n", i);
    while(1);
}


