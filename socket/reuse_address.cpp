#include <arpa/inet.h>
#include <libgen.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cassert>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <string>

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
  // 设置地址复用，允许重用TIME_WAIT状态的端口
  // 为什么需要TIME_WAIT状态
  // 1. 防止历史连接中的数据，被后面相同的四元组的连接错误的接收
  // 2. 保证被动关闭连接的一方，能被正确的关闭
  int reuse = 1;
  setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

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
    char remote[INET_ADDRSTRLEN];
    printf("connected with ip: %s and port:%d\n",
           inet_ntop(AF_INET, &client.sin_addr, remote, INET_ADDRSTRLEN),
           ntohs(client.sin_port));
    close(conn_fd);
  }

  close(sock_fd);
  return 0;
}