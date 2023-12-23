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

  fd_set rd_set;
  fd_set monitor_set;
  FD_ZERO(&monitor_set);
  FD_SET(STDIN_FILENO, &monitor_set);
  FD_SET(sock_fd, &monitor_set);
  char buf[4096]{};

  // 接受连接，获取新的套接字描述符
  int net_fd = -1;
  while (true) {
    memcpy(&rd_set, &monitor_set, sizeof(rd_set));
    select(20, &rd_set, nullptr, nullptr, nullptr);

    // 标准输入就绪
    if (FD_ISSET(STDIN_FILENO, &rd_set)) {
      bzero(buf, sizeof(buf));
      ret = static_cast<int>(read(STDIN_FILENO, buf, sizeof(buf)));
      if (ret == 0) {
        // 读取到EOF
        send(net_fd, "chat is ending", 15, 0);
        break;
      }
      send(net_fd, buf, strlen(buf), 0);
    }

    if (FD_ISSET(sock_fd, &rd_set)) {
      net_fd = accept(sock_fd, nullptr, nullptr);
      if (net_fd == -1) {
        perror("accept");
        return -1;
      }
      FD_SET(net_fd, &monitor_set);
      puts("new connection is accept\n");
    }

    // 网络套接字就绪
    if (FD_ISSET(net_fd, &rd_set)) {
      bzero(buf, sizeof(buf));
      ret = static_cast<int>(read(STDIN_FILENO, buf, sizeof(buf)));
      if (ret == 0) {
        // 读取到EOF
        puts("bye bye");
        FD_CLR(net_fd, &monitor_set);
        net_fd = -1;
        continue;
      }
      puts(buf);
    }
  }

  // 关闭
  close(sock_fd);

  return 0;
}