#include <sys/stat.h>

#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <unistd.h>

#include "common.h"
#include "errors.h"

int main(int argc, char* argv[]) {
  if (argc != 3) {
    printf("error: use %s IN OUT\n", argv[0]);
    return EXIT_FAILURE;
  }

  int in_fd = open(argv[1], O_RDONLY);
  PERROR_IF(in_fd == -1, "[!] Cannot open input for reading");

  int out_fd = open(argv[2], O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
  PERROR_IF(out_fd == -1, "[!] Cannot open output for writing");

  crypto_fd(in_fd, out_fd, true);

  close(in_fd);
  close(out_fd);

  return EXIT_SUCCESS;
}
