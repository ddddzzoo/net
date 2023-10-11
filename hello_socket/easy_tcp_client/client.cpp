#include <iostream>
#include <string>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

const char *SERVER_IP = "127.0.0.1";
const int SERVER_PORT = 12345;

// 使用结构体模拟用户登陆请求
// 头部
enum CMD {
    CMD_LOGIN,
    CMD_LOGOUT,
    CMD_ERROR
};

struct DataHeader {
    short data_length; // 数据长度
    short cmd;  // 数据作用
};

// 数据部分
struct Login {
    std::string username;
    std::string password;
};

struct LoginResult {
    int result;

};

struct Logout {
    std::string username;
};

struct LogoutResult {
    int result;

};

int main() {
    // 创建客户端 socket
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        std::cerr << "Creating socket failed!" << std::endl;
        return 1;
    }

    // 配置服务器地址
    sockaddr_in serverAddr = {};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &(serverAddr.sin_addr));

    // 连接服务器
    if (connect(clientSocket, (sockaddr *) &serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "Connecting socket failed!" << std::endl;
        close(clientSocket);
        return 1;
    }

    while (true) {
        std::string cmd;
        std::cin >> cmd;
        if ("exit" == cmd) {
            break;
        } else if ("login" == cmd) {
            Login login = {"username", "password"};
            DataHeader header = {sizeof(login), CMD_LOGIN};
            // 发送消息给服务器
            send(clientSocket, (const char *) &header, sizeof(header), 0);
            send(clientSocket, (const char *) &login, sizeof(login), 0);
            // 接受服务器消息
            DataHeader ret_header = {};
            LoginResult ret = {};
            recv(clientSocket, (char *) &ret_header, sizeof(ret_header), 0);
            recv(clientSocket, (char *) &ret, sizeof(ret), 0);
            std::cout << "Login result: " << ret.result << std::endl;
        } else if ("logout" == cmd) {
            Logout logout = {"username"};
            DataHeader header = {sizeof(logout), CMD_LOGOUT};
            // 发送消息给服务器
            send(clientSocket, (const char *) &header, sizeof(header), 0);
            send(clientSocket, (const char *) &logout, sizeof(logout), 0);
            // 接受服务器消息
            DataHeader ret_header = {};
            LogoutResult ret = {};
            recv(clientSocket, (char *) &ret_header, sizeof(ret_header), 0);
            recv(clientSocket, (char *) &ret, sizeof(ret), 0);
            std::cout << "Logout result: " << ret.result << std::endl;
        } else {
            std::cout << "Enter exit for exit!" << std::endl;
        }
    }

    // 关闭客户端 socket
    close(clientSocket);

    return 0;
}