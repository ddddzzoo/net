#include "server.hpp"

const int PORT = 12345;
const int BACKLOG = 5;  // 等待队列的最大长度 超过会被服务器拒绝连接

int main() {
  Server server;
  server.init_socket();
  server.bind_ip_port(nullptr, PORT);
  server.listen_port(BACKLOG);

  while (server.is_run()) {
    server.on_run();
  }

  server.close_socket();
  return 0;
}