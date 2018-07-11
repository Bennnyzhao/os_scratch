/*
 *  tools/build.c
 *
 *  �ο�Linus Torvalds linux-0.11
 */

/*
 *  �ó������ڽ�bootsectд�����ӳ���ļ�
 *
 * - bootsect: �Ϊ512�ֽڵ�8086������
 *
 * ld86�����ӳ�������bootsect��ִ�г����ļ�ʱ�������ļ�ͷ������minix ִ��ͷ����Ϣ
 *
 * ��ģ�����ȼ��bootsect����ģ���ļ������Ƿ���ȷ��������������ն�����ʾ����
 * (��ִ��make��ʱ�����ʾ���)��Ȼ��ɾ��ģ��minix�ļ��ṹͷ�������䵽��ȷ�ĳ��ȡ�
 * ͬʱ��һЩ��ʾ��Ϣд��stderr��׼���������
 * �ó�������ϵͳ�����һ���֣���ʹ�õ�ͷ�ļ��Ⱦ�������������linuxϵͳ�ı�׼
 * ���ļ���������ֻ�ǹ�����һ����ϵͳ����������ӵ�һ��ӳ���ļ��Ĺ��ߣ������ó�
 * ����makefile�е���һ������ʹ�á�
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

 #define MINIX_HEADER 32     /* minix ������ģ��ͷ������Ϊ32 �ֽ� */

 /* ��ʾ������Ϣ������ֹ���� */
 void die(char *str)
 {
      fprintf(stderr, "%s\n", str);
      exit(1);
 }

 /* argv����2������ argv[0]Ϊ�ó���ִ��·������һ��λbootsectִ�г��� */
 /* ����û�жԴ���2�����������ݴ��� */
 int main(int argc, char **argv)
 {
      int i, id;
      char buf[1024];
      fprintf(stderr, "delete the minix header first, now begin\n");

      /* ��ʼ��buf ��������ȫ��0�� */
      for(i=0; i<1024; i++) 
      {
        buf[i] = 0;
      }

      /* ��ֻ����ʽ�򿪲���1 ָ�����ļ�(bootsect)������������ʾ������Ϣ���˳���*/
      if((id = open(argv[1], O_RDONLY, 0)) <0)
      {
        die("Unable to open 'boot'");
      }

      /* ��ȡʵ�ʴ������ݣ�Ӧ�÷��ض�ȡ�ֽ���Ϊ512 �ֽڡ� */
      i = read(id, buf, sizeof(buf));
      fprintf(stderr, "Boot sector %d bytes.\n", i);
      if(i != 512)
      {
        die("Boot block must be exactly 512 bytes");
      }

      /* �ж�boot ��0x510 ���Ƿ��п�������־0xAA55�� */
      if((*(unsigned short*)(buf+510)) != 0xAA55)
      {
        die("Boot block hasn't got boot flag (0xAA55)");
      }

      /* ����boot ��512 �ֽڵ�����д����׼���stdout����д���ֽ������ԣ�����ʾ������Ϣ���˳��� */
      /* makefile�ļ���ִ�иó����лὫboot ��512 �ֽ��ض���Imageӳ���ļ��� */
      i = write(1, buf, 512);
      if(i != 512)
      {
        die("Write call failed");
      }

      /* ���ر�bootsect ģ���ļ��� */
      close(id);

      return (0);
 }

 
