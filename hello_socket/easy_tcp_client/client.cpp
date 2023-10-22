#include "client.hpp"

#include <thread>

const char* SERVER_IP = "127.0.0.1";
const int SERVER_PORT = 12345;

int main() {
  Client client1;
  client1.connect_server(SERVER_IP, SERVER_PORT);

  Client client2;
  client2.connect_server(SERVER_IP, SERVER_PORT);

  Client client3;
  client3.connect_server(SERVER_IP, SERVER_PORT);

  Login login;
  login.username = "Yee";
  login.password = "YeePWD";
  while (client1.is_run() || client2.is_run() || client3.is_run()) {
    client1.on_run();
    client2.on_run();
    client3.on_run();

    client1.send_data(&login);
    client2.send_data(&login);
    client3.send_data(&login);
  }

  client1.close_socket();
  client2.close_socket();
  client3.close_socket();

  std::cout << "Exit!" << std::endl;
  return 0;
}