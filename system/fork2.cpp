#include <sys/wait.h>
#include <unistd.h>

#include <cstdio>

// 父进程fork三个子进程
// 调用ps命令
// 调用自定义应用程序
// 调用段错误程序
// 父进程回收三个子进程，打印子进程退出状态
int main() {
  // 父进程fork三个子进程
  for (int i = 0; i < 3; ++i) {
    pid_t child_pid = fork();

    if (-1 == child_pid) {
      perror("fork");
      return -1;
    }

    if (0 == child_pid) {
      // 在子进程中执行不同的操作
      switch (i) {
        default:
          break;
        case 0:
          // 调用ps命令
          execlp("ps", "ps", nullptr);
          perror("ps");
          break;
        case 1:
          // 调用ls命令
          execlp("ls", "ls", nullptr);
          perror("ls");
          break;
        case 2:
          // 调用会导致段错误的程序
          char* nullPointer = nullptr;
          *nullPointer = 42;  // 这里故意引发段错误
          break;
      }
    }
  }

  // 父进程回收三个子进程，打印子进程退出状态
  for (int i = 0; i < 3; ++i) {
    int status;
    pid_t child_pid = wait(&status);

    if (-1 == child_pid) {
      perror("wait");
    }

    if (WIFEXITED(status)) {
      printf("Child process %d exited with status %d\n", child_pid,
             WEXITSTATUS(status));
    }
    else if (WIFSIGNALED(status)) {
      printf("Child process %d terminated by signal %d\n", child_pid,
             WTERMSIG(status));
    }
  }

  return 0;
}