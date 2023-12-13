#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <utime.h>

#include <cstdio>
#include <cstring>

#define SIZE 128

// 实现文件拷贝保留文件属性
int main() {
  // 1. 以只读的方式打开文件
  int fd_read = open("txt", O_RDONLY);

  if (-1 == fd_read) {
    perror("open");
    return 1;
  }

  // 2. 以只写的方式打开文件
  int fd_write = open("txt_copy", O_WRONLY | O_TRUNC | O_CREAT, 0644);

  if (-1 == fd_write) {
    perror("open");
    return 1;
  }

  // 3. 循环拷贝数据
  char buf[SIZE];
  while (true) {
    memset(buf, 0, SIZE);
    // 从第一个文件中读取数据
    auto ret = read(fd_read, buf, SIZE);
    if (ret <= 0) {
      break;
    }
    // 将数据写入第二个文件
    ret = write(fd_write, buf, ret);
    if (ret <= 0) {
      break;
    }
  }

  // 4. 关闭文件
  close(fd_read);
  close(fd_write);

  // 5. 获取源文件属性
  struct stat src_stat {};
  stat("txt", &src_stat);

  // 修改目标文件时间
  struct utimbuf tim_buf {};
  tim_buf.actime = src_stat.st_atimespec.tv_sec;
  tim_buf.modtime = src_stat.st_mtimespec.tv_sec;
  utime("txt_copy", &tim_buf);

  // 修改权限
  chmod("txt_copy", src_stat.st_mode);

  // 修改所有者
  if (chown("txt_copy", src_stat.st_uid, src_stat.st_gid) < 0) {
    perror("chown");
  }

  return 0;
}