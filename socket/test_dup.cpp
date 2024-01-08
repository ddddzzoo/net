#include <arpa/inet.h>
#include <libgen.h>
#include <netinet/in.h>
#include <sys/socket.h>
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

  struct sockaddr_in address {};
  bzero(&address, sizeof(address));
  address.sin_family = AF_INET;
  inet_pton(AF_INET, ip, &address.sin_addr);
  address.sin_port = htons(port);

  int sock = socket(AF_INET,SOCK_STREAM,0);
  assert(sock >= 0);

  int ret = bind(sock, (struct sockaddr*)& address, sizeof(address));
  assert(ret != -1);

  ret = listen(sock,5);
  assert(ret != -1);

  struct sockaddr_in client{};
  socklen_t client_addr_len = sizeof(client);
  int conn_fd  = accept(sock, (struct sockaddr*)&client,&client_addr_len);
  if (conn_fd <0){
    printf("errno is: %d\n", errno);
  }
  else{
    close(STDOUT_FILENO);
    dup(conn_fd);
    printf("a b c d\n");
    close(conn_fd);
  }

  close(sock);
  return 0;
}