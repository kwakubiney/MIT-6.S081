#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "user/user.h"

void
find(char *path, char *target)
{
  /*
   * Search one directory for target. Recursively search each child directory.
   */
  char buf[512];
  char *p;
  int fd;
  struct dirent de;
  struct stat st;

  /*
   * Stage 1: Open the current path and confirm that it is a directory.
   */
  if ((fd = open(path, 0)) < 0) {
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }

  if (fstat(fd, &st) < 0) {
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    return;
  }

  if (st.type != T_DIR) {
    close(fd);
    return;
  }

  /*
   * Stage 2: Prepare a reusable buffer with "path/" at the start.
   * The child name will be copied to the location stored in p.
   */
  if (strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf)) {
    fprintf(2, "find: path too long\n");
    close(fd);
    return;
  }

  strcpy(buf, path);
  p = buf + strlen(buf);
  *p++ = '/';

  /*
   * Stage 3: Read and process one directory entry at a time.
   */
  while (read(fd, &de, sizeof(de)) == sizeof(de)) {
    /*
     * An inode number of zero identifies an unused directory entry.
     */
    if (de.inum == 0) {
      continue;
    }

    memmove(p, de.name, DIRSIZ);
    p[DIRSIZ] = 0;

    /*
     * Stage 4: Skip the current and parent directory entries.
     * Recursing into these entries would create a cycle.
     */
    if (strcmp(p, ".") == 0 || strcmp(p, "..") == 0) {
      continue;
    }

    if (stat(buf, &st) < 0) {
      fprintf(2, "find: cannot stat %s\n", buf);
      continue;
    }

    /*
     * Stage 5: Print a matching path. Recurse if the entry is a directory.
     */
    if (strcmp(p, target) == 0) {
      printf("%s\n", buf);
    }

    if (st.type == T_DIR) {
      find(buf, target);
    }
  }

  /*
   * This directory has no more entries. Release its file descriptor.
   */
  close(fd);
}

int
main(int argc, char *argv[])
{
  /*
   * The command requires a starting directory and a target name.
   */
  if (argc != 3) {
    fprintf(2, "usage: find path name\n");
    exit(1);
  }

  find(argv[1], argv[2]);
  exit(0);
}
