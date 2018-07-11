.code16

.equ BOOTSEG, 0x07c0
.equ INITSEG, 0x9000
.equ SETUPSEG, 0x9020
.equ SETUPLEN, 4

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
    mov %ax, %ss
    mov $0xFF00, %sp
    
    mov $0x0200, %bx
    movb $SETUPLEN, %al
    call read_track
  
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

sread:	.word 1
head:	  .word 0
track:	.word 0
    
read_track:
    push %ax
    push %bx
    push %cx
    push %dx
    mov  track, %dx
    mov  sread, %cx
    inc  %cx
    movb %dl, %ch
    mov  head, %dx
    movb %dl, %dh
    movb $0, %dl
    and  $0x0100, %dx
    movb $2, %ah
    int  $0x13
    jc   bad_rt
    pop  %dx
    pop  %cx
    pop  %bx
    pop  %ax
    ret
    
bad_rt:
    mov $0x0000, %dx
    mov $0x0000, %ax
    int $0x13
    pop %dx
    pop %cx
    pop %bx
    pop %ax
    jmp read_track
    
msg1:
      .byte 13, 10
      .ascii "Hello boot sector, 512 byes ..."
      .byte 13, 10
    
.org 510
boot_flag:
    .word 0xAA55

