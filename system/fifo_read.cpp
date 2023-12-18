#include <fcntl.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>

int main() {
  // 打开有名管道
  int fd = open("my_fifo", O_RDONLY);
  if (-1 == fd) {
    perror("open");
    exit(EXIT_FAILURE);
  }
  // 读取数据并打印
  char buf[50]{};
  read(fd, buf, sizeof(buf));
  printf("read: %s", buf);
  // 关闭文件描述符
  close(fd);

  return 0;
}