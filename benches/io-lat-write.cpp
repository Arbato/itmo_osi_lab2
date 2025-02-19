#include <iostream>
#include <cstdlib>
#include <climits>
#include <cstdio>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <cstring>

#define BLOCK_SIZE 4096  // Define block size for I/O latency test

using namespace std;

/**
 * Allocates a buffer of BLOCK_SIZE bytes, aligned for O_DIRECT.
 */
char* allocBuffer() {
    char* buf;
    if (posix_memalign((void**)&buf, BLOCK_SIZE, BLOCK_SIZE) != 0) {
        perror("Memory alignment failed");
        exit(EXIT_FAILURE);
    }
    memset(buf, 0, BLOCK_SIZE);  // Ensure buffer is zeroed
    return buf;
}

/**
 * Main function to execute the I/O latency write benchmark.
 */
size_t io_lat_write(string filename) {
    int fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_DIRECT, 0644);
    if (fd == -1) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    struct timeval start, stop;

    char* buf = allocBuffer();  

    ssize_t written = write(fd, buf, BLOCK_SIZE);  

    if (written == -1) {
        perror("Error writing to file");
    }

    close(fd);
    free(buf);  // Free aligned buffer
    return 1;
}
