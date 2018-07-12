.code16
.equ INITSEG, 0x9000

.globl _setup

.text
_setup:
    mov $INITSEG, %ax
    mov %ax, %ds
    
    mov $0x3, %ah
    xor %bh, %bh
    int $0x10
    mov %dx, (0)
    
    movb $0x88, %ah
    int $0x15
    mov %ax, (2)
    
    movb $0x0f, %ah
    int $0x10
    mov %bx, (4)
    mov %ax, (6)
    
    movb $0x12, %ah
    movb $0x10, %bl
    int $0x10
    mov %ax, (8)
    mov %bx, (10)
    mov %cx, (12)
    
    mov $0x0000, %ax
    mov %ax, %ds
    lds (4*0x41), %si
    mov $INITSEG, %ax
    mov %ax, %es
    mov $0x80, %di
    mov $0x10, %cx
    rep
    movsb
    
    mov $0x0000, %ax
    mov %ax, %ds
    lds (4*0x46), %si
    mov $INITSEG, %ax
    mov %ax, %es
    mov $0x90, %di
    mov $0x10, %cx
    rep
    movsb
    
    mov $0x1500, %ax
    movb $0x81, %dl
    int $0x13
    jc no_disk1
    cmpb $0x3, %ah
    je is_disk1
    
no_disk1: 
    mov $INITSEG, %ax
    mov %ax, %es
    mov $0x90, %di
    mov $0x10, %cx
    mov $0, %ax
    rep
    stosb
    
is_disk1: 
    mov $INITSEG, %ax
    mov %ax, %es 
    
    mov $0x3, %ah
    xor %bh, %bh
    int $0x10
    
    mov $19, %cx
    mov $0x0007, %bx
    mov $msg1, %bp
    mov $0x1301, %ax
    int $0x10
    
loop:
    jmp loop

msg1:
    .ascii "Loading setup ..."
    .byte 13, 10
   