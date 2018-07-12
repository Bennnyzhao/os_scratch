
AR	=ar
AS	=as --32
LD	=ld
LDFLAGS	=-m elf_i386 -s -x
CC	=gcc -m32
CFLAGS	=-Wall -O -fno-stack-protector -fstrength-reduce -fomit-frame-pointer \
	-finline-functions -nostdinc -I../include
CPP	=gcc -E -nostdinc -I../include

.c.s:
	$(CC) $(CFLAGS) \
	-S -o $*.s $<
.s.o:
	$(AS) -o $*.o $<
.c.o:
	$(CC) $(CFLAGS) \
	-c -o $*.o $<

OBJS  = printk.o vsprintf.o

kernel.o: $(OBJS)
	$(LD) $(LDFLAGS) -r -o kernel.o $(OBJS)
	sync

clean:
	rm -f core *.o *.a tmp_make
	for i in *.c;do rm -f `basename $$i .c`.s;done
	(cd chr_drv; make clean)

dep:
	sed '/\#\#\# Dependencies/q' < Makefile > tmp_make
	(for i in *.c;do echo -n `echo $$i | sed 's,\.c,\.s,'`" "; \
		$(CPP) -M $$i;done) >> tmp_make
	cp tmp_make Makefile
	(cd chr_drv; make dep)

### Dependencies:
printk.s printk.o: printk.c ../include/stdarg.h \
  ../include/linux/kernel.h
vsprintf.s vsprintf.o: vsprintf.c ../include/stdarg.h ../include/string.h
