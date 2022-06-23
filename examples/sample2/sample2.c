/* 
  Source Code for 'sample2' trace.
  
  This is a simple example for how FlowMatrix
  deals with real execution traces. Sample2
  simply read this file by a read syscall
  and dump to sample2.out with a write
  syscall.

  However, in real execution, there are more
  than one read and one write syscall

  Compile with:
    $ gcc ./sample2.c -o sample2
*/
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
int main()
{
    char buffer[1024];
    int fd = open("sample2.c", O_RDONLY);
    int length = read(fd, buffer, 1024);
    close(fd);
    printf("Read %d bytes.\n", length);
    fd = open("sample2.out", O_WRONLY | O_CREAT,  S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    length = write(fd,buffer,length);
    printf("Write %d bytes.\n", length);
    close(fd);
}
