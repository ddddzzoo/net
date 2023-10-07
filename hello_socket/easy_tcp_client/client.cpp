#include <iostream>
#include <string>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

const char *SERVER_IP = "127.0.0.1";
const int SERVER_PORT = 12345;

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

    // 发送消息给服务器
    std::string message = "Hello, server!";
    send(clientSocket, message.c_str(), message.size(), 0);

    // 接受服务器消息
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    recv(clientSocket, buffer, sizeof(buffer), 0);
    std::cout << "Server response: " << buffer << std::endl;

    // 关闭客户端 socket
    close(clientSocket);

    return 0;
}