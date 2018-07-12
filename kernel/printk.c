
#include <stdarg.h>
#include <linux/tty.h>
#include <linux/kernel.h>

static char buf[1024];

extern int vsprintf(char * buf, const char * fmt, va_list args);

int printk(const char * fmt, ...)
{
    va_list args;
    int i;
    
    va_start(args, fmt);
    i = vsprintf(buf, fmt, args);
    va_end(args);

    con_write(buf);

    return i;
}

