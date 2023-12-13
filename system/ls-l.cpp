#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <sys/stat.h>

#include <cstdio>
#include <cstring>
#include <ctime>

#define SIZE 128

// 实现ls -l "txt" 命令功能
// -rw-r--r--  1 shay  staff  12 12 14 03:16 txt
int main() {
  // 1. 获取文件状态
  struct stat st {};
  if (-1 == stat("txt", &st)) {
    perror("stat");
    return -1;
  }

  // 2. 存储文件类型和访问权限
  char perm[11] = {0};
  // 判断文件类型
  switch (st.st_mode & S_IFMT) {
    case S_IFLNK:
      perm[0] = 'l';
      break;
    case S_IFDIR:
      perm[0] = 'd';
      break;
    case S_IFREG:
      perm[0] = '-';
      break;
    case S_IFBLK:
      perm[0] = 'b';
      break;
    case S_IFCHR:
      perm[0] = 'c';
      break;
    case S_IFSOCK:
      perm[0] = 's';
      break;
    case S_IFIFO:
      perm[0] = 'p';
      break;
    default:
      perm[0] = '?';
      break;
  }

  // 判断文件的访问者权限
  // 文件所有者
  perm[1] = (st.st_mode & S_IRUSR) ? 'r' : '-';
  perm[2] = (st.st_mode & S_IWUSR) ? 'w' : '-';
  perm[3] = (st.st_mode & S_IXUSR) ? 'x' : '-';

  // 文件所属组
  perm[4] = (st.st_mode & S_IRGRP) ? 'r' : '-';
  perm[5] = (st.st_mode & S_IWGRP) ? 'w' : '-';
  perm[6] = (st.st_mode & S_IXGRP) ? 'x' : '-';

  // 其他
  perm[7] = (st.st_mode & S_IROTH) ? 'r' : '-';
  perm[8] = (st.st_mode & S_IWOTH) ? 'w' : '-';
  perm[9] = (st.st_mode & S_IXOTH) ? 'x' : '-';

  // 硬链接计数
  int link_num = st.st_nlink;
  // 文件所有者
  char* file_user = getpwuid(st.st_uid)->pw_name;
  // 文件所属组
  char* file_grp = getgrgid(st.st_gid)->gr_name;
  // 文件大小
  int file_size = static_cast<int>(st.st_size);
  // 修改时间
  char* time = ctime(&st.st_mtimespec.tv_sec);
  char mtime[512] = {0};
  strncpy(mtime, time, strlen(time) - 1);

  char buf[1024]{};
  snprintf(buf, sizeof(buf), "%s  %d  %s  %s  %d  %s  \"txt\"", perm, link_num,
           file_user, file_grp, file_size, mtime);

  printf("%s\n", buf);
  return 0;
}