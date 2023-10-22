#ifndef NET_LEARN_SERVER_HPP
#define NET_LEARN_SERVER_HPP

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
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
#include <vector>

#include "message_header.hpp"

class Server {
 public:
  Server();
  virtual ~Server();

  void close_socket();

  SOCKET init_socket();

  int bind_ip_port(const char* ip, int port) const;

  void listen_port(int n) const;

  [[nodiscard]] bool is_run() const { return _server_socket != INVALID_SOCKET; }

  bool on_run();

  size_t send_data(SOCKET client_socket, DataHeader* header) const;

  void send_data_to_all(DataHeader* header);

 private:
  static void on_net_msg(SOCKET client_socket, DataHeader* header);

  void accept_client();

  static int receive_data(SOCKET client_socket);

  SOCKET _server_socket;
  std::vector<SOCKET> _clients;
};

Server::Server() : _server_socket(INVALID_SOCKET) {}

Server::~Server() { close_socket(); }

inline void Server::close_socket() {
  if (_server_socket != INVALID_SOCKET) {
#ifdef _WIN32
    for (int i = static_cast<int>(_clients.size()) - 1; i >= 0; --i) {
      closesocket(_clients.at(i));
    }
    closesocket(server_socket);
    // 清除 Windows socket环境
    WSACleanup();
#else
    for (int i = static_cast<int>(_clients.size()) - 1; i >= 0; --i) {
      close(_clients.at(i));
    }
    // 关闭服务器 socket
    close(_server_socket);
#endif  // _WIN32
  }
}

inline int Server::init_socket() {
#ifdef _WIN32
  // 初始化 Windows 套接字 DLL
  WSADATA wsa_data;
  if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
    std::cout << "WSAStartup failed!" << std::endl;
  }
#endif  // _WIN32
  if (_server_socket != INVALID_SOCKET) {
    std::cerr << "Close old connection socket = " << _server_socket << " !"
              << std::endl;
    close_socket();
  }
  _server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (INVALID_SOCKET == _server_socket) {
    std::cerr << "Creating socket failed!" << std::endl;
  }
  else {
    std::cout << "Creating socket <" << _server_socket << "> success!"
              << std::endl;
  }
  return _server_socket;
}

inline int Server::bind_ip_port(const char* ip, int port) const {
  // 配置服务器地址
  sockaddr_in server_addr = {};
  server_addr.sin_family = AF_INET;    // 使用IPV4
  server_addr.sin_port = htons(port);  // host to network short

#ifdef _WIN32
  if (ip) {
    server_addr.sin_addr.S_un.S_addr = inet_addr(ip);
  }
  else {
    server_addr.sin_addr.S_un.S_addr = INADDR_ANY;
  }
#else
  if (ip) {
    server_addr.sin_addr.s_addr = inet_addr(ip);
  }
  else {
    server_addr.sin_addr.s_addr =
        INADDR_ANY;  // 设置为相应的IP地址 INADDR_ANY 接受来自任何IP的请求
  }
#endif  // _WIN32
  int ret = bind(_server_socket, (sockaddr*)&server_addr, sizeof(server_addr));
  if (ret == SOCKET_ERROR) {
    std::cerr << "Binding failed!" << std::endl;
  }
  else {
    std::cout << "Binding success!" << std::endl;
  }
  return ret;
}

inline void Server::listen_port(int n) const {
  // 监听请求
  int ret = listen(_server_socket, n);
  if (SOCKET_ERROR == ret) {
    std::cerr << "Listening failed!" << std::endl;
  }
  else {
    std::cout << "Listening success!" << std::endl;
  }
}

inline void Server::accept_client() {
  // 接受客户端
  sockaddr_in client_addr = {};
  int client_addr_len = sizeof(client_addr);
  auto client_socket = INVALID_SOCKET;
#ifdef _WIN32
  client_socket =
      accept(_server_socket, (sockaddr*)&client_socket, &client_addr_len);
#else
  client_socket = accept(_server_socket, (sockaddr*)&client_addr,
                         (socklen_t*)&client_addr_len);
#endif  // _WIN32
  if (INVALID_SOCKET == client_socket) {
    std::cerr << "Accepting socket failed!" << std::endl;
  }
  else {
    NewUserJoin new_user_join;
    send_data_to_all(&new_user_join);
    _clients.emplace_back(client_socket);
    std::cout << "New client: socket = " << (int)client_socket
              << ", IP = " << inet_ntoa(client_addr.sin_addr) << std::endl;
  }
}

inline bool Server::on_run() {
  if (is_run()) {
    // 描述符（socket）集合
    fd_set fd_read{};   // 可读状态文件描述符
    fd_set fd_write{};  // 可写状态文件描述符
    fd_set fd_exp{};    // 异常状态文件描述符
    // 清理集合
    FD_ZERO(&fd_read);
    FD_ZERO(&fd_write);
    FD_ZERO(&fd_exp);
    // 将描述符加入集合
    FD_SET(_server_socket, &fd_read);
    FD_SET(_server_socket, &fd_write);
    FD_SET(_server_socket, &fd_exp);

    SOCKET maxfd = _server_socket;
    for (int i = static_cast<int>(_clients.size()) - 1; i >= 0; --i) {
      FD_SET(_clients.at(i), &fd_read);
      if (maxfd < _clients.at(i)) {
        maxfd = _clients.at(i);
      }
    }

    timeval time = {1, 0};
    int ret = select(maxfd + 1, &fd_read, &fd_write, &fd_exp, &time);
    if (ret < 0) {
      std::cout << "Select over!" << std::endl;
      close_socket();
      return false;
    }

    if (FD_ISSET(_server_socket, &fd_read)) {
      FD_CLR(_server_socket, &fd_read);
      accept_client();
    }
    for (int i = static_cast<int>(_clients.size()) - 1; i >= 0; --i) {
      if (FD_ISSET(_clients.at(i), &fd_read)) {
        if (-1 == receive_data(_clients.at(i))) {
          auto iter = _clients.begin() + i;
          if (iter != _clients.end()) {
            _clients.erase(iter);
          }
        }
      }
    }
    return true;
  }
  return false;
}

inline int Server::receive_data(int client_socket) {
  // 缓冲区
  char buffer[4096] = {};
  // 接受客户端消息
  int byte_len =
      static_cast<int>(recv(client_socket, buffer, sizeof(DataHeader), 0));
  auto* header = reinterpret_cast<DataHeader*>(buffer);

  if (byte_len <= 0) {
    std::cout << "Client exit!" << std::endl;
    return -1;
  }
  recv(client_socket, buffer + sizeof(DataHeader),
       header->data_length - sizeof(DataHeader), 0);
  on_net_msg(client_socket, header);
  return 0;
}

inline void Server::on_net_msg(int client_socket, DataHeader* header) {
  switch (header->cmd) {
    case CMD_LOGIN: {
      auto login = reinterpret_cast<Login*>(header);
      std::cout << "Login -- "
                << "Socket: " << client_socket
                << ", data length: " << login->data_length
                << ", username: " << login->username
                << ", password: " << login->password << std::endl;
      // 没有判断用户密码
      LoginResult ret;
      send(client_socket, (char*)&ret, sizeof(LoginResult), 0);
    } break;
    case CMD_LOGOUT: {
      auto logout = reinterpret_cast<Logout*>(header);
      std::cout << "Logout -- "
                << "Socket: " << client_socket
                << ", data length: " << logout->data_length
                << ", username: " << logout->username << std::endl;
      LogoutResult ret;
      send(client_socket, (char*)&ret, sizeof(ret), 0);
    } break;
    default:
      header->cmd = CMD_ERROR;
      header->data_length = 0;
      send(client_socket, (char*)&header, sizeof(*header), 0);
      break;
  }
}

inline size_t Server::send_data(int client_socket, DataHeader* header) const {
  if ((is_run() && header)) {
    return send(client_socket, (const char*)header, header->data_length, 0);
  }
  return SOCKET_ERROR;
}

inline void Server::send_data_to_all(DataHeader* header) {
  for (int i = static_cast<int>(_clients.size()) - 1; i >= 0; --i) {
    send_data(_clients.at(i), header);
  }
}

#endif  // NET_LEARN_SERVER_HPP
