#include <fcntl.h>
#include <unistd.h>
#include <stddef.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#ifndef BASH_HEADER
#define BASH_HEADER 

void open_pipe(int* pipefd);
int safe_open(const char* pathname, int flags, int mode);
void safe_close(int fd);
ssize_t safe_read(int fd, char* buf, size_t buf_sz);
ssize_t safe_write(int fd, const char* buf, size_t size);
char** parse_arguments(char* command);
void run_commands(char*** commands_array, int command_count, char* input_file, char* output_file);

#endif