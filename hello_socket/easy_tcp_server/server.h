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
#include <thread>
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

  int SendData(DataHeader* header) const {
    if (header) {
      return static_cast<int>(
          send(_socket_fd, (const char*)header, header->data_length, 0));
    }
    return SOCKET_ERROR;
  }

 private:
  // socket fd_set
  SOCKET _socket_fd;
  // 第二缓冲区
  char _msgBuff[RCV_BUFF_SIZE * 10]{};
  int _lastPos;
};

class INetEvent {
 public:
  virtual void OnNetJoin(ClientSocket* client_socket) = 0;
  virtual void OnNetLeave(ClientSocket* client_socket) = 0;
  virtual void OnNetMsg(ClientSocket* client_socket, DataHeader* header) = 0;
};

class CellServer {
 public:
  explicit CellServer(SOCKET socket = INVALID_SOCKET)
      : _socket(socket), _net_event(nullptr) {}
  ~CellServer() { Close(); }

  void SetEventObj(INetEvent* event) { _net_event = event; }

  void Close() {
    if (_socket != INVALID_SOCKET) {
#ifdef _WIN32
      for (int n = (int)_clients.size() - 1; n >= 0; n--) {
        closesocket(_clients[n]->getSocket());
        delete _clients[n];
      }
      closesocket(_socket);
#else
      for (int n = (int)_clients.size() - 1; n >= 0; n--) {
        close(_clients[n]->getSocket());
        delete _clients[n];
      }
      close(_socket);
#endif
      _clients.clear();
    }
  }

  [[nodiscard]] bool IsRun() const { return _socket != INVALID_SOCKET; }

  bool OnRun() {
    while (IsRun()) {
      if (!_clients_buff.empty()) {
        std::lock_guard<std::mutex> lock(_mutex);
        for (auto i : _clients_buff) {
          _clients.emplace_back(i);
        }
        _clients_buff.clear();
      }

      if (_clients.empty()) {
        std::chrono::milliseconds t(1);
        std::this_thread::sleep_for(t);
        continue;
      }

      fd_set fd_read;
      FD_ZERO(&fd_read);
      SOCKET max_socket = _clients[0]->getSocket();
      for (int i = static_cast<int>(_clients.size()) - 1; i >= 0; --i) {
        FD_SET(_clients[i]->getSocket(), &fd_read);
        if (max_socket < _clients[i]->getSocket()) {
          max_socket = _clients[i]->getSocket();
        }
      }

      if (select(max_socket + 1, &fd_read, nullptr, nullptr, nullptr) < 0) {
        printf("Select over!\n");
        Close();
        return false;
      }
      for (int i = static_cast<int>(_clients.size()) - 1; i >= 0; --i) {
        if (FD_ISSET(_clients[i]->getSocket(), &fd_read)) {
          if (RcvData(_clients[i]) == -1) {
            auto iter = _clients.begin() + i;
            if (iter != _clients.end()) {
              if (_net_event) {
                _net_event->OnNetLeave(_clients[i]);
              }
              delete _clients[i];
              _clients.erase(iter);
            }
          }
        }
      }
    }
    return false;
  }

  char _rcv[RCV_BUFF_SIZE] = {};

  int RcvData(ClientSocket* client_socket) {
    int len = static_cast<int>(
        recv(client_socket->getSocket(), _rcv, RCV_BUFF_SIZE, 0));
    if (len <= 0) {
      return -1;
    }
    memcpy(client_socket->msgBuf() + client_socket->getLastPos(), _rcv, len);
    client_socket->setLastPos(client_socket->getLastPos() + len);

    while (client_socket->getLastPos() >= sizeof(DataHeader)) {
      auto header = reinterpret_cast<DataHeader*>(client_socket->msgBuf());
      if (client_socket->getLastPos() >= header->data_length) {
        int size = client_socket->getLastPos() - header->data_length;
        OnNetMsg(client_socket, header);
        memcpy(client_socket->msgBuf(),
               client_socket->msgBuf() + header->data_length, size);
        client_socket->setLastPos(size);
      }
      else {
        break;
      }
    }
    return 0;
  }

  virtual void OnNetMsg(ClientSocket* clientSocket, DataHeader* header) {
    _net_event->OnNetMsg(clientSocket, header);
  }

  void AddClient(ClientSocket* client_socket) {
    std::lock_guard<std::mutex> lock(_mutex);
    _clients_buff.emplace_back(client_socket);
  }

  void Start() { _thread = std::thread(std::mem_fn(&CellServer::OnRun), this); }

  size_t GetClientCount() { return _clients.size() + _clients_buff.size(); }

 private:
  SOCKET _socket;
  std::vector<ClientSocket*> _clients;
  std::vector<ClientSocket*> _clients_buff;
  std::mutex _mutex;
  std::thread _thread;
  INetEvent* _net_event;
};

class Server : public INetEvent {
 public:
  Server() : _server_socket(INVALID_SOCKET), _rcv_count(0), _client_count(0) {}
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
      AddClientToCellServer(new ClientSocket(client_socket));
    }
  }

  void AddClientToCellServer(ClientSocket* client_socket) {
    auto min_server = _cell_servers[0];
    for (auto item : _cell_servers) {
      if (min_server->GetClientCount() > item->GetClientCount()) {
        min_server = item;
      }
    }
    min_server->AddClient(client_socket);
    OnNetJoin(client_socket);
  }

  void Start(int server_count) {
    for (int i = 0; i < server_count; ++i) {
      auto ser = new CellServer(_server_socket);
      ser->SetEventObj(this);
      ser->Start();
    }
  }

  void CloseSocket() const {
    if (_server_socket != INVALID_SOCKET) {
#ifdef _WIN32
      closesocket(_server_socket);
      // 清除 Windows socket环境
      WSACleanup();
#else
      // 关闭服务器 socket
      close(_server_socket);
#endif  // _WIN32
    }
  }

  [[nodiscard]] bool IsRun() const { return _server_socket != INVALID_SOCKET; }

  bool OnRun() {
    if (IsRun()) {
      TimeForMsg();
      fd_set fd_read;
      FD_ZERO(&fd_read);
      FD_SET(_server_socket, &fd_read);
      timeval t = {0, 10};
      if (select(_server_socket + 1, &fd_read, nullptr, nullptr, &t)) {
        printf("Select over!\n");
        CloseSocket();
        return false;
      }
      if (FD_ISSET(_server_socket, &fd_read)) {
        FD_CLR(_server_socket, &fd_read);
        Accept();
        return true;
      }
      return true;
    }
    return false;
  }

  void TimeForMsg() {
    auto t1 = _timer.GetElapsedSeconds();
    if (t1 >= 1.0) {
      printf("Thread:%d,time:%lf,socket:%d,client:%d,rcv_count:%d\n",
             static_cast<int>(_cell_servers.size()), t1, _server_socket,
             static_cast<int>(_client_count), static_cast<int>(_rcv_count));
      _rcv_count = 0;
      _timer.update();
    }
  }

  virtual void OnNetMsg(SOCKET client_sock, DataHeader* header) {
    _rcv_count++;
  }

 private:
  SOCKET _server_socket;
  std::vector<CellServer*> _cell_servers;
  Timer _timer;

 protected:
  std::atomic_int _rcv_count;
  std::atomic_int _client_count;
};

#endif  // NET_LEARN_SERVER_HPP
