#ifndef XMRIG_WRAP_ERRORS_H
#define XMRIG_WRAP_ERRORS_H

#define UNUSED(x) ((void)x)

#define EXPECT(res, ok, msg) \
  do {                       \
    if ((res) != (ok)) {     \
      perror(msg);           \
      exit(-1);              \
    }                        \
  } while (0)

#define EXPECT_NOT(res, err, msg) \
  do {                            \
    if ((err) == (res)) {         \
      perror(msg);                \
      exit(-1);                   \
    }                             \
  } while (0)

#define EXPECT_CODE(res, ok, msg, code)       \
  do {                                        \
    if ((ok) != (res)) {                      \
      fprintf(stderr, "%s: %d\n", msg, code); \
      exit(-1);                               \
    }                                         \
  } while (0)

#define EXPECT_NOT_CODE(res, err, msg, code)  \
  do {                                        \
    if ((err) == (res)) {                     \
      fprintf(stderr, "%s: %d\n", msg, code); \
      exit(-1);                               \
    }                                         \
  } while (0)

#define EXPECT_NONE(res, ok, msg)   \
  do {                              \
    if ((ok) != (res)) {            \
      fprintf(stderr, "%s\n", msg); \
      exit(-1);                     \
    }                               \
  } while (0)

#define EXPECT_NOT_NONE(res, err, msg) \
  do {                                 \
    if ((err) == (res)) {              \
      fprintf(stderr, "%s\n", msg);    \
      exit(-1);                        \
    }                                  \
  } while (0)

#endif
