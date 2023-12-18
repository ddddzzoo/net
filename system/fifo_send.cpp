#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

int main() {
  // 创建有名管道
  if (-1 == mkfifo("my_fifo", 0666)) {
    perror("my_fifo");
    exit(EXIT_FAILURE);
  }

  // 打开有名管道
  int fd = open("my_fifo", O_WRONLY);
  if (-1 == fd) {
    perror("open");
    exit(EXIT_FAILURE);
  }

  // 向管道中写入数据
  char buf[] = "string from my_fifo";
  write(fd, buf, strlen(buf));

  // 关闭文件描述符
  close(fd);
  // 删除有名管道
  unlink("my_fifo");

  return 0;
}