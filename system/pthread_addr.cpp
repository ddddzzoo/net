#include "thread_header.h"

int global = 10;  // 线程共享

void* thread_func(void* arg) {
  printf("child: %lu, global: %d\n", (unsigned long)pthread_self(), global);
  return nullptr;
}

void* thread_func2(void* arg) {
  int* heap = (int*)arg;
  *heap = 100;
  printf("child: %lu, heap: %d\n", (unsigned long)pthread_self(), *heap);
  return nullptr;
}

int main() {
  printf("main: %lu, global: %d\n", (unsigned long)pthread_self(), global);
  pthread_t tid1, tid2;
  int ret = pthread_create(&tid1, nullptr, thread_func, nullptr);
  THREAD_ERROR_CHECK(ret, "pthread_create")

  int* heap = (int*)malloc(sizeof(int));
  *heap = 0; // 堆区资源共享
  ret = pthread_create(&tid2, nullptr, thread_func2, (void*)heap);
  THREAD_ERROR_CHECK(ret, "pthread_create")
  sleep(1);
  global = 200;
  printf("main: %lu, global: %d\n", (unsigned long)pthread_self(), global);
  printf("main: %lu, heap: %d\n", (unsigned long)pthread_self(), *heap);
  sleep(1);

  free(heap);
  return 0;
}