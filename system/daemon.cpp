#include <fcntl.h>
#include <sys/time.h>
#include <unistd.h>

#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

void write_time(int num) {
  time_t raw_time;
  time(&raw_time);
  char* cur = ctime(&raw_time);

  int fd = open("txt", O_WRONLY | O_CREAT, 0644);
  if (fd == -1) {
    perror("open");
    exit(EXIT_FAILURE);
  }

  if (write(fd, cur, strlen(cur) + 1) == -1) {
    perror("write");
    exit(EXIT_FAILURE);
  }

  close(fd);
}

// 守护进程 每隔2s获取系统时间写入文件
int main() {
  // 创建子进程 父进程退出
  pid_t pid = fork();
  if (pid == -1) {
    perror("fork");
    exit(EXIT_FAILURE);
  }
  if (pid > 0) {
    exit(EXIT_FAILURE);
  }
  else if (pid == 0) {
    // 子进程中创建新会话（使子进程脱离控制）
    setsid();

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    // 注册信号捕捉函数
    struct sigaction sig_act {};
    sig_act.sa_flags = 0;
    sigemptyset(&sig_act.sa_mask);
    sig_act.sa_handler = write_time;
    // 设置定时器
    struct itimerval act {};
    // 定时周期
    act.it_interval.tv_sec = 2;
    act.it_interval.tv_usec = 0;
    // 设置第一次出发定时器读时间
    act.it_value.tv_sec = 2;
    act.it_value.tv_usec = 0;
    // 开始计时
    setitimer(ITIMER_REAL, &act, nullptr);
    // 防止子进程退出
    while (true)
      ;
  }
  return 0;
}