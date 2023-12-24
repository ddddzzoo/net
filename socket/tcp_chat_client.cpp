#include "header.hpp"

// TCP 简易客户端
int main() {
  // 创建套接字
  int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (sock_fd == -1) {
    perror("socket");
    return -1;
  }

  // 设置服务器地址信息
  struct sockaddr_in addr {};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(1234);
  addr.sin_addr.s_addr = inet_addr("127.0.0.1");

  // 连接到服务器
  int ret = connect(sock_fd, (struct sockaddr*)&addr, sizeof(addr));
  if (ret == -1) {
    perror("connect");
    return -1;
  }

  fd_set rd_set;
  char buf[4096]{};

  while (true) {
    // 使用select监控输入标准和网络套接字
    FD_ZERO(&rd_set);
    FD_SET(STDIN_FILENO, &rd_set);
    FD_SET(sock_fd, &rd_set);

    select(sock_fd + 1, &rd_set, nullptr, nullptr, nullptr);

    // 标准输入就绪
    if (FD_ISSET(STDIN_FILENO, &rd_set)) {
      bzero(buf, sizeof(buf));
      ret = static_cast<int>(read(STDIN_FILENO, buf, sizeof(buf)));
      if (ret == 0) {
        // 读取到EOF
        send(sock_fd, "chat is ending", 15, 0);
        break;
      }
      send(sock_fd, buf, strlen(buf), 0);
    }

    // 网络套接字就绪
    if (FD_ISSET(sock_fd, &rd_set)) {
      bzero(buf, sizeof(buf));
      ret = static_cast<int>(recv(STDIN_FILENO, buf, sizeof(buf), 0));
      if (ret == 0) {
        // 读取到EOF
        puts("chat is end!");
        break;
      }
      puts(buf);
    }
  }

  // 关闭
  close(sock_fd);

  return 0;
}