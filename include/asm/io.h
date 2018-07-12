/* 在前面的main.c中已经注释过 现在放在该文件内 */
#define outb_p(value,port) \
    __asm__ ("outb %%al,%%dx\n" \
            "\tjmp 1f\n" \
            "1:\tjmp 1f\n" \
            "1:"::"a" (value),"d" (port))
    
#define inb_p(port) ({ \
    unsigned char _v; \
    __asm__ volatile ("inb %%dx,%%al\n" \
        "\tjmp 1f\n" \
        "1:\tjmp 1f\n" \
        "1:":"=a" (_v):"d" (port)); \
    _v; \
    })


