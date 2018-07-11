.code16

.globl _start, begtext, begdata, begbss, endtext, enddata, endbss

.text
begtext:
.data 
begdata:
.bss
begbss:

.text
_start:
    mov $0x07c0, %ax
    mov %ax, %es
    
    mov $0x3, %ah
    xor %bh, %bh
    int $0x10
    
    mov $24, %cx
    mov $0x0007, %bx
    mov $msg1, %bp
    mov $0x1301, %ax
    int $0x10
    
loop:
    jmp loop
    
msg1:
      .byte 13, 10
      .ascii "Loading system ..."
      .byte 13, 10, 13, 10
    
.org 510
boot_flag:
    .word 0xAA55

.text
endtext:
.data
enddata:
.bss
endbss:
