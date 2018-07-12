
#define sti() __asm__ ("sti"::)        // 开中断嵌入汇编宏函数。set interrupt
#define cli() __asm__ ("cli"::)        // 关中断。clear interrupt
#define nop() __asm__ ("nop"::)        // 空操作。


