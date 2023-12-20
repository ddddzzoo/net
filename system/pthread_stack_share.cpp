#include "thread_header.h"

void* thread_func(void* arg) {
  long* p = (long*)arg;
  printf("child: %lu, long: %ld\n", (unsigned long)pthread_self(), *p);
  return nullptr;
}

// 多线程栈区相对独立，但是一个线程可以通过地址去访问另一个线程的栈区
int main() {
  pthread_t tid;
  long i = 3;
  int ret = pthread_create(&tid, nullptr, thread_func, (void*)&i);
  THREAD_ERROR_CHECK(ret, "pthread_create")
  i = 555;
  ret = pthread_create(&tid, nullptr, thread_func, (void*)&i);
  THREAD_ERROR_CHECK(ret, "pthread_create")

  sleep(1);
  return 0;
}