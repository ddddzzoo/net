#include "thread_header.h"

void* thread_func(void* arg) {
  while (1) {
    sleep(1);
  }
}

// 单个进程中最多能创建的线程数量
int main() {
  pthread_t tid;

  for (int i = 0;; ++i) {
    int ret = pthread_create(&tid, nullptr, thread_func, nullptr);
    THREAD_ERROR_CHECK(ret, "pthread_create");
    if (ret != 0) {
      printf("i=%d\n", i);
      break;
    }
  }

  usleep(20);

  return 0;
}