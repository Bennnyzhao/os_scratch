.code16

.equ BOOTSEG,  0x07c0
.equ INITSEG,  0x9000
.equ SETUPSEG, 0x9020
.equ SETUPLEN, 4
.equ SYSSIZE,  0x1000
.equ SYSSEG,   0x1200
.equ ENDSEG,   SYSSEG + SYSSIZE

.equ ROOT_DEV, 0x306

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
    mov sread, %ax
    add $SETUPLEN, %ax
    mov %ax, sread
    
    mov $0x00, %dl
    mov $0x0800, %ax
    int $0x13
    movb $0x00, %ch
    andb $0x3f, %cl
    mov %cx, sectors
    
    mov $INITSEG, %ax
    mov %ax, %es
  
    mov $0x3, %ah
    xor %bh, %bh
    int $0x10
    
    mov $35, %cx
    mov $0x0007, %bx
    mov $msg1, %bp
    mov $0x1301, %ax
    int $0x10
    
    mov $SYSSEG, %ax
    mov %ax, %es
    xor %bx, %bx
    movb $1, %al
    call read_track
    mov sread, %ax
    add $1, %ax
    mov %ax, sread
    
    mov $0x2000, %ax
    mov %ax, %es
    xor %bx, %bx
    movb $1, %al
    call read_track
    
    jmp _setup
    
status: .word 1
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
    addb $48, %ah
    movb %ah, status
    mov $INITSEG, %ax
    mov %ax, %es
    mov $0x3, %ah
    xor %bh, %bh
    int $0x10
    
    mov $1, %cx
    mov $0x0007, %bx
    mov $status, %bp
    mov $0x1301, %ax
    int $0x10
loop: jmp loop
    
msg1:
      .byte 13, 10
      .ascii "Hello boot sector, 512 byes ..."
      .byte 13, 10

sectors:
      .word 0
          
.org 508
root_dev:
    .word ROOT_DEV
boot_flag:
    .word 0xAA55

