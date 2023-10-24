#include "server.hpp"

#include <thread>

const int PORT = 12345;
const int BACKLOG = 5;  // 等待队列的最大长度 超过会被服务器拒绝连接

bool g_run = true;
void CmdThread() {
  while (true) {
    std::string cmd;
    std::cin >> cmd;
    if (cmd == "exit") {
      g_run = false;
      std::cout << "Exit CmdThread!" << std::endl;
    }
    else {
      std::cout << "Error Cmd!" << std::endl;
    }
  }
}

int main() {
  Server server;
  server.InitSocket();
  server.Bind(nullptr, PORT);
  server.Listen(BACKLOG);

  std::thread t1(CmdThread);
  t1.detach();

  while (g_run) {
    server.OnRun();
  }

  server.CloseSocket();
  return 0;
}