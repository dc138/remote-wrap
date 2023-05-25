#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
  if (argc != 3) {
    printf("error: use %s IN OUT\n", argv[0]);
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
