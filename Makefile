AS	=as
LD	=ld
LDFLAGS	=-m elf_i386 -Ttext 0 -e _start

.s.o:
	$(AS) --32 -o $*.o $<
	
all: Image

Image: system
	objcopy -O binary -R .note -R .comment system Image
	sync

boot/boot.o: boot/boot.s
	gcc -m32 -I./include -traditional -c boot/boot.s
	mv boot.o boot/
	
boot/setup.o: boot/setup.s

system: boot/boot.o boot/setup.o
	$(LD) $(LDFLAGS) boot/boot.o boot/setup.o -o system
	nm system | grep -v '\(compiled\)\|\(\.o$$\)\|\( [aU] \)\|\(\.\.ng$$\)\|\(LASH[RL]DI\)'| sort > System.map

clean:
	rm -f Image System.map
	rm -f system boot/*.o

