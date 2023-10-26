#include "client.h"

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

// 客户端数量
const int client_count = 8;
// 线程数量
const int thread_count = 4;
// 客户端数组
Client* clients_[client_count];

void SendThread(int thread_id) {
  printf("Thread:%d start!\n", thread_id);
  int thread_clients = client_count / thread_count;
  int begin = (thread_id - 1) * thread_clients;
  int end = thread_id * thread_clients;

  for (int i = begin; i < end; ++i) {
    clients_[i] = new Client();
  }

  for (int i = begin; i < end; ++i) {
    clients_[i]->ConnectServer(SERVER_IP, SERVER_PORT);
  }

  printf("Thread:%d,Connect -- begin:%d,end:%d!\n", thread_id, begin, end);

  std::chrono::milliseconds t(3000);
  std::this_thread::sleep_for(t);
  Login login[10];
  for (auto& i : login) {
    i.username = "Yee";
    i.password = "YeePWD";
  }

  int len = sizeof(login);

  while (g_run) {
    for (int i = begin; i < end; ++i) {
      clients_[i]->SendData(login, len);
      clients_[i]->OnRun();
    }
  }

  for (int i = begin; i < end; ++i) {
    clients_[i]->CloseSocket();
    delete clients_[i];
  }

  printf("Thread:%d exit!\n", thread_id);
}

int main() {
  std::thread t1(CmdThread);
  t1.detach();

  for (int i = 0; i < thread_count; ++i) {
    std::thread t(SendThread, i + 1);
    t.detach();
  }

  while (g_run) {
    sleep(100);
  }

  std::cout << "Exit!" << std::endl;
  return 0;
}