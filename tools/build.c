/*
 *  tools/build.c
 *
 *  ²Î¿¼Linus Torvalds linux-0.11
 */

 #include <stdio.h>
 #include <string.h>
 #include <stdlib.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <linux/fs.h>
 #include <linux/kdev_t.h>
 #include <unistd.h>
 #include <fcntl.h>
 
 #define SYS_SIZE 0x3000
 
 #define DEFAULT_MAJOR_ROOT 2
 #define DEFAULT_MINOR_ROOT 0x1d
 
 #define SETUP_SECTS  4
 #define STRINGIFY(x) #x

 void die(char *str)
 {
      fprintf(stderr, "%s\n", str);
      exit(1);
 }

 void usage(void)
 {
	 die("Usage: build boot system [rootdev] [> image]");
 }

 int main(int argc, char **argv)
 {
      int i, c, id;
      char buf[1024];
      char major_root, minor_root;
      struct stat sb;

      if ((argc != 3) && (argc != 4))
      {
          usage();
      }
      
      if (argc == 4)
      {
          if (strcmp(argv[3], "FLOPPY"))
          {
              if (stat(argv[3], &sb)) 
              {
                 perror(argv[3]);
                 die("Couldn't stat root device.");
              }
              major_root = MAJOR(sb.st_rdev);
              minor_root = MINOR(sb.st_rdev);
          }
          else
          {
              major_root = 0;
              minor_root = 0;
          }
      }
      else
      {
          major_root = DEFAULT_MAJOR_ROOT;
          minor_root = DEFAULT_MINOR_ROOT;
      }
      
      fprintf(stderr, "Root device is (%d, %d)\n", major_root, minor_root);
      if ((major_root != 2) && (major_root != 3) && (major_root != 0))
      {
          fprintf(stderr, "Illegal root device (major = %d)\n", major_root);
          die("Bad root device --- major #");
      }

      for(i=0; i<1024; i++) 
      {
        buf[i] = 0;
      }

      if((id = open(argv[1], O_RDONLY, 0)) <0)
      {
        die("Unable to open 'boot'");
      }

      i = read(id, buf, 512);
      fprintf(stderr, "Boot sector %d bytes.\n", i);
      if(i != 512)
      {
        die("Boot block must be exactly 512 bytes");
      }

      if((*(unsigned short*)(buf+510)) != 0xAA55)
      {
        die("Boot block hasn't got boot flag (0xAA55)");
      }
      
      buf[508] = (char) minor_root;
	    buf[509] = (char) major_root;
      
      i = write(1, buf, 512);
      if(i != 512)
      {
        die("Write call failed");
      }
      
      c = read(id, buf, sizeof(buf));
      for(i=c; c > 0; i += c)
      {
          if(write(1, buf, c) != c)
          {
             die("Write call failed");
          }

          c = read(id, buf, sizeof(buf));
      }
      close(id);
      
      if(i > SETUP_SECTS * 512)
      {
          die("Setup exceeds "STRINGIFY(SETUP_SECTS)" sectors - rewrite build/boot/setup");
      }
      
      fprintf(stderr, "Setup is %d bytes.\n", i);
      
      for (c = 0; c < sizeof (buf); c++)    buf[c] = '\0';
      
      while (i < SETUP_SECTS * 512)
      {
          c = SETUP_SECTS * 512 - i;
          if (c > sizeof (buf))
          {
             c = sizeof (buf);
          }
          if (write (1, buf, c) != c)
          {
             die ("Write call failed");
          }

          i += c;
      }
      
      if ((id=open(argv[2],O_RDONLY,0))<0)
      {
          die("Unable to open 'system'");
      }

    //	if (read(id,buf,GCC_HEADER) != GCC_HEADER)
    //  die("Unable to read header of 'system'");
    //	if (((long *) buf)[5] != 0)
    //  die("Non-GCC header of 'system'");
      for (i=0 ; (c=read(id,buf,sizeof(buf)))>0 ; i+=c )
      {
          if (write(1,buf,c)!=c)
          {
             die("Write call failed");
          }
      }
      close(id);

      fprintf(stderr,"System is %d bytes.\n",i);

      if (i > SYS_SIZE*16)
      {
          die("System is too big");
      }

      return (0);
 }

 
