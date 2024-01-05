#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cassert>
#include <cstdio>

int main(int argc, char* argv[]) {
  assert(argc == 2);

  // time.nist.gov
  char* host = argv[1];
  // 获取主机信息
  struct hostent* host_info = gethostbyname(host);
  assert(host_info);

  // 获取服务端口信息
  struct servent* server_info = getservbyname("daytime", "tcp");
  assert(server_info);
  printf("daytime port is %d\n", ntohs(server_info->s_port));

  struct sockaddr_in address {};
  address.sin_family = AF_INET;
  address.sin_port = server_info->s_port;
  address.sin_addr = *(struct in_addr*)*host_info->h_addr_list;

  int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  int ret = connect(sock_fd, (struct sockaddr*)&address, sizeof(address));
  assert(ret != -1);

  // 读取服务器发送时间
  char buffer[128]{};
  ret = static_cast<int>(read(sock_fd, buffer, sizeof(buffer)));
  assert(ret > 0);
  buffer[ret] = '\0';
  printf("the daytime is: %s\n", buffer);

  close(sock_fd);
  return 0;
}