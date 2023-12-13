#include <fcntl.h>
#include <unistd.h>

#include <cstdio>

int main() {
  // 1. O_RDONLY 以只读的方式打开文件，文件不存在则报错
  // 2. O_WRONLY | O_CREAT
  // 以只写的方式打开文件，文件存在就打开文件,文件不存在则创建文件
  // 3. O_WRONLY | O_CREAT | O_EXCL
  // 以只写的方式打开文件，文件存在就报错,文件不存在则创建文件
  // 4. O_RDWR | O_CREAT
  // 以读写的方式打开文件，文件存在就打开文件,文件不存在则创建文件
  // 5. O_TRUNC 清空文件内容
  // 6. O_APPEND 以追加的方式
  int fd = open("txt", O_RDWR | O_APPEND | O_CREAT, 0644);

  if (-1 == fd) {
    perror("open");
    return 1;
  }

  printf("fd = %d\n", fd);

  close(fd);

  return 0;
}