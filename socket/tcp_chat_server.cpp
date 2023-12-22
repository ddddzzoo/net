#include "header.hpp"

// TCP 简易聊天服务器
int main() {
  // 创建套接字
  int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (sock_fd == -1) {
    perror("socket");
    return -1;
  }

  // 设置套接字选项，允许地址复用
  int opt_val = 1;
  int ret =
      setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(int));
  if (ret == -1) {
    perror("setsockopt");
    return -1;
  }
  // 绑定地址信息到套接字
  struct sockaddr_in addr {};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(8000);
  addr.sin_addr.s_addr = INADDR_ANY;
  ret = bind(sock_fd, (struct sockaddr*)&addr, sizeof(addr));
  if (ret == -1) {
    perror("bind");
    return -1;
  }
  // 监听
  ret = listen(sock_fd, 10);
  if (ret == -1) {
    perror("listen");
    return -1;
  }
  // 接受连接，获取新的套接字描述符
  int net_fd = accept(sock_fd, nullptr, nullptr);
  if (net_fd == -1) {
    perror("accept");
    return -1;
  }

  fd_set rd_set;
  char buf[4096]{};

  while (true) {
    // 使用select监控输入标准和网络套接字
    FD_ZERO(&rd_set);
    FD_SET(STDIN_FILENO, &rd_set);
    FD_SET(net_fd, &rd_set);

    select(net_fd + 1, &rd_set, nullptr, nullptr, nullptr);

    // 标准输入就绪
    if (FD_ISSET(STDIN_FILENO, &rd_set)) {
      bzero(buf, sizeof(buf));
      ret = static_cast<int>(read(STDIN_FILENO, buf, sizeof(buf)));
      if (ret == 0) {
        // 读取到EOF
        send(net_fd, "chat is ending", 15, 0);
        break;
      }
    }

    // 网络套接字就绪
    if (FD_ISSET(net_fd, &rd_set)) {
      bzero(buf, sizeof(buf));
      ret = static_cast<int>(read(STDIN_FILENO, buf, sizeof(buf)));
      if (ret == 0) {
        // 读取到EOF
        send(net_fd, "chat is ending", 15, 0);
        break;
      }
      puts(buf);
    }
  }

  // 关闭
  close(sock_fd);
  close(net_fd);

  return 0;
}