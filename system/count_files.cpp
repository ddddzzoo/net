#include <dirent.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

// 统计出指定目录中普通文件的个数
int get_file_num(char *root) {
  int total = 0;
  // 打开目录
  DIR *dir = opendir(root);
  // 循环从目录中读文件
  char path[1024];

  struct dirent *ptr = readdir(dir);

  while (ptr != nullptr) {
    // 跳过 . 和 ..
    if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0) {
      continue;
    }
    // 判断是不是目录
    if (ptr->d_type == DT_DIR) {
      snprintf(path, sizeof(path), "%s/%s", root, ptr->d_name);
      // 递归读目录
      total += get_file_num(path);
    }
    // 普通文件
    if (ptr->d_type == DT_REG) {
      total++;
    }
  }
  closedir(dir);
  return total;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("./a.out path");
    exit(1);
  }

  int total = get_file_num(argv[1]);
  printf("%s has file number:%d\n", argv[1], total);

  return 0;
}