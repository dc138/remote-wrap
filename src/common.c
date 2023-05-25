#define _GNU_SOURCE
#include <sys/mman.h>

#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "errors.h"

#define AES_256_KEY_SIZE 32
#define AES_BLOCK_SIZE 16

static uint8_t crypto_key[AES_256_KEY_SIZE] = {0};
static uint8_t crypto_iv[AES_BLOCK_SIZE]    = {0};

char* fd_to_path(int fd) {
  char* ret = NULL;
  ERROR_IF(asprintf(&ret, "/proc/self/fd/%d", fd) == -1, "[!] Cannot allocate string");
  return ret;
}

int create_mem_fd(void) {
  int out_fd = memfd_create("download", 0);
  PERROR_IF(out_fd == -1, "[!] Cannot create memory file");
  return out_fd;
}

int crypto_fd(int in_fd, EVP_CIPHER const* cipher, bool encrypt) {
  int out_fd = memfd_create("out", 0);

  long in_size = lseek(in_fd, 0, SEEK_END);
  PERROR_IF(in_size == -1, "[!] Cannot lseek input file");
  PERROR_IF(lseek(in_fd, 0, SEEK_SET) == -1, "[!] Cannot lseek input file");

  PERROR_IF(ftruncate(out_fd, in_size) == -1, "[!] Cannot ftruncate output file");

  void* out_data = mmap(NULL, in_size, PROT_READ | PROT_WRITE, MAP_SHARED, out_fd, 0);
  PERROR_IF(out_data == MAP_FAILED, "[!] Cannot mmap output memory file");

  return out_fd;
}
