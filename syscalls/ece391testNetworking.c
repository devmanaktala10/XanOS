#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"

#define COUNT_LIM 10

int main() {

  int fd;
  if (-1 == (fd = ece391_open((uint8_t*) "/dev/udp/22"))) {
      ece391_fdputs (1, (uint8_t*) "Could not open port\n");
      return 2;
  }

  if (-1 == ioctl(fd, 0x1, (unsigned long) "23")) {
    ece391_fdputs(1, (uint8_t*)"Could not change sender port");
  }

  int count = 0;
  int bytes = 0;
  char buf[16 * 1024];

  while (count < COUNT_LIM) {
    bytes = ece391_read(fd, buf, 0);
    ece391_write(fd, buf, bytes);
    count++;
  }

  return 0;

}
