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
    mov %ax, %ds
    lgdt gdt_48
    
    call empty_8042
    movb $0xD1, %al
    outb %al, $0x64
    call empty_8042
    movb $0xDF, %al
    outb %al, $0x60
    call empty_8042
    
    mov $0x0001, %ax
    lmsw %ax
    ljmp $8, $0
    
empty_8042:
    .word 0x00EB, 0x00EB
    inb $0x64, %al
    test $0x2, %al
    jnz empty_8042
    ret

gdt:
   .word 0, 0, 0, 0
   
   .word 0x07FF
   .word start_32      # modified
   .word 0x9A09        # modified
   .word 0x00C0
   
   .word 0x07FF
   .word 0x0000
   .word 0x9200
   .word 0x00C0
    
gdt_48:
    .word (0x800-1)
    .word gdt, 0x9
    
start_32:
.code32
    movl $0x10, %eax
    mov %ax, %ds
    movl $0xb8c80, %ebx
    mov $0x0c41, %dx
    movw %dx, (%ebx)
    add $0x2, %ebx
    mov $0x0c42, %dx
    movw %dx, (%ebx)

loop:
    jmp loop

