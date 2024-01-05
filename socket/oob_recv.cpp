#include <arpa/inet.h>
#include <libgen.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cassert>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <string>

#define BUF_SIZE 1024

int main(int argc, char* argv[]) {
  if (argc <= 2) {
    printf("usage: %s ip_address port_number\n", basename(argv[0]));
    return 1;
  }
  const char* ip = argv[1];
  int port = std::stoi(argv[2]);

  struct sockaddr_in address {};
  bzero(&address, sizeof(address));
  address.sin_family = AF_INET;
  inet_pton(AF_INET, ip, &address.sin_addr);
  address.sin_port = htons(port);

  int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  assert(sock_fd >= 0);

  int ret = bind(sock_fd, (struct sockaddr*)&address, sizeof(address));
  assert(ret != -1);

  ret = listen(sock_fd, 5);
  assert(ret != -1);

  struct sockaddr_in client {};
  socklen_t client_addr_len = sizeof(client);
  int conn_fd = accept(sock_fd, (struct sockaddr*)&client, &client_addr_len);
  if (conn_fd < 0) {
    printf("errno is: %d\n", errno);
  }
  else {
    char buffer[BUF_SIZE];

    memset(buffer, '\0', BUF_SIZE);
    ret = static_cast<int>(recv(conn_fd, buffer, BUF_SIZE - 1, 0));
    printf("got %d byte of normal data '%s'\n", ret, buffer);  // 123ab

    memset(buffer, '\0', BUF_SIZE);
    ret = static_cast<int>(recv(conn_fd, buffer, BUF_SIZE - 1, MSG_OOB));
    printf("got %d byte of obb data '%s'\n", ret, buffer);  // c

    memset(buffer, '\0', BUF_SIZE);
    ret = static_cast<int>(recv(conn_fd, buffer, BUF_SIZE - 1, 0));
    printf("got %d byte of normal data '%s'\n", ret, buffer);  // 123

    // 带外数据只有1个字节，最少出现了2个recv对数据进行接收
    close(conn_fd);
  }

  close(sock_fd);
  return 0;
}