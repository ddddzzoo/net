#ifndef NET_LEARN_TIMER_H
#define NET_LEARN_TIMER_H

#include <chrono>

class Timer {
 public:
  Timer() { update(); }

  // 更新时间
  void update() { _timestamp = std::chrono::high_resolution_clock::now(); }

  // 获取当前秒
  double GetElapsedSeconds() {
    return this->GetElapsedMicroseconds() * 0.000001;
  }

  // 获取当前毫秒
  double GetElapsedMilliseconds() {
    return this->GetElapsedMicroseconds() * 0.001;
  }
  // 获取当前微秒
  long long GetElapsedMicroseconds() {
    return std::chrono::duration_cast<std::chrono::microseconds>(
               std::chrono::high_resolution_clock::now() - _timestamp)
        .count();
  }

 private:
  std::chrono::time_point<std::chrono::high_resolution_clock> _timestamp;
};

#endif  // NET_LEARN_TIMER_H