
#include <asm/system.h>

#include <linux/sched.h>
#include <linux/head.h>
#include <linux/kernel.h>

volatile void do_exit(long code);

static inline volatile void oom(void)
{
    printk("out of memory\n\r");
    do_exit(SIGSEGV);
}

#define invalidate() \
__asm__("movl %%eax, %%cr3"::"a"(0))

#define LOW_MEM		(0x100000)
#define PAGING_MEM	(15*1024*1024)
#define PAGING_PAGES	(PAGING_MEM >> 12)
#define MAP_NR(addr)	(((addr)-LOW_MEM)>>12)
#define USED		(100)

#define CODE_SPACE(addr) ((((addr)+4095)&~4095) < \
current->start_code + current->end_code)

#define copy_page(from, to)\
__asm__("cld; rep; movsl"::"S"(from), "D"(to), "c"(1024))

static long HIGH_MEMORY = 0;
static unsigned char mem_map[PAGING_PAGES]={0,};

unsigned long get_free_page(void)
{
    register unsigned long __res asm("ax");
    __asm__("std; repne; scasb\n\t"
	    "jne 1f\n\t"
	    "movb $1, 1(%%edi)\n\t"
	    "sall $12, %%ecx\n\t"
	    "addl %2, %%ecx\n\t"
	    "movl %%ecx, %%edx\n\t"
	    "movl $1024, %%ecx\n\t"
	    "leal 4092(%%edx), %%edi\n\t"
	    "rep; stosl\n\t"
	    "movl %%edx, %%eax\n"
	    "1:"
	    :"=a"(__res)
	    :"0"(0),"i"(LOW_MEM),"c"(PAGING_PAGES),
	    "D"(mem_map+PAGING_PAGES-1)
	    :"dx");
    return __res;
}

void free_page(unsigned long addr)
{
    if (addr < LOW_MEM) return;
    if (addr >= HIGH_MEMORY)
	panic("trying to free nonexist page");
    addr -= LOW_MEM;
    addr >>= 12;
    if (mem_map[addr]--) return;
    mem_map[addr]=0;
    panic("trying to free free page");
}

int free_page_tables(unsigned long from, unsigned long size)
{
    unsigned long *page_table;
    unsigned long *dir, nr;

    if (from & 0x3fffff)
	panic("free_page_tables called with wrong aligned");
    if (!from)
	panic("Trying to free up swapper memory space");
    size = (size+0x3fffff) >> 22;
    dir = (unsigned long*)((from >> 20) & 0xffc);
    for (; size-->0; dir++) {
	if (!(1&*dir))
	    continue;
	page_table= (unsigned long*)(0xfffff000 & *dir);
	for (nr=0; nr<1024; nr++) {
	    if (1&*page_table)
		free_page(0xfffff000 & *page_table);
	    *page_table = 0;
	    page_table++;
	}
	free_page(0xfffff000 & *dir);
	*dir = 0;
    }
    invalidate();
    return 0;
}

int copy_page_tables(unsigned long from, unsigned long to, long size)
{
    unsigned long *from_page_table;
    unsigned long *to_page_table;
    unsigned long this_page;
    unsigned long *from_dir, *to_dir;
    unsigned long nr;

    if ((from&0x3fffff) || (to&0x3fffff))
	panic("copy_page_tables called with wrong alignment");
    from_dir = (unsigned long *)((from >> 20)&0xffc);
    to_dir = (unsigned long *)((to >> 20)&0xffc);
    size = ((unsigned)(size+0x3fffff))>>22;
    for (; size-->0; from_dir++, to_dir++){
	if (1 & *to_dir)
	    panic("copy_page_tables already exist");
	if (!(1&*from_dir))
	    continue;
	from_page_table = (unsigned long *)(*from_dir & 0xfffff000);
	if (!(to_page_table=(unsigned long*)get_free_page()))
	    return -1;
	*to_dir = ((unsigned long)to_page_table) | 7;
	nr = (from==0)?0xA0:1024;
	for (; nr-->0; from_page_table++, to_page_table++){
	    this_page = *from_page_table;
	    if (!(1&this_page))
		continue;
	    this_page &= ~2;
	    *to_page_table = this_page;
	    if (this_page > LOW_MEM) {
		*from_page_table = this_page;
		this_page -= LOW_MEM;
		this_page >>= 12;
		mem_map[this_page]++;
	    }
	}
    }
    invalidate();
    return 0;
}

unsigned long put_page(unsigned long page, unsigned long address)
{
    unsigned long tmp, *page_table;
    
    if (page < LOW_MEM || page >= HIGH_MEMORY)
	printk("Trying to put page %p at %p\n", page, address);
    if (mem_map[(page-LOW_MEM)>>12] != 1)
	printk("mem_map disagrees with %p at %p\n", page, address);
    page_table = (unsigned long *)((address>>20)&0xffc);
    if (1 & *page_table)
	page_table = (unsigned long*)(*page_table & 0xfffff000);
    else {
	if (!(tmp=get_free_page()))
	    return 0;
	*page_table = tmp | 7;
	page_table = (unsigned long *)tmp;
    }
    page_table[(address>>12) & 0x3ff] = page | 7;
    return page;
}

void un_wp_page(unsigned long *table_entry)
{
    unsigned long old_page, new_page;

    old_page = 0xfffff000 & *table_entry;
    if (old_page >= LOW_MEM && mem_map[MAP_NR(old_page)]==1){
	*table_entry |= 2;
	invalidate();
	return;
    }

    if (!(new_page=get_free_page()))
	oom();
    if (old_page >= LOW_MEM)
	mem_map[MAP_NR(old_page)]--;
    *table_entry = new_page | 7;
    invalidate();
    copy_page(old_page, new_page);
}

void do_wp_page(unsigned long error_code, unsigned long address)
{
    un_wp_page((unsigned long*)
		(((address>>10)&0xffc) + (0xfffff000 &
		*((unsigned long*)((address>>20)&0xffc)))));
}

void write_verify(unsigned long address)
{
    unsigned long page;
    
    if (!((page=*((unsigned long *)((address>>20)&0xffc)))&1))
	return;
    page &= 0xfffff000;
    page += ((address>>10)&0xffc);
    if ((3&*(unsigned long*)page)==1)
	un_wp_page((unsigned long*)page);
    return;
}

void get_empty_page(unsigned long address)
{
    unsigned long tmp;

    if (!(tmp=get_free_page()) || !put_page(tmp, address)){
	free_page(tmp);
	oom();
    }
}

static int try_to_share(unsigned long address, struct task_struct *p)
{
    unsigned long from;
    unsigned long to;
    unsigned long from_page;
    unsigned long to_page;
    unsigned long phys_addr;

    from_page = to_page = ((address>>20)&0xffc);
    from_page += ((p->start_code>>20)&0xffc);
    to_page += ((current->start_code>>20)&0xffc);
    from = *(unsigned long*)from_page;
    if (!(from&1))
	return 0;
    from &= 0xfffff000;
    from_page = from + ((address>>10)&0xffc);
    phys_addr = *(unsigned long*)from_page;
    if ((phys_addr & 0x41) != 0x01)
	return 0;
    to = *(unsigned long*)to_page;
    if (!(to&1))
	if (to=get_free_page())
	    *(unsigned long*) to_page = to | 7;
	else
	    oom();

    to &= 0xfffff000;
    to_page = to + ((address>>10)&0xffc);
    if (1 & *(unsigned long*)to_page)
	panic("try_to_share: to_page already exists");
    *(unsigned long*) from_page &= ~2;
    *(unsigned long*) to_page = *(unsigned long*) from_page;
    invalidate();
    phys_addr -= LOW_MEM;
    phys_addr >>= 12;
    mem_map[phys_addr]++;
    return 1;
}

static int share_page(unsigned long address)
{
    struct task_struct **p;

    if (!current->executable)
	return 0;
    if (current->executable->i_count < 2)
	return 0;
    for (p=&LAST_TASK; p>&FIRST_TASK; --p) {
	if (!*p)
	    continue;
	if (current == *p)
	    continue;
	if ((*p)->executable != current->executable)
	    continue;
	if (try_to_share(address, *p))
	    return 1;
    }
    return 0;
}

void do_no_page(unsigned long error_code, unsigned long address)
{
    int nr[4];
    unsigned long tmp;
    unsigned long page;
    int block, i;

    address &= 0xfffff000;
    tmp = address - current->start_code;
    if (!current->executable || tmp >= current->end_data) {
	get_empty_page(address);
	return;
    }
    if (share_page(tmp))
	return;
    if (!(page = get_free_page()))
	oom();
    block = 1 + tmp/BLOCK_SIZE;
    for (i=0; i<4; block++, i++) {
	nr[i]=bmap(current->executable, block);
    }
    bread_page(page, current->executable->i_dev,nr);
    i = tmp + 4096 - current->end_data;
    tmp = page + 4096;
    while (i-- > 0) {
	tmp--;
	*(char*)tmp=0;
    }

    if (put_page(page, address))
	return;
    free_page(page);
    oom();
}

void mem_init(long start_mem, long end_mem)
{
    int i;
    HIGH_MEMORY = end_mem;
    for (i=0; i<PAGING_PAGES; i++)
	mem_map[i] = USED;
    i = MAP_NR(start_mem);
    end_mem -= start_mem;
    end_mem >>= 12;
    while (end_mem-->0)
	mem_map[i++] = 0;
}

