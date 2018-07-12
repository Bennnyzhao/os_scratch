#ifndef _UNISTD_H
#define _UNISTD_H
#define __NR_setup	0	/* used only by init, to get system going */
#define __NR_fork	1
#define __NR_pause	2

#define _syscall0(type,name)\
type name(void)\
{\
long __res;\
__asm__ volatile("int $0x80"\
	:"=a"(__res)\
	:"0"(__NR_##name));\
if (__res>=0)\
    return (type)__res;\
return -1;\
}
//errno = -__res; 

int task_int(int num)
{
    long res=0;
    __asm__ volatile("int $0x80\n\t"
	:"=a"(res):"0"(__NR_setup),"b"(num));
    return (int)res;
}

int fork(void);
#endif

