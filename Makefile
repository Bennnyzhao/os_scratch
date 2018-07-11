AS	=as
LD	=ld
LDFLAGS	=-m elf_i386 -Ttext 0 -e _start
CC = gcc 

CFLAGS = -Wall -O2 -fomit-frame-pointer

#
# ROOT_DEV specifies the default root-device when making the image.
# This can be either FLOPPY, /dev/xxxx or empty, in which case the
# default of /dev/hd6 is used by 'build'.
#
ROOT_DEV= #FLOPPY 

.s.o:
	$(AS) --32 -o $*.o $<
	
all: Image

Image: tools/system tools/build
	objcopy -O binary -R .note -R .comment tools/system tools/kernel
	tools/build tools/kernel > Image
	rm tools/kernel -f
	sync

tools/build: tools/build.c
	$(CC) $(CFLAGS) \
	-o tools/build tools/build.c

boot/boot.o: boot/boot.s
	gcc -m32 -I./include -traditional -c boot/boot.s
	mv boot.o boot/

tools/system: boot/boot.o
	$(LD) $(LDFLAGS) boot/boot.o -o tools/system
	nm tools/system | grep -v '\(compiled\)\|\(\.o$$\)\|\( [aU] \)\|\(\.\.ng$$\)\|\(LASH[RL]DI\)'| sort > System.map

clean:
	rm -f Image System.map
	rm -f tools/system boot/*.o tools/build

