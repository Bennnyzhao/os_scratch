.code16

.equ BOOTSEG, 0x07c0
.equ INITSEG, 0x9000
.equ SETUPSEG, 0x9020
.equ SETUPLEN, 1

.globl _start

.text
_start:
    mov $BOOTSEG, %ax
    mov %ax, %ds
    mov $INITSEG, %ax
    mov %ax, %es
    mov $256, %cx
    xor %di, %di
    xor %si, %si
    rep
    movsw
    ljmp $INITSEG, $go

go:
    mov %cs, %ax
    mov %ax, %ds
    mov %ax, %es
    
load_setup:
    mov $0x0000, %dx
    mov $0x0002, %cx
    mov $0x0200, %bx
    mov $0x0200 + SETUPLEN, %ax
    int $0x13
    jnc ok_load_setup
    mov $0x0000, %dx
    mov $0x0000, %ax
    int $0x13
    jmp load_setup

ok_load_setup:  
    mov $0x3, %ah
    xor %bh, %bh
    int $0x10
    
    mov $35, %cx
    mov $0x0007, %bx
    mov $msg1, %bp
    mov $0x1301, %ax
    int $0x10
    
    #ljmp $SETUPSEG, $0
    jmp _setup
    
msg1:
      .byte 13, 10
      .ascii "Hello boot sector, 512 byes ..."
      .byte 13, 10
    
.org 510
boot_flag:
    .word 0xAA55

