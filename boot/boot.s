.code16

.equ BOOTSEG, 0x07c0
.equ INITSEG, 0x9000

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
    
    mov $0x00, %dl
    mov $0x0800, %ax
    int $0x13
    movb $0x00, %ch
    #andb $0x3f, %cl
    #mov $0x4142, %cx
    mov %cx, sectors
    #mov %cx, %cs:sectors
    
    mov $INITSEG, %ax
    mov %ax, %es
    
    mov $msg1, %di
    mov sectors, %ax
    mov %ax, 2(%di)
    #mov %ax, (%si)
    #movsw
      
    mov $0x3, %ah
    xor %bh, %bh
    int $0x10
    
    mov $35, %cx
    mov $0x0007, %bx
    mov $msg1, %bp
    mov $0x1301, %ax
    int $0x10
    
loop:
    jmp loop
    
msg1:
      .byte 13, 10
      .ascii "Hello boot sector, 512 byes ..."
      .byte 13, 10
      
sectors:
      .word 0
    
.org 510
boot_flag:
    .word 0xAA55

