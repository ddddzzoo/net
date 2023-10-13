#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Ws2tcpip.h>
#include <windows.h>
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#define SOCKET int
#define INVALID_SOCKET (SOCKET)(~0)
#define SOCKET_ERROR (-1)
#endif  // _WIN32

#include <iostream>
#include <string>
#include <thread>

const char* SERVER_IP = "127.0.0.1";
const int SERVER_PORT = 12345;

// 使用结构体模拟用户登陆请求
// 头部
enum CMD {
  CMD_LOGIN,
  CMD_LOGIN_RESULT,
  CMD_LOGOUT,
  CMD_LOGOUT_RESULT,
  CMD_NEW_USER_JOIN,
  CMD_ERROR
};

struct DataHeader {
  short data_length;  // 数据长度
  short cmd;          // 数据作用
};

// 数据部分
struct Login : public DataHeader {
  Login() : DataHeader() {
    data_length = sizeof(Login);
    cmd = CMD_LOGIN;
  }

  std::string username;
  std::string password;
};

struct LoginResult : public DataHeader {
  LoginResult() : DataHeader(), result(0) {
    data_length = sizeof(LoginResult);
    cmd = CMD_LOGIN_RESULT;
  }

  int result;
};

struct Logout : public DataHeader {
  Logout() : DataHeader() {
    data_length = sizeof(Logout);
    cmd = CMD_LOGOUT;
  }

  std::string username;
};

struct LogoutResult : public DataHeader {
  LogoutResult() : DataHeader(), result(0) {
    data_length = sizeof(LoginResult);
    cmd = CMD_LOGOUT_RESULT;
  }

  int result;
};

struct NewUserJoin : public DataHeader {
  NewUserJoin() : DataHeader(), socket(0) {
    data_length = sizeof(NewUserJoin);
    cmd = CMD_NEW_USER_JOIN;
  }

  int socket;
};

int processor(int client_socket) {
  // 缓冲区
  char buffer[4096] = {};

  int byte_len =
      static_cast<int>(recv(client_socket, buffer, sizeof(DataHeader), 0));
  auto* header = reinterpret_cast<DataHeader*>(buffer);

  if (byte_len <= 0) {
    std::cout << "Connection lost!" << std::endl;
    return -1;
  }

  switch (header->cmd) {
    case CMD_LOGIN_RESULT: {
      recv(client_socket, buffer + sizeof(DataHeader),
           header->data_length - sizeof(DataHeader), 0);
      auto login_result = reinterpret_cast<LoginResult*>(buffer);
      std::cout << "CMD_LOGIN_RESULT -- Data length: "
                << login_result->data_length << std::endl;
    } break;
    case CMD_LOGOUT_RESULT: {
      recv(client_socket, buffer + sizeof(DataHeader),
           header->data_length - sizeof(DataHeader), 0);
      auto logout_result = reinterpret_cast<LogoutResult*>(buffer);
      std::cout << "CMD_LOGOUT_RESULT -- Data length: "
                << logout_result->data_length << std::endl;
    } break;
    case CMD_NEW_USER_JOIN: {
      recv(client_socket, buffer + sizeof(DataHeader),
           header->data_length - sizeof(DataHeader), 0);
      auto new_user_join = reinterpret_cast<NewUserJoin*>(buffer);
      std::cout << "CMD_NEW_USER_JOIN -- Data length: "
                << new_user_join->data_length << std::endl;
    } break;
  }
  return 0;
}

bool run = true;

void cmd_thread(int client_socket) {
  while (true) {
    std::string cmd;
    std::cin >> cmd;
    if ("exit" == cmd) {
      run = false;
      std::cout << "Exit cmd thread!" << std::endl;
      break;
    }
    else if ("login" == cmd) {
      Login login;
      login.username = "username";
      login.password = "password";
      // 发送消息给服务器
      send(client_socket, (const char*)&login, sizeof(login), 0);
    }
    else if ("logout" == cmd) {
      Logout logout;
      logout.username = "username";
      // 发送消息给服务器
      send(client_socket, (const char*)&logout, sizeof(logout), 0);
    }
    else {
      std::cout << "Unknown cmd!" << std::endl;
    }
  }
}

int main() {
#ifdef _WIN32
  // 初始化 Windows 套接字 DLL
  WSADATA wsa_data;
  if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
    std::cout << "WSAStartup failed!" << std::endl;
    return 1;
  }
#endif  // _WIN32

  // 创建客户端 socket
  SOCKET client_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (INVALID_SOCKET == client_socket) {
    std::cerr << "Creating socket failed!" << std::endl;
#ifdef _WIN32
    WSACleanup();
#endif  // _WIN32
    return 1;
  }

  // 配置服务器地址
  sockaddr_in server_addr = {};
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(SERVER_PORT);
#ifdef _WIN32
  InetPton(AF_INET, SERVER_IP, &server_addr.sin_addr);
#else
  inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);
#endif  // _WIN32

  // 连接服务器
  if (connect(client_socket, (sockaddr*)&server_addr, sizeof(server_addr)) ==
      -1) {
    std::cerr << "Connecting socket failed!" << std::endl;

#ifdef _WIN32
    closesocket(client_socket);
    WSACleanup();
#else
    close(client_socket);
#endif  // _WIN32
    return 1;
  }

  std::thread t1(cmd_thread, client_socket);
  t1.detach();

  while (run) {
    fd_set fd_read{};
    FD_ZERO(&fd_read);
    FD_SET(client_socket, &fd_read);
    timeval time{1, 0};
    int ret = select(client_socket + 1, &fd_read, nullptr, nullptr, &time);

    if (ret < 0) {
      std::cout << "Select over!" << std::endl;
      break;
    }

    if (FD_ISSET(client_socket, &fd_read)) {
      FD_CLR(client_socket, &fd_read);

      if (-1 == processor(client_socket)) {
        std::cout << "Select over!!" << std::endl;
        break;
      }
    }
  }

  // 关闭客户端 socket
#ifdef _WIN32
  closesocket(client_socket);
  WSACleanup();
#else
  close(client_socket);
#endif  // _WIN32

  return 0;
}