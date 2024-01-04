#include <bit>
#include <cstdio>

// 通过 union 对大小端进行判断
int main() {
  // 定义一个联合体，包含一个 short 型成员和一个 char 数组成员
  union {
    short value;                      // 16 位的数据
    char union_bytes[sizeof(short)];  // 以字节数组的形式访问 value 的各个字节
  } test{};

  // 将 short 型数据的值设为 0x0102
  test.value = 0x0102;

  // 判断字节序
  if ((test.union_bytes[0] == 1) && (test.union_bytes[1] == 2)) {
    printf("big endian\n");  // 如果第一个字节存储的是高位字节，则为 big endian
  }
  else if ((test.union_bytes[0] == 2) && (test.union_bytes[1] == 1)) {
    printf("little endian\n");  // 如果第一个字节存储的是低位字节，则为 little
                                // endian
  }
  else {
    printf("unknown...\n");  // 无法确定字节序的情况
  }

  // 使用位运算检测字节序
  int n = 1;
  if (*((char *)&n) == 1) {  // 第一个字节存储的是低位地址
    printf("little endian\n");
  }
  else {
    printf("big endian\n");
  }

  // 使用C++20提供的 std::endian 查询
  if constexpr (std::endian::native == std::endian::big) {
    printf("big endian\n");
  }
  else if constexpr (std::endian::native == std::endian::little) {
    printf("little endian\n");
  }
  else {
    printf("unknown...\n");
  }

  return 0;
}