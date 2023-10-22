#ifndef _client_hpp_
#define _client_hpp_

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

#include "message_header.hpp"

class Client {
 public:
  Client();
  virtual ~Client();

  // 连接服务器
  int connect_server(const char* ip, unsigned short port);

  // 关闭 socket
  void close_socket();

  // 发送数据
  int send_data(DataHeader* header);

  // 是否在工作
  bool is_run() { return _client_socket != INVALID_SOCKET; }

  // 处理网络消息
  bool on_run();

 private:
  // 初始化 socket
  void init_socket();

  // 响应网络消息
  void on_net_msg(DataHeader* header);

  // 接受数据处理
  int recv_data();

  SOCKET _client_socket;
};

Client::Client() : _client_socket(INVALID_SOCKET) {}

Client::~Client() { close_socket(); }

inline void Client::init_socket() {
#ifdef _WIN32
  // 初始化 Windows 套接字 DLL
  WSADATA wsa_data;
  if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
    std::cout << "WSAStartup failed!" << std::endl;
    close_socket();
  }
#endif  // _WIN32

  if (INVALID_SOCKET != _client_socket) {
    std::cerr << "Close old connection sockect = " << _client_socket << " !"
              << std::endl;
  }

  _client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (INVALID_SOCKET == _client_socket) {
    std::cerr << "Creating socket failed!" << std::endl;
  }
}

inline int Client::connect_server(const char* ip, unsigned short port) {
  if (INVALID_SOCKET == _client_socket) {
    init_socket();
  }

  // 配置服务器地址
  sockaddr_in server_addr = {};
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
#ifdef _WIN32
  InetPton(AF_INET, ip, &server_addr.sin_addr);
#else
  inet_pton(AF_INET, ip, &server_addr.sin_addr);
#endif  // _WIN32

  // 连接服务器
  int ret =
      connect(_client_socket, (sockaddr*)&server_addr, sizeof(server_addr));

  if (SOCKET_ERROR == ret) {
    std::cerr << "Connecting socket failed!" << std::endl;
  }

  return ret;
}

inline void Client::close_socket() {
  if (_client_socket != INVALID_SOCKET) {
#ifdef _WIN32
    closesocket(_client_socket);
    // 清除Windows socket环境
    WSACleanup();
#else
    close(_client_socket);
#endif  // _WIN32
  }
}

inline int Client::recv_data() {
  // 缓冲区
  char buffer[4096] = {};

  int byte_len =
      static_cast<int>(recv(_client_socket, buffer, sizeof(DataHeader), 0));
  auto* header = reinterpret_cast<DataHeader*>(buffer);

  if (byte_len <= 0) {
    std::cout << "Connection lost!" << std::endl;
    return -1;
  }
  recv(_client_socket, buffer + sizeof(DataHeader),
       header->data_length - sizeof(DataHeader), 0);
  on_net_msg(header);
  return 0;
}

inline bool Client::on_run() {
  if (is_run()) {
    fd_set fd_read{};
    FD_ZERO(&fd_read);
    FD_SET(_client_socket, &fd_read);
    timeval time{1, 0};
    int ret = select(_client_socket + 1, &fd_read, nullptr, nullptr, &time);

    if (ret < 0) {
      std::cout << "Select over!" << std::endl;
      return false;
    }

    if (FD_ISSET(_client_socket, &fd_read)) {
      FD_CLR(_client_socket, &fd_read);

      if (-1 == recv_data()) {
        std::cout << "Select over!!" << std::endl;
        return false;
      }
    }
    return true;
  }
  return false;
}

inline void Client::on_net_msg(DataHeader* header) {
  switch (header->cmd) {
    case CMD_LOGIN_RESULT: {
      auto login_result = reinterpret_cast<LoginResult*>(header);
      std::cout << "CMD_LOGIN_RESULT -- Data length: "
                << login_result->data_length << std::endl;
    } break;
    case CMD_LOGOUT_RESULT: {
      auto logout_result = reinterpret_cast<LogoutResult*>(header);
      std::cout << "CMD_LOGOUT_RESULT -- Data length: "
                << logout_result->data_length << std::endl;
    } break;
    case CMD_NEW_USER_JOIN: {
      auto new_user_join = reinterpret_cast<NewUserJoin*>(header);
      std::cout << "CMD_NEW_USER_JOIN -- Data length: "
                << new_user_join->data_length << std::endl;
    } break;
  }
}

inline int Client::send_data(DataHeader* header) {
  if (is_run() && header) {
    return send(_client_socket, (const char*)&header, header->data_length, 0);
  }
  return SOCKET_ERROR;
}

#endif