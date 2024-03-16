#include <dirent.h>
#include <unistd.h>

#include <iostream>
using namespace std;

int main() {
  char path1[256];
  // 获取当前工作目录的路径名
  getcwd(path1, sizeof(path1));
  cout << "path1: " << path1 << endl;

  char *path2 = get_current_dir_name();
  cout << "path2: " << path2 << endl;
  free(path2);  // 注意释放内存

  DIR *dir;  // 定义目录指针

  // 打开目录
  if ((dir = opendir(path1)) == nullptr) {
    return -1;
  }

  struct dirent *stdinfo = nullptr;

  while (true) {
    // 读取一项内容并显示出来
    if ((stdinfo = readdir(dir)) == nullptr) {
      break;
    }
    cout << "文件名：" << stdinfo->d_name << "，文件类型："
         << (int)stdinfo->d_type << endl;
  }

  closedir(dir);  // 关闭目录指针

  return 0;
}