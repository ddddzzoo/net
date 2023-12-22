#include "header.hpp"

// 点分十进制转换
int main() {
  char buf[] = "192.168.0.1";
  unsigned int num = 0;
  inet_pton(AF_INET, buf, &num);
  printf("%d\n", num);
  auto *p = (unsigned char *)&num;
  printf("%d %d %d %d\n", *p, *(p + 1), *(p + 2), *(p + 3));
  char ip[16] = "";
  printf("%s\n", inet_ntop(AF_INET, &num, ip, 16));

  return 0;
}