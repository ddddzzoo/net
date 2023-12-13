#include <fcntl.h>
#include <unistd.h>

#include <cstdio>
#include <cstring>

// 写文件
int main() {
  // 1. 以只写的方式打开文件
  int fd = open("txt", O_WRONLY | O_CREAT, 0644);

  if (-1 == fd) {
    perror("open");
    return 1;
  }

  printf("fd = %d\n", fd);

  // 2. 写文件
  const char* str = "Hello world!";
  auto ret = write(fd, str, strlen(str));

  if (-1 == ret) {
    perror("write");
    close(fd);
    return 1;
  }

  printf("write len: %zd\n", ret);
  // 3. 关闭文件
  close(fd);

  return 0;
}