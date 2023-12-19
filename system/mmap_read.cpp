#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>

#define MEM_SIZE 64
// 使用存储映射实现不同进程之间通信
int main() {
  // 打开文件
  int fd = open("my_mmap", O_RDONLY);
  if (-1 == fd) {
    perror("open");
    exit(EXIT_FAILURE);
  }

  // 创建内存映射区
  void* shared_memory = mmap(nullptr, MEM_SIZE, PROT_READ, MAP_SHARED, fd, 0);
  if (shared_memory == MAP_FAILED) {
    perror("mmap");
    exit(EXIT_FAILURE);
  }

  // 关闭文件
  close(fd);

  // 读数据
  printf("read:[%s]\n", (char*)shared_memory);

  // 释放内存映射区
  if (-1 == munmap(shared_memory, MEM_SIZE)) {
    perror("munmap");
    exit(EXIT_FAILURE);
  }

  return 0;
}