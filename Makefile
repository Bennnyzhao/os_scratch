AS	=as
LD	=ld

LDFLAGS	:=-m elf_i386 -Ttext 0 -e _start
CC = gcc 

HOST-CFLAGS = -Wall -O2 -fomit-frame-pointer
CFLAGS = -Wall -fomit-frame-pointer

CPP	=cpp -nostdinc -Iinclude

ROOT_DEV= #FLOPPY

.s.o:
	$(AS) --32 -o $*.o $<
	
.c.o:
	$(CC) $(CFLAGS) \
	-m32 -nostdinc -I./include -c -o $*.o $<

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

tools/system: boot/head.o init/main.o
	$(LD) $(LDFLAGS) boot/head.o init/main.o -o tools/system
	nm tools/system | grep -v '\(compiled\)\|\(\.o$$\)\|\( [aU] \)\|\(\.\.ng$$\)\|\(LASH[RL]DI\)'| sort > System.map

boot/boot: boot/boot.o boot/setup.o
	$(LD) $(LDFLAGS) boot/boot.o boot/setup.o -o boot/bootsect
	objcopy -O binary -R .note -R .comment boot/bootsect boot/boot
	rm boot/bootsect -f

clean:
	rm -f Image System.map boot/boot
	rm -f init/*.o tools/system boot/*.o tools/build

dep:
	sed '/\#\#\# Dependencies/q' < Makefile > tmp_make
	(for i in init/*.c;do echo -n "init/";$(CPP) -M $$i;done) >> tmp_make
	cp tmp_make Makefile

### Dependencies:
init/main.o: init/main.c
