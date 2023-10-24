#ifndef NET_LEARN_CLIENT_HPP
#define NET_LEARN_CLIENT_HPP

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

#ifndef RCV_BUFF_SIZE
#define RCV_BUFF_SIZE 102400
#endif  // !RCV_BUFF_SIZE

class Client {
 public:
  Client() : _client_socket(INVALID_SOCKET) {}
  virtual ~Client() { CloseSocket(); }

  // 连接服务器
  int ConnectServer(const char* ip, unsigned short port) {
    if (INVALID_SOCKET == _client_socket) {
      InitSocket();
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
    std::cout << "Socket:<" << _client_socket << "> is connecting server!"
              << std::endl;
    int ret =
        connect(_client_socket, (sockaddr*)&server_addr, sizeof(server_addr));

    if (SOCKET_ERROR == ret) {
      std::cerr << "Connecting socket failed!" << std::endl;
    }
    else {
      std::cout << "Socket:<" << _client_socket
                << "> connecting server success!" << std::endl;
    }

    return ret;
  }

  // 关闭 socket
  void CloseSocket() {
    if (_client_socket != INVALID_SOCKET) {
#ifdef _WIN32
      closesocket(_client_socket);
      // 清除Windows socket环境
      WSACleanup();
#else
      close(_client_socket);
#endif  // _WIN32
      _client_socket = INVALID_SOCKET;
    }
  }

  // 发送数据
  size_t SendData(DataHeader* header) const {
    if (IsRun() && header) {
      return send(_client_socket, (const char*)header, header->data_length, 0);
    }
    return SOCKET_ERROR;
  }

  // 是否在工作
  [[nodiscard]] bool IsRun() const { return _client_socket != INVALID_SOCKET; }

  // 处理网络消息
  bool OnRun() {
    if (IsRun()) {
      fd_set fd_read{};
      FD_ZERO(&fd_read);
      FD_SET(_client_socket, &fd_read);
      timeval time{0, 0};
      int ret = select(_client_socket + 1, &fd_read, nullptr, nullptr, &time);

      if (ret < 0) {
        std::cout << "Select over!" << std::endl;
        CloseSocket();
        return false;
      }

      if (FD_ISSET(_client_socket, &fd_read)) {
        FD_CLR(_client_socket, &fd_read);

        if (-1 == RcvData()) {
          std::cout << "Select over!!" << std::endl;
          CloseSocket();
          return false;
        }
      }
      return true;
    }
    return false;
  }

 private:
  // 初始化 socket
  void InitSocket() {
#ifdef _WIN32
    // 初始化 Windows 套接字 DLL
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
      std::cout << "WSAStartup failed!" << std::endl;
    }
#endif  // _WIN32

    if (INVALID_SOCKET != _client_socket) {
      std::cerr << "Close old connection socket = " << _client_socket << " !"
                << std::endl;
      CloseSocket();
    }
    _client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (INVALID_SOCKET == _client_socket) {
      std::cerr << "Creating socket failed!" << std::endl;
    }
    else {
      std::cout << "Creating socket <" << _client_socket << "> success!"
                << std::endl;
    }
  }

  // 响应网络消息
  static void OnNetMsg(DataHeader* header) {
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
      case CMD_ERROR: {
        std::cout << "CMD_ERROR -- Data length: " << header->data_length
                  << std::endl;
      } break;
      default: {
        std::cout << "Unknown -- Data length: " << header->data_length
                  << std::endl;
      }
    }
  }

  // 接受数据处理
  [[nodiscard]] int RcvData() {
    int byte_len =
        static_cast<int>(recv(_client_socket, _buffer, RCV_BUFF_SIZE, 0));

    if (byte_len <= 0) {
      std::cout << "Connection lost!" << std::endl;
      return -1;
    }
    // 将收到的消息拷贝到消息缓冲区
    memcpy(_msgBuf + _lastPos, _buffer, byte_len);
    // 消息缓冲区的数据尾部位置后移
    _lastPos += byte_len;
    // 消息缓冲区的数据大于消息头DataHeader长度
    while (_lastPos >= sizeof(DataHeader)) {
      // 获取当前消息长度
      auto* header = reinterpret_cast<DataHeader*>(_msgBuf);
      // 消息缓冲区长度大于消息长度
      if (_lastPos >= header->data_length) {
        // 消息缓冲区剩余未处理的长度
        int size = _lastPos - header->data_length;
        // 处理消息
        OnNetMsg(header);
        // 将未处理消息前移
        memcpy(_msgBuf, _msgBuf + header->data_length, size);
        // 消息缓冲区数据尾部位置前移
        _lastPos = size;
      }
      else {
        // 缓冲区数据不够一条完整消息
        break;
      }
    }
    return 0;
  }

  SOCKET _client_socket;

  // 消息缓冲区
  char _msgBuf[RCV_BUFF_SIZE * 10] = {};
  // 消息缓冲区尾部位置
  int _lastPos = 0;
  // 接收缓冲区
  char _buffer[RCV_BUFF_SIZE] = {};
};

#endif  // NET_LEARN_CLIENT_HPP