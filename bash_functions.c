#include "bash.h"

void open_pipe(int* pipefd)
{
    if(pipe(pipefd) == -1)
    {
        perror("error in opening pipe");
        exit(1);
    }
}


int safe_open(const char* path, int flags, int mode)
{   
    int fd = open(path, flags, mode); 

    if (fd < 0)
    {
        perror("error in open sys_call");
        exit(1);
    }

    return fd;
}


void safe_close(int fd)
{
    if (close(fd) < 0)
    {
        perror("error in close sys_call");
        exit(1);
    }
}

ssize_t my_read(int fd, char* buffer, size_t size) {

    ssize_t n = 1, read_bytes = 0;

    while (n > 0) {   
        n = read(fd, buffer + read_bytes, size - read_bytes); 
        if (n < 0)
        {
            perror("error in reading file\n");
            exit(1);
        }

        read_bytes += n;

        if (fd == 0 && *(buffer + read_bytes - 1) == '\n')
            return read_bytes;
    }

    return read_bytes;
}

int my_write(int fd, const char* buffer, size_t size) {
    size_t n = 0;
    size_t written = 0;
    while (size - written != 0) {
        n = write(fd, buffer + written, size - written);
        written += n;
        
        if (n < 0) {
            perror("erorr in writing");
            exit(0);
        }
    }

    return written;
}
