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
  addr.sin_port = htons(1234);
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

  // 创建文件描述符集合
  fd_set rd_set;       // 单纯保存就绪的fd
  fd_set monitor_set;  // 使用一个单独的监听集合
  FD_ZERO(&monitor_set);
  FD_SET(STDIN_FILENO, &monitor_set);
  FD_SET(sock_fd, &monitor_set);

  char buf[4096]{};
  int net_fd_arr[10]{};  // 存储已连接的套接字描述符
  int cur_conn = 0;      // 当前连接数

  // 接受连接，获取新的套接字描述符
  while (true) {
    memcpy(&rd_set, &monitor_set, sizeof(rd_set));

    select(20, &rd_set, nullptr, nullptr, nullptr);

    if (FD_ISSET(sock_fd, &rd_set)) {
      net_fd_arr[cur_conn] = accept(sock_fd, nullptr, nullptr);
      if (net_fd_arr[cur_conn] == -1) {
        perror("accept");
        return -1;
      }
      FD_SET(net_fd_arr[cur_conn], &monitor_set);
      printf("New connection accepted! cur conn = %d\n", cur_conn);
      ++cur_conn;
    }

    for (int i = 0; i < cur_conn; ++i) {
      if (FD_ISSET(net_fd_arr[i], &rd_set)) {
        bzero(buf, sizeof(buf));
        recv(net_fd_arr[i], buf, sizeof(buf), 0);
        for (int j = 0; j < cur_conn; ++j) {
          if (j == i) {
            continue;
          }
          send(net_fd_arr[j], buf, strlen(buf), 0);
        }
      }
    }
  }

  // 关闭
  close(sock_fd);

  return 0;
}