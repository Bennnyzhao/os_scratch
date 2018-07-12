#ifndef _SYS_H
#define _SYS_H
extern int display_task();
extern int sys_fork();
extern int sys_pause();
fn_ptr sys_call_table[] = { display_task,sys_fork,sys_pause};
#endif

