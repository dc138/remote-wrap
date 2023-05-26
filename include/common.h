#ifndef XMRIG_WRAP_COMMON_H
#define XMRIG_WRAP_COMMON_H

#include <stdbool.h>

#include <openssl/evp.h>

char* fd_to_path(int fd);

int create_mem_fd(void);

void crypto_fd(int in_fd, int out_fd, bool encrypt);

#endif
