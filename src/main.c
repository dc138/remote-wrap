#define _GNU_SOURCE
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define EXPECT_NOT(res, err, msg) \
  do {                            \
    if ((err) == (res)) {         \
      perror(msg);                \
      abort();                    \
    }                             \
  } while (0)

int write_to_mem_file(char* filename) {
  // Open input file
  FILE* in_file = fopen(filename, "rb");
  EXPECT_NOT(in_file, NULL, "[!] Cannot open input file");

  // Get its size in bytes
  fseek(in_file, 0, SEEK_END);
  long in_size = ftell(in_file);
  fseek(in_file, 0, SEEK_SET);

  // Create an in-memory file
  int out_fd = memfd_create("prog", 0);
  EXPECT_NOT(out_fd, -1, "[!] Cannot create memory file");

  // Sets its size to the input file's size
  EXPECT_NOT(ftruncate(out_fd, in_size), -1, "[!] Cannot ftruncate output file");

  // Map the output file's data to memory
  void* out_data = mmap(NULL, in_size, PROT_READ | PROT_WRITE, MAP_SHARED, out_fd, 0);
  EXPECT_NOT(out_data, MAP_FAILED, "[!] Cannot mmap output memory file");

  // Read the input file directly into this buffer
  EXPECT_NOT(fread(out_data, in_size, 1, in_file), -1ul, "[!] Cannot read input file");

  // Close the input file
  EXPECT_NOT(fclose(in_file), EOF, "[!] Cannot close input file");

  // Unmap the region
  EXPECT_NOT(munmap(out_data, in_size), -1, "[!] Cannot unmap memory region");

  return out_fd;
}

char* fd_to_path(int fd) {
  char* ret = NULL;
  EXPECT_NOT(asprintf(&ret, "/proc/self/fd/%d", fd), -1, "[!] Cannot allocate string");
  return ret;
}

void exec_mem_file(char* path, char* argv[]) {
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
    printf("error: no file specified\n");
    return -1;
  }

  int out_fd = write_to_mem_file(argv[1]);

  char* out_path = fd_to_path(out_fd);

  char* mock_argv[] = {out_path, NULL};
  exec_mem_file(out_path, mock_argv);

  free(out_path);
  close(out_fd);

  return 0;
}
