/*
 *  linux/kernel/system_call.s
 *
 *  (C) 1991  Linus Torvalds
 */

/*
 *  system_call.s  contains the system-call low-level handling routines.
 * This also contains the timer-interrupt handler, as some of the code is
 * the same. The hd- and flopppy-interrupts are also here.
 *
 * NOTE: This code handles signal-recognition, which happens every time
 * after a timer-interrupt and after each system call. Ordinary interrupts
 * don't handle signal-recognition, as that would clutter them up totally
 * unnecessarily.
 *
 * Stack layout in 'ret_from_system_call':
 *
 *	 0(%esp) - %eax
 *	 4(%esp) - %ebx
 *	 8(%esp) - %ecx
 *	 C(%esp) - %edx
 *	10(%esp) - %fs
 *	14(%esp) - %es
 *	18(%esp) - %ds
 *	1C(%esp) - %eip
 *	20(%esp) - %cs
 *	24(%esp) - %eflags
 *	28(%esp) - %oldesp
 *	2C(%esp) - %oldss
 */

/*
 * Ok, I get parallel printer interrupts while using the floppy for some
 * strange reason. Urgel. Now I just ignore them.
 */

.globl timer_interrupt

.align 2
timer_interrupt:
   push %ds
   push %es
   push %fs
   pushl %edx
   pushl %ecx
   pushl %ebx
   pushl %eax
   movl $0x10, %eax
   mov %ax, %ds
   mov %ax, %es
   #movl $0x17, %eax   # 设置fs段 系统会不断重启 不知道为啥 （因为此时还没有设置ldt）
   #mov %ax, %fs
   #incl jiffies
   movb $0x20, %al
   outb %al, $0x20
   #movl CS(%esp), %eax
   #andl $0x3, %eax
   #pushl %eax
   call do_timer
   #addl $4, %esp
   popl %eax
   popl %ebx
   popl %ecx
   popl %edx
   pop %fs
   pop %es
   pop %ds
   iret



