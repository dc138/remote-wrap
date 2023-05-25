#ifndef XMRIG_WRAP_COMMON_H
#define XMRIG_WRAP_COMMON_H

#include <stdbool.h>

#include <openssl/evp.h>

// 32 bytes = 256 bits key
#define AES_256_KEY_SIZE 32

// 16 bytes = 128 bits block
#define AES_BLOCK_SIZE 16

static uint8_t crypto_key[AES_256_KEY_SIZE] = {0};
static uint8_t crypto_iv[AES_BLOCK_SIZE]    = {0};

char* fd_to_path(int fd);

int create_mem_fd(void);

int crypto_fd(int fd, EVP_CIPHER const* cipher, bool encrypt);

#endif
