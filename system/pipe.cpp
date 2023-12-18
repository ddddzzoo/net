#include <unistd.h>

#include <cstdio>
#include <string>

// 子进程通过无名管道向父进程传递字符串
int main() {
  // 1. 创建管道
  int pipe_fd[2]{};

  if (pipe(pipe_fd) < 0) {
    perror("pipe");
  }
  // 2. 创建进程
  pid_t pid = fork();

  if (0 == pid) {  // 子进程
    char buf[] = "string from child";
    // 3. 向管道中写数据 pipe_fd[1] 写管道文件描述符
    write(pipe_fd[1], buf, strlen(buf));
    _exit(0);
  }
  else if (pid > 0) {  //  父进程
    wait(nullptr);     // 等待子进程结束
    char str[50]{};

    // 4. 从管道读取数据
    read(pipe_fd[0], str, sizeof(str));

    printf("str = [%s]\n", str);
  }

  // 查看管道缓冲区函数 _PC_PIPE_BUF：管道缓冲区大小 _PC_NAME_MAX：文件名字字数的上限
  long num = fpathconf(pipe_fd[0], _PC_PIPE_BUF);
  printf("num = %ld\n", num);

  return 0;
}