#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

const int PORT = 12345;
const int BACKLOG = 5; // 等待队列的最大长度 超过会被服务器拒绝连接

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
    short data_length; // 数据长度
    short cmd;  // 数据作用
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

int processor(int clientSocket) {
    // 缓冲区
    char buffer[4096] = {};
    // 接受客户端消息
    int byte_len = static_cast<int>( recv(clientSocket, buffer, sizeof(DataHeader), 0));
    auto *header = reinterpret_cast<DataHeader *>(buffer);

    if (byte_len <= 0) {
        std::cout << "Client exit!" << std::endl;
        return -1;
    }

    switch (header->cmd) {
        case CMD_LOGIN: {
            recv(clientSocket, buffer + sizeof(DataHeader), header->data_length - sizeof(DataHeader), 0);
            auto login = reinterpret_cast<Login *>(buffer);
            std::cout << "Login -- " << "Socket: " << clientSocket << ", data length: " << login->data_length
                      << ", username: " << login->username << ", password: " << login->password << std::endl;
            // 没有判断用户密码
            LoginResult ret;
            send(clientSocket, (char *) &ret, sizeof(LoginResult), 0);
        }
            break;
        case CMD_LOGOUT: {
            recv(clientSocket, buffer + sizeof(DataHeader), header->data_length - sizeof(DataHeader), 0);
            auto logout = reinterpret_cast<Logout *>(buffer);
            std::cout << "Logout -- " << "Socket: " << clientSocket << ", data length: " << logout->data_length
                      << ", username: " << logout->username << std::endl;
            LogoutResult ret;
            send(clientSocket, (char *) &ret, sizeof(ret), 0);
        }
            break;
        default:
            header->cmd = CMD_ERROR;
            header->data_length = 0;
            send(clientSocket, (char *) &header, sizeof(*header), 0);
            break;
    }
    return 0;
}

int main() {
    // 客户端套接字集合
    std::vector<int> clients;
    // 创建服务器套接字
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Creating socket failed!" << std::endl;
        return 1;
    }

    // 配置服务器地址
    sockaddr_in serverAddr = {};
    serverAddr.sin_family = AF_INET; // 使用IPV4
    serverAddr.sin_port = htons(PORT); // host to network short
    serverAddr.sin_addr.s_addr = INADDR_ANY; // 设置为相应的IP地址 INADDR_ANY 接受来自任何IP的请求

    // 绑定服务器 socket
    if (bind(serverSocket, (sockaddr *) &serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "Binding socket failed!" << std::endl;
        return 1;
    }

    // 监听请求
    if (listen(serverSocket, BACKLOG) == -1) {
        std::cerr << "Listening socket failed!" << std::endl;
        close(serverSocket);
        return 1;
    }

    std::cout << "Server is listening on port " << PORT << std::endl;

    while (true) {
        // 描述符（socket）集合
        fd_set fd_read; // 可读状态文件描述符
        fd_set fd_write; // 可写状态文件描述符
        fd_set fd_exp; // 异常状态文件描述符
        // 清理集合
        FD_ZERO(&fd_read);
        FD_ZERO(&fd_write);
        FD_ZERO(&fd_exp);
        // 将描述符加入集合
        FD_SET(serverSocket, &fd_read);
        FD_SET(serverSocket, &fd_write);
        FD_SET(serverSocket, &fd_exp);

        int maxfd = serverSocket;
        for (int i = static_cast<int>( clients.size() - 1); i >= 0; --i) {
            FD_SET(clients.at(i), &fd_read);
            if (maxfd < clients.at(i)) {
                maxfd = clients.at(i);
            }
        }

        timeval time = {1, 0};
        int ret = select(maxfd + 1, &fd_read, &fd_write, &fd_exp, &time);
        if (ret < 0) {
            std::cout << "Select over!" << std::endl;
            break;
        }

        if (FD_ISSET(serverSocket, &fd_read)) {
            FD_CLR(serverSocket, &fd_read);
            // 接受客户端
            sockaddr_in clientAddr = {};
            socklen_t clientAddrLen = sizeof(clientAddr);
            int clientSocket = accept(serverSocket, (sockaddr *) &clientAddr, &clientAddrLen);
            if (clientSocket == -1) {
                std::cerr << "Accepting socket failed!" << std::endl;
            } else {
                for (int i = static_cast<int>(clients.size()) - 1; i >= 0; --i) {
                    NewUserJoin new_user_join;
                    new_user_join.socket = clientSocket;
                    send(clients.at(i), (const char *) &new_user_join, sizeof(NewUserJoin), 0);
                }
                clients.emplace_back(clientSocket);
                std::cout << "New client：socket = " << clientSocket << ", IP = "
                          << inet_ntoa(clientAddr.sin_addr) << std::endl;
            }
        }

        for (int i = static_cast<int>(clients.size()) - 1; i >= 0; --i) {
            if (FD_ISSET(clients.at(i), &fd_read)) {
                if (-1 == processor(clients.at(i))) {
                    auto iter = clients.begin() + i;
                    if (iter != clients.end()) {
                        clients.erase(iter);
                    }
                }
            }
        }
    }

    for (int i = static_cast<int>(clients.size()) - 1; i >= 0; --i) {
        close(clients.at(i));
    }

    // 关闭服务器 socket
    close(serverSocket);
    return 0;
}