#ifndef NET_LEARN_THREAD_HEADER_H
#define NET_LEARN_THREAD_HEADER_H

#include <pthread.h>
#include <unistd.h>

#include <cstdio>
#include <cstring>

#define THREAD_ERROR_CHECK(ret, msg)                  \
  {                                                   \
    if (ret != 0) {                                   \
      fprintf(stderr, "%s:%s\n", msg, strerror(ret)); \
    }                                                 \
  }

#endif  // NET_LEARN_THREAD_HEADER_H