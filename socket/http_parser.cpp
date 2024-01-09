#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cassert>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#define BUFFER_SIZE 4096  // 读缓冲区大小

// 主状态机：当前正在分析请求行，当前正在分析头部字段
enum CHECK_STATE { CHECK_STATE_REQUESTLINE = 0, CHECK_STATE_HEADER };

// 从状态机：读取完整行，行出错，行数据不完整
enum LINE_STATUS { LINE_OK = 0, LINE_BAD, LINE_OPEN };

// 服务器处理HTTP请求结果
enum HTTP_CODE {
  NO_REQUEST,         // 请求不完整，需要继续读取客户数据
  GET_REQUEST,        // 完整客户请求
  BAD_REQUEST,        // 客户请求语法错误
  FORBIDDEN_REQUEST,  // 客户对资源没有足够访问权
  INTERNAL_ERROR,     // 服务器内部错误
  CLOSED_CONNECTION   // 客户端连接已关闭
};

// 根据服务器处理结果回应以下数据
static const char* sz_ret[] = {"I get a correct result\n", "Something wrong\n"};

// 从状态机，用于解析出一行内容
LINE_STATUS parse_line(char* buffer, int& checked_index, int& read_index) {
  char temp;
  for (; checked_index < read_index; ++checked_index) {
    // 获取当前要分析的字节
    temp = buffer[checked_index];
    // 当前字节是\r
    if (temp == '\r') {
      // \r是buffer中的最后一个字符
      if (checked_index + 1 == read_index) {
        return LINE_OPEN;
      }
      else if (buffer[checked_index + 1] == '\n')  // 下一个字符是\n
      {
        buffer[checked_index++] = '\0';
        buffer[checked_index++] = '\0';
        return LINE_OK;
      }
      // 用户发送的HTTP请求存在语法问题
      return LINE_BAD;
    }
    else if (temp == '\n') {
      if ((checked_index > 1) && (buffer[checked_index - 1] == '\r')) {
        buffer[checked_index++] = '\0';
        buffer[checked_index++] = '\0';
        return LINE_OK;
      }
      return LINE_BAD;
    }
  }

  // 所有内容都没有\r
  return LINE_OPEN;
}

// 分析请求行
HTTP_CODE parse_request_line(char* temp, CHECK_STATE& check_state) {
  char* url = strpbrk(temp, " \t");
  // 请求行中没有空白字符或者\t则请求存在问题
  if (!url) {
    return BAD_REQUEST;
  }

  *url++ = '\0';
  char* method = temp;
  // 仅支持GET方法
  if (strcasecmp(method, "GET") == 0) {
    printf("The request method is GET\n");
  }
  else {
    return BAD_REQUEST;
  }
  url += strspn(url, "\t");
  char* version = strpbrk(url, "\t");
  if (!version) {
    return BAD_REQUEST;
  }
  *version++ = '\0';
  version += strspn(version, "\t");
  // 仅支持HTTP/1.1
  if (strcasecmp(version, "HTTP/1.1") != 0) {
    return BAD_REQUEST;
  }
  // 检查URL是否合法
  if (strncasecmp(url, "http://", 7) == 0) {
    url += 7;
    url = strchr(url, '/');
  }
  if (!url || url[0] != '/') {
    return BAD_REQUEST;
  }
  printf("The request URL is: %s\n", url);

  check_state = CHECK_STATE_HEADER;
  return NO_REQUEST;
}

// 分析头部字段
HTTP_CODE parse_header(char* temp) {
  if (temp[0] == '\0') {
    return GET_REQUEST;
  }
  else if (strncasecmp(temp, "HOST:", 5) == 0) {
    temp += 5;
    temp += strspn(temp, "\t");
    printf("the request host is: %s\n", temp);
  }
  else {
    printf("I can not handdle this header\n");
  }
  return NO_REQUEST;
}

HTTP_CODE parse_content(char* buffer, int& check_index,
                        CHECK_STATE& check_state, int& read_index,
                        int& start_line) {
  LINE_STATUS line_status = LINE_OK;
  HTTP_CODE ret_code = NO_REQUEST;
  while ((line_status = parse_line(buffer, check_index, read_index)) ==
         LINE_OK) {
    char* temp = buffer + start_line;
    start_line = check_index;
    switch (check_state) {
      case CHECK_STATE_REQUESTLINE: {
        ret_code = parse_request_line(temp, check_state);
        if (ret_code = BAD_REQUEST) {
          return BAD_REQUEST;
        }
        break;
      }

      case CHECK_STATE_HEADER: {
        ret_code = parse_header(temp);
        if (ret_code == BAD_REQUEST) {
          return BAD_REQUEST;
        }
        else if (ret_code == GET_REQUEST) {
          return GET_REQUEST;
        }
        break;
      }

      default: {
        return INTERNAL_ERROR;
      };
    }
  }
  if (line_status == LINE_OPEN) {
    return NO_REQUEST;
  }
  else {
    return BAD_REQUEST;
  }
}

int main(int argc, char* argv[]) {
  if (argc <= 2) {
    printf("usage: %s ip_address port_number\n", basename(argv[0]));
    return 1;
  }
  const char* ip = argv[1];
  int port = atoi(argv[2]);

  struct sockaddr_in address;
  bzero(&address, sizeof(address));
  address.sin_family = AF_INET;
  inet_pton(AF_INET, ip, &address.sin_addr);
  address.sin_port = htons(port);

  int sock = socket(AF_INET, SOCK_STREAM, 0);
  assert(sock >= 0);

  int ret = bind(sock, (struct sockaddr*)&address, sizeof(address));
  assert(ret != -1);

  ret = listen(sock, 5);
  assert(ret != -1);

  struct sockaddr_in client_address;
  socklen_t client_addr_len = sizeof(client_address);
  int fd = accept(sock, (struct sockaddr*)&client_address, &client_addr_len);
  if (fd < 0) {
    printf("errno is:%d\n", errno);
  }
  else {
    char buffer[BUFFER_SIZE];
    memset(buffer, '\0', BUFFER_SIZE);
    int data_read = 0;
    int read_index = 0;
    int checked_index = 0;
    int start_line = 0;
    CHECK_STATE checkstate = CHECK_STATE_REQUESTLINE;
    while (1) {
      data_read = recv(fd, buffer + read_index, BUFFER_SIZE - read_index, 0);
      if (data_read == -1) {
        printf("reading failed\n");
        break;
      }
      else if (data_read == 0) {
        printf("remote client has closed the connection\n");
        break;
      }

      read_index += data_read;
      HTTP_CODE result = parse_content(buffer, checked_index, checkstate,
                                       read_index, start_line);
      if (result == NO_REQUEST) {
        continue;
      }
      else if (result == GET_REQUEST) {
        send(fd, sz_ret[0], strlen(sz_ret[0]), 0);
        break;
      }
      else {
        send(fd, sz_ret[1], strlen(sz_ret[1]), 0);
        break;
      }
    }
    close(fd);
  }

  close(sock);
  return 0;
}