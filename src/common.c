#define _GNU_SOURCE
#include <sys/mman.h>

#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "errors.h"

char* fd_to_path(int fd) {
  char* ret = NULL;
  EXPECT_NOT(asprintf(&ret, "/proc/self/fd/%d", fd), -1, "[!] Cannot allocate string");
  return ret;
}

int create_mem_fd(void) {
  int out_fd = memfd_create("prog", 0);
  EXPECT_NOT(out_fd, -1, "[!] Cannot create memory file");
  return out_fd;
}

int crypto_fd(int fd, EVP_CIPHER const* cipher, bool encrypt) {
  // TODO: implement
  return -1;
}
