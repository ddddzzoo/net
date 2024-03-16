#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 程序传入参数检测宏函数
#define ARGS_CHECK(argc, num)           \
  {                                     \
    if (argc != num) {                  \
      fprintf(stderr, "args error!\n"); \
      return -1;                        \
    }                                   \
  }

// 错误检查
#define ERROR_CHECK(ret, num, msg) \
  {                                \
    if (ret == num) {              \
      perror(msg);                 \
      return -1;                   \
    }                              \
  }
