#include "thread_header.h"

void* thread_func(void* arg) {
  printf("child: %lu\n", (unsigned long)pthread_self());
  pthread_exit((void*)23456);
}

int main() {
  printf("main: %lu\n", (unsigned long)pthread_self());
  pthread_t tid;

  int ret = pthread_create(&tid, nullptr, thread_func, nullptr);
  THREAD_ERROR_CHECK(ret, "pthread_create")

  void* val;  // 存储子线程返回值
  ret = pthread_join(tid, &val);
  THREAD_ERROR_CHECK(ret, "pthread_join")

  printf("%ld", (long)val);
  return 0;
}