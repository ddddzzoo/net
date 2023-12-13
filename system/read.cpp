#include <fcntl.h>
#include <unistd.h>

#include <cstdio>
#include <cstring>

#define SIZE 128

// 读文件
int main() {
  // 1. 以只读的方式打开文件
  int fd = open("txt", O_RDONLY);

  if (-1 == fd) {
    perror("open");
    return 1;
  }

  printf("fd = %d\n", fd);

  // 2. 读文件
  char buf[SIZE];
  memset(buf, 0, SIZE);
  // 从文件中最多读取SIZE个字节保存到buf中
  auto ret = read(fd, buf, SIZE);

  if (-1 == ret) {
    perror("read");
    close(fd);
    return 1;
  }

  printf("read len: %zd\n", ret);
  // 3. 关闭文件
  close(fd);

  return 0;
}