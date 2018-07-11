.code16
.equ INITSEG, 0x9000

.globl _setup

.text
_setup:
   # mov $INITSEG, %ax
   # mov %ax, %ds
   # mov %ax, %es
    
    mov $0x3, %ah
    xor %bh, %bh
    int $0x10
    
    mov $21, %cx
    mov $0x0007, %bx
    mov $msg1, %bp
    mov $0x1301, %ax
    int $0x10
    
loop:
    jmp loop

msg1:
    .ascii "Beyond 512 byes ..."
    .byte 13, 10
    
.org 2048

    