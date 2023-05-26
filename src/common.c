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

void crypto_fd(int in_fd, int out_fd, bool encrypt) {
  long in_size = lseek(in_fd, 0, SEEK_END);
  PERROR_IF(in_size == -1, "[!] Cannot lseek input file");
  PERROR_IF(lseek(in_fd, 0, SEEK_SET) == -1, "[!] Cannot lseek input file");

  void* in_data = mmap(NULL, in_size, PROT_READ, MAP_SHARED, in_fd, 0);
  PERROR_IF(in_data == MAP_FAILED, "[!] Cannot mmap input memory file");

  // TODO: allocate less extra space
  int const initial_out_size = in_size * 2;

  PERROR_IF(ftruncate(out_fd, initial_out_size) == -1, "[!] Cannot ftruncate output file");

  void* out_data = mmap(NULL, initial_out_size, PROT_WRITE | PROT_READ, MAP_SHARED, out_fd, 0);
  PERROR_IF(out_data == MAP_FAILED, "[!] Cannot mmap output memory file");

  EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
  ERROR_IF(ctx == NULL, "[!] Cannot allocate EVP pointer: %s", ERR_error_string(ERR_get_error(), NULL));

  ERROR_IF(EVP_CipherInit_ex2(ctx, EVP_aes_256_cbc(), NULL, NULL, encrypt, NULL) == 0,
           "[!] Cannot init EVP instance: %s",
           ERR_error_string(ERR_get_error(), NULL));

  ERROR_IF(EVP_CIPHER_CTX_get_key_length(ctx) != AES_256_KEY_SIZE, "[!] Key sizes do not match");
  ERROR_IF(EVP_CIPHER_CTX_get_iv_length(ctx) != AES_BLOCK_SIZE, "[!] Block sizes do not match");

  ERROR_IF(EVP_CipherInit_ex2(ctx, NULL, crypto_key, crypto_iv, encrypt, NULL) == 0, "[!] Cannot set cipher values");

  int partial_out_size = 0, out_size = 0;

  ERROR_IF(EVP_CipherUpdate(ctx, out_data, &partial_out_size, in_data, in_size) == 0,
           "[!] Cannot run cipher update: %s",
           ERR_error_string(ERR_get_error(), NULL));

  ERROR_IF(EVP_CipherFinal_ex(ctx, out_data, &out_size) == 0,
           "[!] Cannot run cipher final: %s",
           ERR_error_string(ERR_get_error(), NULL));

  out_size += partial_out_size;

  EVP_CIPHER_CTX_cleanup(ctx);

  PERROR_IF(munmap(out_data, initial_out_size) == -1, "[!] Cannot unmap memory region");
  PERROR_IF(munmap(in_data, in_size) == -1, "[!] Cannot unmap memory region");

  PERROR_IF(ftruncate(out_fd, out_size) == -1, "[!] Cannot ftruncate output file");
}
