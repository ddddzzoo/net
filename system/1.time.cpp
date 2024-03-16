#include <sys/time.h>
#include <unistd.h>

#include <ctime>
#include <iostream>
#include <string>
using namespace std;

int main() {
  // time_t 类型实际上就是long int
  // 表示从1970/1/1的0时0分0秒到现在的秒数
  time_t now1 = time(nullptr);
  long now2;
  time(&now2);

  cout << "now1: " << now1 << endl;
  cout << "now2: " << now2 << endl;

  time_t now = time(nullptr);
  cout << "now: " << now << endl;

  // tm更符合使用习惯的时间结构体
  tm tm_now;
  // 将time_t表示的时间转化为tm结构体表示的时间
  // localtime非线程安全 localtime_r线程安全
  localtime_r(&now, &tm_now);

  string str_time =
      to_string(tm_now.tm_year + 1900) + "-" + to_string(tm_now.tm_mon + 1) +
      "-" + to_string(tm_now.tm_mday) + " " + to_string(tm_now.tm_hour) + ":" +
      to_string(tm_now.tm_min) + ":" + to_string(tm_now.tm_sec);

  cout << "str_time: " << str_time << endl;

  // 将tm结构体时间转化为time_t时间
  time_t mk_time = mktime(&tm_now);

  cout << "mk_time: " << mk_time << endl;

  // 将字符串时间转化为time_t
  tm from_str;
  if (strptime(str_time.c_str(), "%Y-%m-%d %H:%M:%S", &from_str) != nullptr) {
    time_t t_time = mktime(&from_str);
    cout << "time_t value: " << t_time << endl;
  }
  else {
    cerr << "Failed to parse time string" << endl;
  }

  // 获取1970年1月1日到现在的秒和当前秒中已逝去的微秒数
  timeval currentTime;
  gettimeofday(&currentTime, nullptr);
  cout << "Seconds: " << currentTime.tv_sec << endl;
  cout << "Microseconds: " << currentTime.tv_usec << endl;

  // 程序睡眠1s
  sleep(1);
  return 0;
}
