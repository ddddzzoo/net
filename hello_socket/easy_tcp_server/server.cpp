#include "server.h"

#include <thread>

const int PORT = 12345;
const int BACKLOG = 5;  // 等待队列的最大长度 超过会被服务器拒绝连接

bool g_run = true;
[[noreturn]] void CmdThread() {
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

class MyServer : public Server {
 public:
  void OnNetJoin(ClientSocket *client_socket) override {
    _client_count++;
    printf("Client:%d join\n", client_socket->getSocket());
  }

  void OnNetLeave(ClientSocket *client_socket) override {
    _client_count--;
    printf("Client:%d leave\n", client_socket->getSocket());
  }

  void OnNetMsg(ClientSocket *client_socket, DataHeader *header) override {
    _rcv_count++;
    switch (header->cmd) {
      case CMD_LOGIN: {
        auto *login = (Login *)header;
        LoginResult ret;
        client_socket->SendData(&ret);
      } break;
      case CMD_LOGOUT: {
        auto *logout = (Logout *)header;
      } break;
      default: {
        printf("Unknown msg form socket:%d,data length:%d\n",
               client_socket->getSocket(), header->data_length);
      } break;
    }
  }
};

int main() {
  MyServer server;
  server.InitSocket();
  server.Bind(nullptr, PORT);
  server.Listen(BACKLOG);
  server.Start(4);

  std::thread t1(CmdThread);
  t1.detach();

  while (g_run) {
    server.OnRun();
  }

  server.CloseSocket();
  return 0;
}