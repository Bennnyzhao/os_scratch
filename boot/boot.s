.code16

.equ BOOTSEG,  0x07c0
.equ INITSEG,  0x9000
.equ SETUPSEG, 0x9020
.equ SETUPLEN, 4
.equ SYSSIZE,  0x3000
.equ SYSSEG,   0x1000
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
    call kill_motor
    
    mov %cs:root_dev, %ax
    cmp $0, %ax
    jne root_defined
    mov %cs:sectors, %bx
    mov $0x0208, %ax
    cmp $15, %bx
    je root_defined
    mov $0x021c, %ax
    cmp $18, %bx
    je root_defined
undef_root:
    jmp undef_root
 
root_defined:
    mov %ax, %cs:root_dev
    
    #ljmp $SETUPSEG, $0
    jmp _setup

sread:	.word 1
head:	  .word 0
track:	.word 0

read_it:
    mov %es, %ax
    test $0x0fff, %ax
die:
    jne die
    
    xor %bx, %bx
rep_read:
    mov %es, %ax
    cmp $ENDSEG, %ax
    jb ok_read1
    ret

ok_read1:
    #mov %cs:sectors, %ax
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
    add sread, %ax
    #cmp %cs:sectors, %ax
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
    mov $0x0000, %dx
    mov $0x0000, %ax
    int $0x13
    pop %dx
    pop %cx
    pop %bx
    pop %ax
    jmp read_track

kill_motor:
    push %dx
    mov $0x3f2, %dx
    movb $0, %al
    outsb 
    pop  %dx
    ret
    
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

