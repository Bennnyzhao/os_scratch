/*
 *  tools/build.c
 *
 *  参考Linus Torvalds linux-0.11
 */

/*
 *  该程序用于将bootsect写入磁盘映像文件
 *
 * - bootsect: 最长为512字节的8086机器码
 *
 * ld86在链接程序生成bootsect可执行程序文件时，会在文件头部加上minix 执行头部信息
 *
 * 该模块首先检查bootsect程序模块文件类型是否正确，并将检查结果在终端上显示出来
 * (在执行make的时候会显示结果)，然后删除模块minix文件结构头部并扩充到正确的长度。
 * 同时将一些提示信息写入stderr标准错误输出。
 * 该程序不属于系统代码的一部分，其使用的头文件等均属于宿主机器linux系统的标准
 * 库文件，在这里只是构建了一个将系统代码编译连接到一个映像文件的工具，并将该程
 * 序在makefile中当作一个命令使用。
 *
 */

 #include <stdio.h>
 #include <string.h>
 #include <stdlib.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <linux/fs.h>
 #include <unistd.h>
 #include <fcntl.h>

 #define MINIX_HEADER 32     /* minix 二进制模块头部长度为32 字节 */

 /* 显示出错信息，并终止程序。 */
 void die(char *str)
 {
      fprintf(stderr, "%s\n", str);
      exit(1);
 }

 /* argv包含2个参数 argv[0]为该程序执行路径，另一个位bootsect执行程序 */
 /* 程序没有对大于2个参数进行容错处理 */
 int main(int argc, char **argv)
 {
      int i, id;
      char buf[1024];
      fprintf(stderr, "delete the minix header first, now begin\n");

      /* 初始化buf 缓冲区，全置0。 */
      for(i=0; i<1024; i++) 
      {
        buf[i] = 0;
      }

      /* 以只读方式打开参数1 指定的文件(bootsect)，若出错则显示出错信息，退出。*/
      if((id = open(argv[1], O_RDONLY, 0)) <0)
      {
        die("Unable to open 'boot'");
      }

      /* 读取实际代码数据，应该返回读取字节数为512 字节。 */
      i = read(id, buf, sizeof(buf));
      fprintf(stderr, "Boot sector %d bytes.\n", i);
      if(i != 512)
      {
        die("Boot block must be exactly 512 bytes");
      }

      /* 判断boot 块0x510 处是否有可引导标志0xAA55。 */
      if((*(unsigned short*)(buf+510)) != 0xAA55)
      {
        die("Boot block hasn't got boot flag (0xAA55)");
      }

      /* 将该boot 块512 字节的数据写到标准输出stdout，若写出字节数不对，则显示出错信息，退出。 */
      /* makefile文件中执行该程序中会将boot 块512 字节重定向到Image映像文件中 */
      i = write(1, buf, 512);
      if(i != 512)
      {
        die("Write call failed");
      }

      /* 最后关闭bootsect 模块文件。 */
      close(id);

      return (0);
 }

 
