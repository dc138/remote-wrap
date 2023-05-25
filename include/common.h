#ifndef XMRIG_WRAP_COMMON_H
#define XMRIG_WRAP_COMMON_H

#include <stdbool.h>

#include <openssl/evp.h>

char* fd_to_path(int fd);

int create_mem_fd(void);

int crypto_fd(int in_fd, bool encrypt);

#endif
