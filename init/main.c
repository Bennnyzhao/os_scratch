/*
     init/main.c		Reference to Linus Torvalds Linux-0.11
*/
#include <linux/tty.h>

void main(void)
{
    con_init();
    con_write("hello main! I am jumping in.\n");
    while(1);
}


