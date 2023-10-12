#include <iostream>
#include <string>
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

int main() {
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

    // 接受客户端
    sockaddr_in clientAddr = {};
    socklen_t clientAddrLen = sizeof(clientAddr);
    int clientSocket = accept(serverSocket, (sockaddr *) &clientAddr, &clientAddrLen);
    if (clientSocket == -1) {
        std::cerr << "Accepting socket failed!" << std::endl;
        close(serverSocket);
        return 1;
    }

    std::cout << "New client：socket = " << clientSocket << ", IP = " << inet_ntoa(clientAddr.sin_addr) << std::endl;

    while (true) {
        DataHeader header = {};
        // 接受客户端消息
        int byte_len = recv(clientSocket, (char *) &header, sizeof(DataHeader), 0);
        if (byte_len <= 0) {
            std::cout << "Client exit!" << std::endl;
            break;
        }
        std::cout << "CMD: " << header.cmd << " ,data length: " << header.data_length << std::endl;

        switch (header.cmd) {
            case CMD_LOGIN: {
                Login login;
                recv(clientSocket, (char *) &login + sizeof(DataHeader), sizeof(Login), 0);
                // 没有判断用户密码
                LoginResult ret;
                send(clientSocket, (char *) &ret, sizeof(LoginResult), 0);
            }
                break;
            case CMD_LOGOUT: {
                Logout logout;
                recv(clientSocket, (char *) &logout, sizeof(logout), 0);
                //忽略判断用户密码是否正确的过程
                LogoutResult ret;
                send(clientSocket, (char *) &ret, sizeof(ret), 0);
            }
                break;
            default:
                header.cmd = CMD_ERROR;
                header.data_length = 0;
                send(clientSocket, (char *) &header, sizeof(header), 0);
                break;
        }
    }

    // 关闭服务器 socket
    close(serverSocket);

    return 0;
}