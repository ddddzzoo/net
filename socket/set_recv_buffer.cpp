#include <arpa/inet.h>
#include <libgen.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <cstring>

#define BUFFER_SIZE 1024

int main(int argc, char* argv[]) {
  if (argc <= 3) {
    printf("usage: %s ip_address port_number send_buffer_size\n",
           basename(argv[0]));
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

  int recv_buf = std::stoi(argv[3]);
  int len = sizeof(recv_buf);
  setsockopt(sock_fd, SOL_SOCKET, SO_RCVBUF, &recv_buf, sizeof(recv_buf));
  getsockopt(sock_fd, SOL_SOCKET, SO_RCVBUF, &recv_buf, (socklen_t*)&len);
  printf("the tcp recv buffer size after setting is %d\n", recv_buf);

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
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    while (recv(conn_fd, buffer, BUFFER_SIZE - 1, 0) > 0) {
    }
    close(conn_fd);
  }

  close(sock_fd);
  return 0;
}