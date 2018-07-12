#ifndef _SYS_H
#define _SYS_H
extern int display_task();
extern int sys_fork();
fn_ptr sys_call_table[] = { display_task,sys_fork};
#endif

