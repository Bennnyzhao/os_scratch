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
    call read_it
    
    jmp _setup

status: .byte 0
sread:	.word 1
head:	  .word 0
track:	.word 0
flag:   .word 0

read_it:
    #mov %es, %ax
    #test $0x0fff, %ax
#die:
    #jne die
    
    xor %bx, %bx
rep_read:
    mov %es, %ax
    cmp $ENDSEG, %ax
    jb ok_read1
read_ret:
    ret

ok_read1:
    mov sectors, %ax
    sub sread, %ax
    mov %ax, %cx
    shl $9,  %cx
    add %bx, %cx
    jnc ok_read2
    je ok_read2
    xor %ax, %ax
    sub %bx, %ax
    shr $9, %ax

ok_read2:
    call read_track
    
    mov %ax, %cx
    mov flag, %ax
    add $1, %ax
    cmp $7, %ax
    je read_ret
    mov %ax, flag
    mov %cx, %ax
    add sread, %ax
    cmp sectors, %ax
    jne ok_read3
    mov $1, %ax
    sub head, %ax
    jne ok_read4
    incw track
    
ok_read4:
    mov %ax, head
    xor %ax, %ax
    
ok_read3:
    mov %ax, sread
    shl $9,  %cx
    add %cx, %bx
    jnc ok_read1
    jmp read_ret
    
    mov %es, %ax
    add $0x1000, %ax
    mov %ax, %es
    xor %bx, %bx
    jmp rep_read
    
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

