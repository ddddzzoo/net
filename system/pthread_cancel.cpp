#include "thread_header.h"

void* thread_func(void* arg) {
  while (true) {
    pthread_testcancel();
  }
  pthread_exit((void*)nullptr);
}

int main() {
  printf("main: %lu\n", (unsigned long)pthread_self());
  pthread_t tid;
  int ret = pthread_create(&tid, nullptr, thread_func, nullptr);
  THREAD_ERROR_CHECK(ret, "pthread_create")

  pthread_cancel(tid);
  void* ret_val;
  pthread_join(tid, &ret_val);

  printf("child thread return value: %ld\n", (long)ret_val);
}