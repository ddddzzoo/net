#include <sys/wait.h>
#include <unistd.h>

#include <csignal>
#include <cstdio>
#include <cstdlib>

void sig_child(int sig) {
  pid_t pid = waitpid(-1, nullptr, WNOHANG);
  if (pid > 0) {
    printf("Child %d terminated.\n", pid);
  }
}

int main() {
  signal(SIGCHLD, sig_child);

  pid_t pid = fork();
  if (pid < 0) {
    perror("fork");
    exit(EXIT_FAILURE);
  }
  else if (pid == 0) {
    printf("child %d\n", getppid());
    exit(EXIT_SUCCESS);
  }
  else {
    sleep(3);
    printf("father \n");
    system("ps -ef | grep defunct");
  }

  return 0;
}