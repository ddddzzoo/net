#include "client.hpp"

#include <thread>

const char* SERVER_IP = "127.0.0.1";
const int SERVER_PORT = 12345;

void cmd_thread(Client client) {
  while (true) {
    std::string cmd;
    std::cin >> cmd;
    if ("exit" == cmd) {
      client.close_socket();
      std::cout << "Exit cmd thread!" << std::endl;
      break;
    }
    else if ("login" == cmd) {
      Login login;
      login.username = "username";
      login.password = "password";
      // 发送消息给服务器
      client.send_data(&login);
    }
    else if ("logout" == cmd) {
      Logout logout;
      logout.username = "username";
      // 发送消息给服务器
      client.send_data(&logout);
    }
    else {
      std::cout << "Unknown cmd!" << std::endl;
    }
  }
}

int main() {
  Client client;
  client.connect_server(SERVER_IP, SERVER_PORT);

  std::thread t1(cmd_thread, &client);
  t1.detach();

  while (client.is_run()) {
    client.on_run();
  }

  client.close_socket();

  return 0;
}