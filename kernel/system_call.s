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

SIG_CHLD	= 17

EAX		= 0x00
EBX		= 0x04
ECX		= 0x08
EDX		= 0x0C
FS		= 0x10
ES		= 0x14
DS		= 0x18
EIP		= 0x1C
CS		= 0x20
EFLAGS		= 0x24
OLDESP		= 0x28
OLDSS		= 0x2C

state	= 0		# these are offsets into the task-struct.
counter	= 4
priority = 8
signal	= 12
sigaction = 16		# MUST be 16 (=len of sigaction)
blocked = (33*16)

# offsets within sigaction
sa_handler = 0
sa_mask = 4
sa_flags = 8
sa_restorer = 12

nr_system_calls = 72
/*
 * Ok, I get parallel printer interrupts while using the floppy for some
 * strange reason. Urgel. Now I just ignore them.
 */
.globl system_call,sys_fork
.globl timer_interrupt
.globl parallel_interrupt
.globl device_not_available, coprocessor_error

.align 4
bad_sys_call:
    movl $-1, %eax
    iret
.align 4
reschedule:
    pushl $ret_from_sys_call
    jmp schedule

.align 4
system_call:
    cmpl $nr_system_calls-1, %eax
    ja bad_sys_call
    push %ds
    push %es
    push %fs
    pushl %edx
    pushl %ecx
    pushl %ebx
    movl $0x10,%edx
    mov %dx,%ds
    mov %dx,%es
    movl $0x17,%edx
    mov %dx,%fs
    call sys_call_table(,%eax,4)
    pushl %eax
    movl current,%eax
    cmpl $0, state(%eax)
    jne reschedule
    cmpl $0, counter(%eax)
    je reschedule
ret_from_sys_call:
    movl current, %eax
    cmpl task, %eax
    je 3f
    cmpw $0x0f, CS(%esp)
    jne 3f
    cmpw $0x17, OLDSS(%esp)
    jne 3f
    movl signal(%eax), %ebx
    movl blocked(%eax), %ecx
    notl %ecx
    andl %ebx, %ecx
    bsfl %ecx, %ecx
    je 3f
    btrl %ecx, %ebx
    movl %ebx, signal(%eax)
    incl %ecx
    pushl %ecx
    call do_signal
    popl %eax
3:  popl %eax
    popl %ebx
    popl %ecx
    popl %edx
    pop %fs
    pop %es
    pop %ds
    iret

.align 4
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
   movl $0x17, %eax   # 设置fs段 系统会不断重启 不知道为啥 （因为此时还没有设置ldt）
   mov %ax, %fs
   incl jiffies
   movb $0x20, %al
   outb %al, $0x20
   movl CS(%esp), %eax
   andl $0x3, %eax
   pushl %eax
   call do_timer
   addl $4, %esp
   popl %eax
   popl %ebx
   popl %ecx
   popl %edx
   pop %fs
   pop %es
   pop %ds
   iret

.align 4
coprocessor_error:
	push %ds
	push %es
	push %fs
	pushl %edx
	pushl %ecx
	pushl %ebx
	pushl %eax
	movl $0x10,%eax
	mov %ax,%ds
	mov %ax,%es
	movl $0x17,%eax
	mov %ax,%fs
	pushl $ret_from_sys_call
	jmp math_error

.align 4
device_not_available:
	push %ds
	push %es
	push %fs
	pushl %edx
	pushl %ecx
	pushl %ebx
	pushl %eax
	movl $0x10,%eax
	mov %ax,%ds
	mov %ax,%es
	movl $0x17,%eax
	mov %ax,%fs
	pushl $ret_from_sys_call
	clts				# clear TS so that we can use math
	movl %cr0,%eax
	testl $0x4,%eax			# EM (math emulation bit)
	je math_state_restore
	pushl %ebp
	pushl %esi
	pushl %edi
	call math_emulate
	popl %edi
	popl %esi
	popl %ebp
	ret
.align 4
sys_fork:
    call find_empty_process
    testl %eax, %eax
    js 1f
    push %gs
    pushl %esi
    pushl %edi
    pushl %ebp
    pushl %eax
    call copy_process
    addl $20, %esp
1:  ret

parallel_interrupt:
	pushl %eax
	movb $0x20,%al
	outb %al,$0x20
	popl %eax
	iret

