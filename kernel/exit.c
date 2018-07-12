#include <errno.h>
#include <signal.h>
#include <sys/wait.h>

#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/tty.h>
#include <asm/segment.h>

int sys_pause(void);
int sys_close(int fd);

void release(struct task_struct *p)
{
    int i;
    if (!p)
	return;
    for (i=1; i<NR_TASKS; i++)
	if (task[i]==p) {
	    task[i]=NULL;
	    free_page((long)p);
	    schedule();
	    return;
	}
    panic("trying to release non-existent task");
}

static inline int send_sig(long sig, struct task_struct *p, int priv)
{
    if (!p || sig<1 || sig>32)
	return -EINVAL;
    if (priv || (current->euid==p->euid) || suser())
	p->signal |= (1<<(sig-1));
    else
	return -EPERM;
    return 0;
}

static void kill_session(void)
{
    struct task_struct **p = NR_TASKS + task;

    while (--p > &FIRST_TASK) {
	if (*p && (*p)->session == current->session)
	    (*p)->signal |= 1 << (SIGHUP-1);
    }
}

int sys_kill(int pid, int sig)
{
    struct task_struct **p = NR_TASKS + task;
    int err, retval = 0;

    if (!pid)
	while (--p > &FIRST_TASK) {
	    if (*p && (*p)->pgrp == current->pid)
		if ((err = send_sig(sig,*p,1)))
		    retval = err;
	}
    else if (pid > 0)
	while (--p > &FIRST_TASK) {
	    if (*p && (*p)->pid == pid)
		if ((err=send_sig(sig,*p,0)))
		    retval = err;
	}
   else if (pid == -1) 
	while (--p > &FIRST_TASK) {
	    if (*p )
		if ((err=send_sig(sig,*p,0)))
		    retval = err;
	}
    else
	while (--p > &FIRST_TASK) {
	    if (*p && (*p)->pgrp == -pid)
		if ((err=send_sig(sig,*p,0)))
		    retval = err;
	}
    return retval;
}

static void tell_father(int pid)
{
    int i;

    if (pid)
	for (i=0; i<NR_TASKS; i++) {
	    if (!task[i])
		continue;
	    if (task[i]->pid != pid)
		continue;
	    task[i]->signal |= (1<<(SIGCHLD-1));
	    return;
	}
    printk("BAD BAD - no father found\n\r");
    release(current);
}

int do_exit(long code)
{
    int i;
    free_page_tables(get_base(current->ldt[1]),get_limit(0x0f));
    free_page_tables(get_base(current->ldt[2]),get_limit(0x17));
    for (i=0; i<NR_TASKS; i++)
	if (task[i] && task[i]->father == current->pid) {
	    task[i]->father = 1;
	    if (task[i]->state == TASK_ZOMBIE)
		(void) send_sig(SIGCHLD,task[1],1);
	}
   /* for (i=0; i<NR_OPEN; i++)
	if (current->filp[i])
	    sys_close(i);
    */
    if (last_task_used_math == current)
	last_task_used_math = NULL;
    if (current->leader)
	kill_session();
    current->state = TASK_ZOMBIE;
    current->exit_code = code;
    tell_father(current->father);
    schedule();
    return (-1);
}

int sys_exit(int error_code)
{
    return do_exit((error_code&0xff)<<8);
}

int sys_waitpid(pid_t pid, unsigned long *stat_addr, int options)
{
    int flag, code;
    struct task_struct **p;

    verify_area(stat_addr,4);
repeat:
    flag=0;
    for (p=&LAST_TASK; p>&FIRST_TASK; --p) {
	if (!*p || *p == current)
	    continue;
	if ((*p)->father != current->pid)
	    continue;
	if (pid > 0) {
	    if ((*p)->pid != pid)
	    continue;
	}else if (!pid) {
	    if ((*p)->pgrp != current->pgrp)
	    continue;
	}else if (pid != -1) {
	    if ((*p)->pgrp != -pid)
		continue;
	}
	switch ((*p)->state) {
	    case TASK_STOPPED:
		if (!(options & WUNTRACED))
		    continue;
		put_fs_long(0x7f,stat_addr);
		return (*p)->pid;
	    case TASK_ZOMBIE:
		current->cutime += (*p)->utime;
		current->cstime += (*p)->stime;
		flag = (*p)->pid;
		code = (*p)->exit_code;
		release(*p);
		put_fs_long(code,stat_addr);
		return flag;
	    default:
		flag = 1;
		continue;
	}
    }
    if (flag) {
	if (options & WNOHANG)
	    return 0;
	current->state = TASK_INTERRUPTIBLE;
	schedule();
	if (!(current->signal &= ~(1<<(SIGCHLD-1))))
	    goto repeat;
	else
	    return -EINTR;
    }
    return -ECHILD;
}
