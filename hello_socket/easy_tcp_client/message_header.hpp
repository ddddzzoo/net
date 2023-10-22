#ifndef NET_LEARN_MESSAGE_HEADER_HPP
#define NET_LEARN_MESSAGE_HEADER_HPP

enum CMD {
  CMD_LOGIN,
  CMD_LOGIN_RESULT,
  CMD_LOGOUT,
  CMD_LOGOUT_RESULT,
  CMD_NEW_USER_JOIN,
  CMD_ERROR
};

struct DataHeader {
  short data_length;  // 数据长度
  short cmd;          // 数据作用
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
  LoginResult() : DataHeader() {
    data_length = sizeof(LoginResult);
    cmd = CMD_LOGIN_RESULT;
  }
};

struct Logout : public DataHeader {
  Logout() : DataHeader() {
    data_length = sizeof(Logout);
    cmd = CMD_LOGOUT;
  }

  std::string username;
};

struct LogoutResult : public DataHeader {
  LogoutResult() : DataHeader() {
    data_length = sizeof(LoginResult);
    cmd = CMD_LOGOUT_RESULT;
  }
};

struct NewUserJoin : public DataHeader {
  NewUserJoin() : DataHeader() {
    data_length = sizeof(NewUserJoin);
    cmd = CMD_NEW_USER_JOIN;
  }
};

#endif  // NET_LEARN_MESSAGE_HEADER_HPP