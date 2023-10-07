#include <iostream>
#include <string>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

const int PORT = 12345;
const int BACKLOG = 5; // 等待队列的最大长度 超过会被服务器拒绝连接

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

    while (true) {
        // 接受客户端
        sockaddr_in clientAddr = {};
        socklen_t clientAddrLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (sockaddr *) &clientAddr, &clientAddrLen);
        if (clientSocket == -1) {
            std::cerr << "Accepting socket failed!" << std::endl;
            close(serverSocket);
            return 1;
        }
        // 向客户端发送消息
        std::string welcomeMsg = "Welcome to the server!";
        send(clientSocket, welcomeMsg.c_str(), welcomeMsg.size(), 0);
        // 关闭客户端连接
        close(clientSocket);
    }

    // 关闭服务器 socket
    close(serverSocket);

    return 0;
}