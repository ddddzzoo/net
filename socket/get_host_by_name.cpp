#include "header.hpp"

// 获取远程主机信息 baidu
int main() {
  struct hostent *host = gethostbyname("www.baidu.com");
  if (host == nullptr) {
    fprintf(stderr, "get host by name:%s\n", strerror(h_errno));
    return -1;
  }

  printf("Host real name = %s\n", host->h_name);

  for (int i = 0; host->h_aliases[i] != nullptr; ++i) {
    printf("alias name = %s\n", host->h_aliases[i]);
  }

  printf("addr type = %d\n", host->h_addrtype);
  printf("addr length = %d\n", host->h_length);

  for (int i = 0; host->h_addr_list[i] != nullptr; ++i) {
    char buf[1024]{};
    inet_ntop(host->h_addrtype, host->h_addr_list[i], buf, sizeof(buf));
    printf("addr = %s\n", buf);
  }

  return 0;
}