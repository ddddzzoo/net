#include <arpa/inet.h>
#include <fcntl.h>
#include <libgen.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cassert>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <string>

#ifdef __linux__
#include <sys/sendfile.h>
#endif

int main(int argc, char* argv[]) {
  if (argc <= 3) {
    printf("usage: %s ip_address port_number filename\n", basename(argv[0]));
    return 1;
  }
  const char* ip = argv[1];
  int port = std::stoi(argv[2]);
  const char* file_name = argv[3];

  int file_fd = open(file_name, O_RDONLY);
  assert(file_fd > 0);
  struct stat stat_buf {};
  fstat(file_fd, &stat_buf);

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

  struct sockaddr_in client {};
  socklen_t client_addr_len = sizeof(client);
  int conn_fd = accept(sock, (struct sockaddr*)&client, &client_addr_len);
  if (conn_fd < 0) {
    printf("errno is: %d\n", errno);
  }
  else {
#ifdef __linux__
    sendfile(conn_fd, file_fd, nullptr, stat_buf.st_size);
#elif defined(__APPLE__)
    sendfile(file_fd, conn_fd, 0, &stat_buf.st_size, nullptr, 0);
#endif

    close(conn_fd);
  }

  close(sock);
  return 0;
}