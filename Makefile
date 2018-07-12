AS	=as
LD	=ld

LDFLAGS	:=-m elf_i386 -Ttext 0 -e _start
CC = gcc 

HOST-CFLAGS = -Wall -O2 -fomit-frame-pointer
CFLAGS	=-Wall -O -fno-stack-protector -fstrength-reduce -fomit-frame-pointer \
	-finline-functions -nostdinc -Iinclude

CPP	=cpp -E -nostdinc -Iinclude

ROOT_DEV= FLOPPY

ARCHIVES=kernel/kernel.o mm/mm.o fs/fs.o
DRIVERS =kernel/blk_drv/blk_drv.a kernel/chr_drv/chr_drv.a
MATH	=kernel/math/math.a
LIBS	=lib/lib.a

.s.o:
	$(AS) --32 -o $*.o $<
	
.c.o:
	$(CC) $(CFLAGS) \
	-m32 -nostdinc -Iinclude -c -o $*.o $<

all: Image

Image: boot/boot tools/system tools/build
	objcopy -O binary -R .note -R .comment tools/system tools/kernel
	tools/build boot/boot tools/kernel $(ROOT_DEV) > Image
	rm tools/kernel -f
	sync

tools/build: tools/build.c
	$(CC) $(HOST-CFLAGS) \
	-o tools/build tools/build.c

boot/head.o: boot/head.s
	gcc -m32 -I./include -traditional -c boot/head.s
	mv head.o boot/

boot/boot.o: boot/boot.s

boot/setup.o: boot/setup.s

tools/system: boot/head.o init/main.o \
$(ARCHIVES) $(DRIVERS) $(MATH) $(LIBS)
	$(LD) $(LDFLAGS) boot/head.o init/main.o \
	$(ARCHIVES) $(DRIVERS) $(MATH) \
	$(LIBS) \
-o tools/system
	nm tools/system | grep -v '\(compiled\)\|\(\.o$$\)\|\( [aU] \)\|\(\.\.ng$$\)\|\(LASH[RL]DI\)'| sort > System.map

boot/boot: boot/boot.o boot/setup.o
	$(LD) $(LDFLAGS) boot/boot.o boot/setup.o -o boot/bootsect
	objcopy -O binary -R .note -R .comment boot/bootsect boot/boot
	rm boot/bootsect -f

kernel/math/math.a:
	(cd kernel/math;make)

kernel/blk_drv/blk_drv.a:
	(cd kernel/blk_drv; make)

kernel/chr_drv/chr_drv.a:
	(cd kernel/chr_drv; make)

kernel/kernel.o:
	(cd kernel; make)

mm/mm.o:
	(cd mm; make)

fs/fs.o:
	(cd fs; make)

lib/lib.a:
	(cd lib; make)

clean:
	rm -f Image System.map tmp_make core boot/boot
	rm -f init/*.o tools/system boot/*.o tools/build
	(cd mm;make clean)
	(cd fs;make clean)
	(cd kernel;make clean)
	(cd lib;make clean)

dep:
	sed '/\#\#\# Dependencies/q' < Makefile > tmp_make
	(for i in init/*.c;do echo -n "init/";$(CPP) -M $$i;done) >> tmp_make
	cp tmp_make Makefile
	(cd kernel; make dep)

### Dependencies:
init/main.o : init/main.c include/unistd.h include/sys/stat.h \
  include/sys/types.h include/sys/times.h include/sys/utsname.h \
  include/utime.h include/time.h include/linux/tty.h include/termios.h \
  include/linux/sched.h include/linux/head.h include/linux/fs.h \
  include/linux/mm.h include/signal.h include/asm/system.h include/asm/io.h \
  include/stddef.h include/stdarg.h include/fcntl.h 
