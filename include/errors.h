#ifndef XMRIG_WRAP_ERRORS_H
#define XMRIG_WRAP_ERRORS_H

#define UNUSED(x) ((void)x)

#define ERROR_IF(condition, ...)    \
  do {                              \
    if (condition) {                \
      fprintf(stderr, __VA_ARGS__); \
      fputc('\n', stderr);          \
      exit(-1);                     \
    }                               \
  } while (0)

#define PERROR_IF(condition, msg) \
  do {                            \
    if (condition) {              \
      perror(msg);                \
      exit(-1);                   \
    }                             \
  } while (0)

#endif
