#include <arpa/inet.h>
#include <libgen.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cassert>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <string>

// 监听队列中处于ESTABLISHED状态的连接对应客户端网络异常
// 服务器accept调用是否成功
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

  int sock = socket(PF_INET, SOCK_STREAM, 0);
  assert(sock >= 0);

  int ret = bind(sock, (struct sockaddr*)&address, sizeof(address));
  assert(ret != -1);

  ret = listen(sock, 5);
  assert(ret != -1);

  sleep(20);

  struct sockaddr_in client {};
  socklen_t client_addr_length = sizeof(client);

  // accept只是从监听队列中取出连接，而不论连接处于何种状态，更不关心任何网络状况的变化
  int conn_fd = accept(sock, (struct sockaddr*)&client, &client_addr_length);
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

  close(sock);
  return 0;
}