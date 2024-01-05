#include <arpa/inet.h>
#include <libgen.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <string>

#define BUFFER_SIZE 1024

int main(int argc, char* argv[]) {
  if (argc <= 3) {
    printf("usage: %s ip_address port_number send_buffer_size\n",
           basename(argv[0]));
    return 1;
  }
  const char* ip = argv[1];
  int port = std::stoi(argv[2]);

  struct sockaddr_in server_address {};
  bzero(&server_address, sizeof(server_address));
  server_address.sin_family = AF_INET;
  inet_pton(AF_INET, ip, &server_address.sin_addr);
  server_address.sin_port = htons(port);

  int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  assert(sock_fd >= 0);

  int send_buf = std::stoi(argv[3]);
  int len = sizeof(send_buf);
  setsockopt(sock_fd, SOL_SOCKET, SO_SNDBUF, &send_buf, sizeof(send_buf));
  getsockopt(sock_fd, SOL_SOCKET, SO_SNDBUF, &send_buf, (socklen_t*)&len);
  printf("the tcp send buffer size after setting is %d\n", send_buf);

  if (connect(sock_fd, (struct sockaddr*)&server_address,
              sizeof(server_address)) != -1) {
    char buffer[BUFFER_SIZE];
    memset(buffer, 'a', BUFFER_SIZE);
    send(sock_fd, buffer, BUFFER_SIZE, 0);
  }

  close(sock_fd);
  return 0;
}