#include <arpa/inet.h>
#include <fcntl.h>
#include <libgen.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cassert>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

int main(int argc, char* argv[]) {
  if (argc <= 2) {
    printf("usage: %s ip_address port_number\n", basename(argv[0]));
    return 1;
  }
  const char* ip = argv[1];
  int port = std::stoi(argv[2]);
  printf("ip is %s and port is %d\n", ip, port);

  struct sockaddr_in address {};
  bzero(&address, sizeof(address));
  address.sin_family = AF_INET;
  inet_pton(AF_INET, ip, &address.sin_addr);
  address.sin_port = htons(port);

  int sock = socket(AF_INET, SOCK_STREAM, 0);
  assert(sock >= 0);

  int ret = bind(sock, (struct sockaddr*)&address, sizeof(address));
  assert(ret != -1);

  ret = listen(sock, 5);
  assert(ret != -1);

  struct sockaddr_in client_address {};
  socklen_t client_addr_len = sizeof(client_address);
  int conn_fd =
      accept(sock, (struct sockaddr*)&client_address, &client_addr_len);
  if (conn_fd < 0) {
    printf("errno is %d\n", errno);
  }

  char remote_addr[INET_ADDRSTRLEN];
  printf("connected with ip: %s and port: %d\n",
         inet_ntop(AF_INET, &client_address.sin_addr, remote_addr,
                   INET_ADDRSTRLEN),
         ntohs(client_address.sin_port));

  char buf[1024];
  fd_set read_fds;
  fd_set exception_fds;

  FD_ZERO(&read_fds);
  FD_ZERO(&exception_fds);

  int reuse_addrs = 1;
  setsockopt(conn_fd, SOL_SOCKET, SO_OOBINLINE, &reuse_addrs,
             sizeof(reuse_addrs));

  while (true) {
    memset(buf, 0, sizeof(buf));
    FD_SET(conn_fd, &read_fds);
    FD_SET(conn_fd, &exception_fds);

    ret = select(conn_fd + 1, &read_fds, nullptr, &exception_fds, nullptr);
    printf("select one\n");
    if (ret < 0) {
      printf("selection failure\n");
      break;
    }
    if (FD_ISSET(conn_fd, &read_fds)) {
      ret = recv(conn_fd, buf, sizeof(buf) - 1, 0);
      if (ret <= 0) {
        break;
      }
      printf("get %d bytes of normal data: %s\n", ret, buf);
    }
    else if (FD_ISSET(conn_fd, &exception_fds)) {
      ret = recv(conn_fd, buf, sizeof(buf) - 1, MSG_OOB);
      if (ret <= 0) {
        break;
      }
      printf("get %d bytes of oob data: %s\n", ret, buf);
    }
  }

  close(conn_fd);
  close(sock);
  return 0;
}