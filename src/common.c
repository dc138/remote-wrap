#define _GNU_SOURCE
#include <sys/mman.h>

#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>

#include "openssl/err.h"

#include "common.h"
#include "errors.h"

#define AES_256_KEY_SIZE 32
#define AES_BLOCK_SIZE 16

static uint8_t const crypto_key[AES_256_KEY_SIZE] = {0};
static uint8_t const crypto_iv[AES_BLOCK_SIZE]    = {0};

char* fd_to_path(int fd) {
  char* ret = NULL;
  ERROR_IF(asprintf(&ret, "/proc/self/fd/%d", fd) == -1, "[!] Cannot allocate string");
  return ret;
}

int create_mem_fd(void) {
  int out_fd = memfd_create("down", 0);
  PERROR_IF(out_fd == -1, "[!] Cannot create memory file");
  return out_fd;
}

int crypto_fd(int in_fd, bool encrypt) {
  int out_fd = memfd_create("out", 0);

  long in_size = lseek(in_fd, 0, SEEK_END);
  PERROR_IF(in_size == -1, "[!] Cannot lseek input file");
  PERROR_IF(lseek(in_fd, 0, SEEK_SET) == -1, "[!] Cannot lseek input file");

  void* in_data = mmap(NULL, in_size, PROT_READ | PROT_WRITE, MAP_SHARED, in_fd, 0);
  PERROR_IF(in_data == MAP_FAILED, "[!] Cannot mmap input memory file");

  // TODO: allocate less extra space
  int const initial_out_size = in_size * 2;

  PERROR_IF(ftruncate(out_fd, initial_out_size) == -1, "[!] Cannot ftruncate output file");

  void* out_data = mmap(NULL, initial_out_size, PROT_READ | PROT_WRITE, MAP_SHARED, out_fd, 0);
  PERROR_IF(out_data == MAP_FAILED, "[!] Cannot mmap output memory file");

  EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
  ERROR_IF(ctx == NULL, "[!] Cannot allocate EVP pointer: %s", ERR_error_string(ERR_get_error(), NULL));

  ERROR_IF(EVP_CipherInit_ex(ctx, EVP_aes_256_cbc(), NULL, crypto_key, crypto_iv, encrypt) != 0,
           "[!] Cannot init EVP instance: %s",
           ERR_error_string(ERR_get_error(), NULL));

  int out_size = 0;

  ERROR_IF(EVP_CipherUpdate(ctx, out_data, &out_size, in_data, in_size) != 0,
           "[!] Cannot run cipher update: %s",
           ERR_error_string(ERR_get_error(), NULL));

  ERROR_IF(EVP_CipherFinal_ex(ctx, out_data, &out_size) != 0,
           "[!] Cannot run cipher final: %s",
           ERR_error_string(ERR_get_error(), NULL));

  EVP_CIPHER_CTX_cleanup(ctx);

  PERROR_IF(munmap(out_data, initial_out_size) == -1, "[!] Cannot unmap memory region");
  PERROR_IF(munmap(in_data, in_size) == -1, "[!] Cannot unmap memory region");

  PERROR_IF(ftruncate(out_fd, out_size) == -1, "[!] Cannot ftruncate output file");

  return out_fd;
}
