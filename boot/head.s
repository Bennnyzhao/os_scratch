#.code32
.globl _start

.text
_start:
    movl $0x10, %eax
    mov %ax, %ds
    movl addr, %ebx
    mov $0x0c41, %dx
    movw %dx, (%ebx)
    add $0x2, %ebx
    mov $0x0c42, %dx
    movw %dx, (%ebx)
loop:
    jmp loop
    
.align 2
addr:
    .long 0xb8c80
    