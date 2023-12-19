#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#define MEM_SIZE 64

// 使用存储映射实现不同进程之间通信
int main() {
  // 打开一个文件
  int fd = open("my_mmap", O_RDWR | O_CREAT, 0644);
  if (-1 == fd) {
    perror("open");
    exit(EXIT_FAILURE);
  }

  // 设置文件大小
  if (ftruncate(fd, MEM_SIZE) < 0) {
    perror("ftruncate");
    exit(EXIT_FAILURE);
  }

  // 创建内存映射区
  void *ptr = mmap(nullptr, 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (ptr == MAP_FAILED) {
    perror("mmap error");
    exit(EXIT_FAILURE);
  }

  // 关闭文件
  close(fd);

  // 写入映射存储区
  memcpy(ptr, "1234567890", 10);

  // 释放内存映射区
  if (-1 == munmap(ptr, MEM_SIZE)) {
    perror("munmap error");
    exit(EXIT_FAILURE);
  }

  // 删除文件
  sleep(10);
  if (-1 == unlink("my_mmap")) {
    perror("unlink");
    exit(EXIT_FAILURE);
  }

  return 0;
}