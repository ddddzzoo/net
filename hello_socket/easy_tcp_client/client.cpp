#include "client.hpp"

#include <thread>

const char* SERVER_IP = "127.0.0.1";
const int SERVER_PORT = 12345;

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

int main() {
  const int count = FD_SETSIZE - 1;
  Client* client[count]{};

  for (auto & i : client) {
    i = new Client();
  }

  for (auto & i : client) {
    i->ConnectServer(SERVER_IP, SERVER_PORT);
  }

  std::thread t1(CmdThread);
  t1.detach();

  Login login;
  login.username = "Yee";
  login.password = "YeePWD";
  while (g_run) {
    for (auto & i : client) {
      i->SendData(&login);
      i->OnRun();
    }
  }

  for (auto & i : client) {
    i->CloseSocket();
  }

  std::cout << "Exit!" << std::endl;
  return 0;
}