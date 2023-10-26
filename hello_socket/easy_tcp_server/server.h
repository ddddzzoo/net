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
#include "timer.h"

#ifndef RCV_BUFF_SIZE
#define RCV_BUFF_SIZE 10240
#endif  // !RCV_BUFF_SIZE

class ClientSocket {
 public:
  explicit ClientSocket(SOCKET socket_fd = INVALID_SOCKET)
      : _socket_fd(socket_fd), _lastPos(0) {
    memset(_msgBuff, 0, sizeof(_msgBuff));
  }

  [[nodiscard]] SOCKET getSocket() const { return _socket_fd; }

  char* msgBuf() { return _msgBuff; }

  [[nodiscard]] int getLastPos() const { return _lastPos; }

  void setLastPos(int pos) { _lastPos = pos; }

 private:
  // socket fd_set
  SOCKET _socket_fd;
  // 第二缓冲区
  char _msgBuff[RCV_BUFF_SIZE * 10]{};
  int _lastPos;
};

class Server {
 public:
  Server() : _server_socket(INVALID_SOCKET), _rcv_count(0) {}
  virtual ~Server() { CloseSocket(); }

  SOCKET InitSocket() {
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
      CloseSocket();
    }
    _server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (INVALID_SOCKET == _server_socket) {
      std::cerr << "Creating socket failed!" << std::endl;
    }
    else {
      std::cout << "Creating socket <" << static_cast<int>(_server_socket)
                << "> success!" << std::endl;
    }
    return _server_socket;
  }

  int Bind(const char* ip, int port) const {
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
    int ret =
        bind(_server_socket, (sockaddr*)&server_addr, sizeof(server_addr));
    if (ret == SOCKET_ERROR) {
      std::cerr << "Binding failed!" << std::endl;
    }
    else {
      std::cout << "Binding success!" << std::endl;
    }
    return ret;
  }

  void Listen(int n) const {
    // 监听请求
    if (SOCKET_ERROR == listen(_server_socket, n)) {
      std::cerr << "Listening failed!" << std::endl;
    }
    else {
      std::cout << "Listening success!" << std::endl;
    }
  }

  void Accept() {
    // 接受客户端
    sockaddr_in client_addr = {};
    int client_addr_len = sizeof(sockaddr_in);
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
      // NewUserJoin new_user_join;
      // SendDataToAll(&new_user_join);
      _clients.emplace_back(new ClientSocket(client_socket));
      // std::cout << "New client: socket = " << (int)client_socket
      //          << ", IP = " << inet_ntoa(client_addr.sin_addr) << std::endl;
    }
  }

  void CloseSocket() {
    if (_server_socket != INVALID_SOCKET) {
#ifdef _WIN32
      for (int i = static_cast<int>(_clients.size()) - 1; i >= 0; --i) {
        closesocket(_clients.at(i)->getSocket());
        delete _clients[i];
      }
      closesocket(_server_socket);
      // 清除 Windows socket环境
      WSACleanup();
#else
      for (int i = static_cast<int>(_clients.size()) - 1; i >= 0; --i) {
        close(_clients.at(i)->getSocket());
        delete _clients[i];
      }
      // 关闭服务器 socket
      close(_server_socket);
#endif  // _WIN32
      _clients.clear();
    }
  }

  [[nodiscard]] bool IsRun() const { return _server_socket != INVALID_SOCKET; }

  bool OnRun() {
    if (IsRun()) {
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
        FD_SET(_clients.at(i)->getSocket(), &fd_read);
        if (maxfd < _clients.at(i)->getSocket()) {
          maxfd = _clients.at(i)->getSocket();
        }
      }

      timeval time = {1, 0};
      int ret = select(static_cast<int>(maxfd) + 1, &fd_read, &fd_write,
                       &fd_exp, &time);
      if (ret < 0) {
        std::cout << "Select over!" << std::endl;
        CloseSocket();
        return false;
      }

      if (FD_ISSET(_server_socket, &fd_read)) {
        FD_CLR(_server_socket, &fd_read);
        Accept();
        return true;
      }
      for (int i = (static_cast<int>(_clients.size()) - 1); i >= 0; --i) {
        if (FD_ISSET(_clients.at(i)->getSocket(), &fd_read)) {
          if (-1 == RcvData(_clients.at(i))) {
            auto iter = _clients.begin() + i;
            if (iter != _clients.end()) {
              delete _clients[i];
              _clients.erase(iter);
            }
          }
        }
      }
      return true;
    }
    return false;
  }

  int RcvData(ClientSocket* client_socket) {
    // 接受客户端消息
    int byte_len = static_cast<int>(
        recv(client_socket->getSocket(), buffer, RCV_BUFF_SIZE, 0));

    if (byte_len <= 0) {
      std::cout << "Client exit!" << std::endl;
      return -1;
    }

    // 将收到的数据拷贝到消息缓冲区
    memcpy(client_socket->msgBuf() + client_socket->getLastPos(), buffer,
           byte_len);
    // 消息缓冲区的数据尾部位置后移
    client_socket->setLastPos(client_socket->getLastPos() + byte_len);
    // 消息缓冲区的数据大于消息头DataHeader的长度
    while (client_socket->getLastPos() >= sizeof(DataHeader)) {
      // 获取当前消息长度
      auto* header = reinterpret_cast<DataHeader*>(client_socket->msgBuf());
      // 消息缓冲区数据大于消息长度
      if (client_socket->getLastPos() >= header->data_length) {
        // 消息缓冲区剩余未处理数据的长度
        int size = client_socket->getLastPos() - header->data_length;
        // 处理网络消息
        OnNetMsg(client_socket->getSocket(), header);
        // 将消息缓冲区剩余未处理的消息前移
        memcpy(client_socket->msgBuf(),
               client_socket->msgBuf() + header->data_length, size);
        // 消息缓冲区尾部位置前移
        client_socket->setLastPos(size);
      }
      else {
        // 消息缓冲区剩余数据不够一条完整的消息
        break;
      }
    }
    return 0;
  }

  virtual void OnNetMsg(SOCKET client_sock, DataHeader* header) {
    _rcv_count++;
    auto t1 = _timer.GetElapsedSeconds();
    if (t1 >= 1.0) {
      std::cout << "Time:" << t1 << " socket:" << client_sock
                << " clients:" << _clients.size() << " rcvCount:" << _rcv_count
                << std::endl;
      _rcv_count = 0;
      _timer.update();
    }

    switch (header->cmd) {
      case CMD_LOGIN: {
        auto login = reinterpret_cast<Login*>(header);
        // std::cout << "Login -- "
        //           << "Socket: " << client_sock
        //           << ", data length: " << login->data_length
        //           << ", username: " << login->username
        //           << ", password: " << login->password << std::endl;
        //// 没有判断用户密码
        // LoginResult ret;
        // send(client_sock, (char*)&ret, sizeof(LoginResult), 0);
      } break;
      case CMD_LOGOUT: {
        auto logout = reinterpret_cast<Logout*>(header);
        // std::cout << "Logout -- "
        //           << "Socket: " << client_sock
        //           << ", data length: " << logout->data_length
        //           << ", username: " << logout->username << std::endl;
        // LogoutResult ret;
        // send(client_sock, (char*)&ret, sizeof(ret), 0);
      } break;
      default:
        std::cerr << "Error!" << std::endl;
        // DataHeader data_header = {0, CMD_ERROR};
        // send(client_sock, (char*)&data_header, sizeof(data_header), 0);
        break;
    }
  }

  size_t SendData(SOCKET client_sock, DataHeader* header) const {
    if ((IsRun() && header)) {
      return send(client_sock, (const char*)header, header->data_length, 0);
    }
    return SOCKET_ERROR;
  }

  void SendDataToAll(DataHeader* header) {
    for (int i = static_cast<int>(_clients.size()) - 1; i >= 0; --i) {
      SendData(_clients.at(i)->getSocket(), header);
    }
  }

 private:
  // 缓冲区
  char buffer[RCV_BUFF_SIZE] = {};

  SOCKET _server_socket;
  std::vector<ClientSocket*> _clients;
  Timer _timer;
  int _rcv_count;
};

#endif  // NET_LEARN_SERVER_HPP
