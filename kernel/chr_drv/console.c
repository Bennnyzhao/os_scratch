
#include <linux/tty.h>
#include <asm/io.h>
#include <asm/system.h>


#define ORIG_X			    (*(unsigned char *)0x90000)
#define ORIG_Y			    (*(unsigned char *)0x90001)
#define ORIG_VIDEO_PAGE		(*(unsigned short *)0x90004)
#define ORIG_VIDEO_MODE		((*(unsigned short *)0x90006) & 0xff)
#define ORIG_VIDEO_COLS 	(((*(unsigned short *)0x90006) & 0xff00) >> 8)
#define ORIG_VIDEO_LINES	(25)
#define ORIG_VIDEO_EGA_AX	(*(unsigned short *)0x90008)
#define ORIG_VIDEO_EGA_BX	(*(unsigned short *)0x9000a)
#define ORIG_VIDEO_EGA_CX	(*(unsigned short *)0x9000c)

#define VIDEO_TYPE_MDA		0x10	/* Monochrome Text Display	*/
#define VIDEO_TYPE_CGA		0x11	/* CGA Display 			*/
#define VIDEO_TYPE_EGAM		0x20	/* EGA/VGA in Monochrome Mode	*/
#define VIDEO_TYPE_EGAC		0x21	/* EGA/VGA in Color Mode	*/


static unsigned char	video_type;		/* Type of display being used	*/
static unsigned long	video_num_columns;	/* Number of text columns	*/
static unsigned long	video_size_row;		/* Bytes per row		*/
static unsigned long	video_num_lines;	/* Number of test lines		*/
static unsigned char	video_page;		    /* Initial video page		*/
static unsigned long	video_mem_start;	/* Start of video RAM		*/
static unsigned long	video_mem_end;		/* End of video RAM (sort of)	*/
static unsigned short	video_port_reg;		/* Video register select port	*/
static unsigned short	video_port_val;		/* Video register value port	*/
static unsigned short	video_erase_char;	/* Char+Attrib to erase with	*/

static unsigned long	origin;		/* Used for EGA/VGA fast scroll	*/
static unsigned long	scr_end;	/* Used for EGA/VGA fast scroll	*/
static unsigned long	pos;
static unsigned long	x,y;
static unsigned long	top,bottom;
static unsigned char	attr=0x07;

extern desc_table idt;
extern void keyboard_interrupt(void);


static inline void gotoxy(unsigned int new_x,unsigned int new_y)
{
	if (new_x > video_num_columns || new_y >= video_num_lines)
		return;
	x   = new_x;
	y   = new_y;
	pos = origin + y*video_size_row + (x<<1); /* 一个字符在内存中占2个字节的数据(属性占一个字节) */
}

static inline void set_origin(void)
{
	cli();
	outb_p(12, video_port_reg);
	outb_p(0xff&((origin-video_mem_start)>>9), video_port_val);
	outb_p(13, video_port_reg);
	outb_p(0xff&((origin-video_mem_start)>>1), video_port_val);
	sti();
}

static void scrup(void)
{
	if (video_type == VIDEO_TYPE_EGAC || video_type == VIDEO_TYPE_EGAM)
	{
		if (!top && bottom == video_num_lines) { /* 对应屏幕已满向上移动一行 */
			origin += video_size_row; 
			pos += video_size_row;
			scr_end += video_size_row;
			if (scr_end > video_mem_end) {
				__asm__("cld\n\t"
					"rep\n\t"
					"movsl\n\t"
					"movl video_num_columns,%1\n\t"
					"rep\n\t"
					"stosw"
					::"a" (video_erase_char),
					"c" ((video_num_lines-1)*video_num_columns>>1),
					"D" (video_mem_start),
					"S" (origin)
					);
				scr_end -= origin-video_mem_start;
				pos -= origin-video_mem_start;
				origin = video_mem_start;
			} else { /* 对应屏幕未满如回车换行 另起新行的操作 */
				__asm__("cld\n\t"
					"rep\n\t"
					"stosw"
					::"a" (video_erase_char),
					"c" (video_num_columns),
					"D" (scr_end-video_size_row)
					);
			}
			set_origin();
		} 
        else { /* 对应的操作为 删除一行 */
		
			__asm__("cld\n\t"
				"rep\n\t"
				"movsl\n\t"
				"movl video_num_columns,%%ecx\n\t"
				"rep\n\t"
				"stosw"
				::"a" (video_erase_char),
				"c" ((bottom-top-1)*video_num_columns>>1),
				"D" (origin+video_size_row*top),
				"S" (origin+video_size_row*(top+1))
				);
		}
	}
	else		/* Not EGA/VGA */
	{
		__asm__("cld\n\t"
			"rep\n\t"
			"movsl\n\t"
			"movl video_num_columns,%%ecx\n\t"
			"rep\n\t"
			"stosw"
			::"a" (video_erase_char),
			"c" ((bottom-top-1)*video_num_columns>>1),
			"D" (origin+video_size_row*top),
			"S" (origin+video_size_row*(top+1))
			);
	}
}


static void lf(void)
{
	if (y+1<bottom) {
		y++;
		pos += video_size_row;
		return;
	}
	scrup();
}

static void cr(void)
{
	pos -= x<<1;
	x=0;
}

static inline void set_cursor(void)
{
	cli();
	outb_p(14, video_port_reg);
	outb_p(0xff&((pos-video_mem_start)>>9), video_port_val);
	outb_p(15, video_port_reg);
	outb_p(0xff&((pos-video_mem_start)>>1), video_port_val);
	sti();
}

void set_pos(unsigned long position, unsigned long pos_x)
{
    pos = position;
    x   = pos_x;
}

unsigned long get_pos(unsigned long *pos_x)
{
    *pos_x = x;
    return pos;
}

void con_write(char *buf)
{
    while(*buf)
    {   
        if(*buf == 10)
        {
            cr();
            lf();
        }
        else
        { 
            if((*buf<32) || (*buf>126))
            {
                return;
            }

            if (x >= video_num_columns)
            {
                x   -= video_num_columns;
                pos -= video_size_row;
                lf();
            }
            __asm__("movb attr, %%ah\n\t"
                "movw %%ax, %1\n\t"
                ::"a"(*buf), "m"(*(short*)pos)
                );
            pos += 2;
            x++;
        }
        buf++;
    }

    set_cursor();
}

void con_init(void)
{
    register unsigned char a;
    char *display_desc = "????";
	char *display_ptr;

    video_num_columns = ORIG_VIDEO_COLS;
    video_size_row    = video_num_columns * 2;
    video_num_lines   = ORIG_VIDEO_LINES;
	video_page        = ORIG_VIDEO_PAGE;
	video_erase_char  = 0x0720;

    if(ORIG_VIDEO_MODE == 7)
    {
        video_mem_start = 0xb0000;
        video_port_reg  = 0x03b4;
		video_port_val  = 0x03b5;
        if ((ORIG_VIDEO_EGA_BX & 0xff) != 0x10)
        {
            video_type    = VIDEO_TYPE_EGAM;
			video_mem_end = 0xb8000;
			display_desc  = "EGAm";
        }
        else
        {
            video_type    = VIDEO_TYPE_MDA;
			video_mem_end = 0xb2000;
			display_desc  = "*MDA";
        }
    }
    else
    {
        video_mem_start = 0xb8000;
        video_port_reg  = 0x03d4;
		video_port_val  = 0x03d5;
        if ((ORIG_VIDEO_EGA_BX & 0xff) != 0x10)
        {
            video_type    = VIDEO_TYPE_EGAC;
			video_mem_end = 0xbc000;
			display_desc  = "EGAc";
        }
        else
        {
            video_type    = VIDEO_TYPE_CGA;
			video_mem_end = 0xba000;
			display_desc  = "*CGA";
        }
    }

    display_ptr = ((char*)video_mem_start) + video_size_row - 8;
    display_tm  = display_ptr - 18;
    while(*display_desc)
    {
        *display_ptr = *display_desc++;
        display_ptr += 2;
    }

    origin	= video_mem_start;
	scr_end	= video_mem_start + video_num_lines * video_size_row;
	top	    = 0;
	bottom	= video_num_lines;
    
    gotoxy(ORIG_X,ORIG_Y);
    
    set_trap_gate(0x21,&keyboard_interrupt);
	outb_p(inb_p(0x21)&0xfd,0x21);
	a=inb_p(0x61);
	outb_p(a|0x80,0x61);
	outb(a,0x61);
}

void do_tty_interrupt(void)
{
    //char *buf = kb_buf;
    //printk("keyboard\n");
    printk(kb_buf);
    //while(*buf)
    //{
    //    *buf = 0;
    //    buf++;
    //}
}


