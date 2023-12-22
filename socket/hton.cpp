#include "header.hpp"

// 大小端转换
int main() {
  unsigned short s = 0x1234;
  printf("s = %x\n", s);
  printf("htons(s) = %x\n", htons(s));
  unsigned int i = 0x12345678;
  printf("i = %x\n", i);
  printf("htonl(i) = %x\n", htonl(i));

  return 0;
}