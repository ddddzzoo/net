#include <arpa/inet.h>
#include <libgen.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cassert>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

// 全局变量，用于标志是否接收到终止信号
static bool stop = false;
// 处理终止信号函数
static void handle_term(int sig) { stop = true; }

// 处于ESTABLISHED状态的连接只有backlog值加1个，其他的连接都处于SYN_RCVD状态
int main(int argc, char* argv[]) {
  // 注册信号处理函数
  signal(SIGTERM, handle_term);

  // 检查参数是否足够
  if (argc <= 3) {
    printf("usage: %s ip_address port_number backlog\n", basename(argv[0]));
    return 1;
  }
  // 从命令行参数获取ip地址、端口和监听队列长度
  const char* ip = argv[1];
  int port = std::stoi(argv[2]);
  int backlog = std::stoi(argv[3]);

  // 创建套接字
  int sock = socket(PF_INET, SOCK_STREAM, 0);
  assert(sock >= 0);

  // 初始化地址结构
  struct sockaddr_in address {};
  bzero(&address, sizeof(address));
  address.sin_family = AF_INET;
  inet_pton(AF_INET, ip, &address.sin_addr);
  address.sin_port = htons(port);

  // 绑定套接字到端口
  int ret = bind(sock, (struct sockaddr*)&address, sizeof(address));
  assert(ret != -1);

  // 开始监听连接请求
  ret = listen(sock, backlog);
  assert(ret != -1);

  while (!stop) {
    sleep(1);
  }

  // 关闭套接字
  close(sock);

  return 0;
}