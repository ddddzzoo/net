#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#define SIZE 128

// 使用有名管道实现聊天室
int main() {
  // 创建有名管道 fifo1 fifo2
  if (access("fifo1", F_OK)) {
    printf("create pipe\n");
    if (mkfifo("fifo1", 0644) == -1) {
      perror("mkfifo");
      exit(EXIT_FAILURE);
    }
  }

  if (access("fifo2", F_OK)) {
    printf("create pipe\n");
    if (mkfifo("fifo2", 0644) == -1) {
      perror("mkfifo");
      exit(EXIT_FAILURE);
    }
  }
  // 只写打开fifo1
  int fdw = open("fifo1", O_WRONLY);
  if (fdw == -1) {
    perror("open");
    exit(EXIT_FAILURE);
  }
  printf("open fifo1 for reading\n");
  // 只读打开fifo2
  int fdr = open("fifo2", O_RDONLY);
  if (fdr == -1) {
    perror("open");
    exit(EXIT_FAILURE);
  }
  printf("open fifo2 for writing\n");
  // 创建子进程
  pid_t pid = fork();
  if (pid == -1) {
    perror("fork");
    close(fdr);
    close(fdw);
    exit(EXIT_FAILURE);
  }

  // 子进程读 fifo2
  char buf[SIZE];
  if (pid == 0) {
    while (true) {
      memset(buf, 0, SIZE);
      if (read(fdr, buf, SIZE) <= 0) {
        perror("read");
        break;
      }
      printf("%s \n", buf);
    }
    exit(EXIT_FAILURE);
  }

  // 父进程写 fifo1
  while (true) {
    memset(buf, 0, SIZE);
    fgets(buf, SIZE, stdin);
    buf[strlen(buf) - 1] = 0;

    if (write(fdw, buf, strlen(buf)) == -1) {
      perror("write");
      break;
    }
  }

  // 关闭文件
  close(fdr);
  close(fdw);

  return 0;
}