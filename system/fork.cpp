#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstdio>
#include <cstring>

#define SIZE 128

// 测试fork之前打开文件，父子进程间是否共享文件
// 子进程write 父进程read
int main() {
  // 1. 打开文件
  int fd = open("txt", O_RDWR | O_CREAT | O_APPEND, 0644);
  if (-1 == fd) {
    perror("open");
    return -1;
  }

  // 2. 创建子进程
  pid_t pid = fork();
  if (pid < 0) {
    perror("fork");
  }

  if (0 == pid) {
    // 子进程
    const char* str = "Hello world!";
    auto ret = write(fd, str, strlen(str));
    if (-1 == ret) {
      perror("write");
      close(fd);
      return -1;
    }
  }
  else if (pid > 0) {
    // 父进程
    waitpid(pid, nullptr, 0);  // 等待子进程结束
    // 重新定位文件指针到文件头
    lseek(fd, 0, SEEK_SET);
    char buf[SIZE];
    memset(buf, 0, SIZE);
    auto ret = read(fd, buf, SIZE);
    if (-1 == ret) {
      perror("read");
      close(fd);
      return 1;
    }

    printf("read len: %zd\n", ret);
    printf("%s\n", buf);
  }

  return 0;
}