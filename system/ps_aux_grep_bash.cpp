#include <unistd.h>

#include <cstdio>
#include <cstdlib>

int main() {
  // 创建管道
  int pipe_fd[2]{};
  if (pipe(pipe_fd) == -1) {
    perror("pipe");
    exit(EXIT_FAILURE);
  }
  // 创建子进程1
  pid_t child_pid1 = fork();
  if (child_pid1 == -1) {
    perror("fork");
    exit(EXIT_FAILURE);
  }

  // 子进程1执行ps aux命令
  if (child_pid1 == 0) {
    close(pipe_fd[0]);
    dup2(pipe_fd[1], STDOUT_FILENO);  // 将标准输出重定向到管道写端
    close(pipe_fd[1]);

    execlp("ps", "ps", "aux", nullptr);
    perror("execlp");
    exit(EXIT_FAILURE);
  }
  else {
    // 创建子进程2
    pid_t child_pid2 = fork();
    if (child_pid2 == -1) {
      perror("fork");
      exit(EXIT_FAILURE);
    }

    if (child_pid2 == 0) {
      // 子进程2执行grep bash命令
      close(pipe_fd[1]);
      dup2(pipe_fd[0], STDIN_FILENO);  // 将标准输入重定向到管道读端
      close(pipe_fd[0]);

      execlp("grep", "grep", "bash", nullptr);
      perror("execlp");
      exit(EXIT_FAILURE);
    }
    else {
      // 回收进程资源
      close(pipe_fd[0]);
      close(pipe_fd[1]);
      waitpid(child_pid1, nullptr, 0);
      waitpid(child_pid2, nullptr, 0);
    }
  }

  return 0;
}