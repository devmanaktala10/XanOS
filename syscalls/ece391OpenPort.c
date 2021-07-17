#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"

#define COUNT_LIM 3

int main() {
  int32_t fd;

  if (-1 == (fd = ece391_open("/dev/udp/22"))) {
      ece391_fdputs (1, (uint8_t*)"could not open port\n");
      return 2;
  }
  char buf[16 * 1024];
  int32_t bytes = 0;
  int count = 0;
  while(1) {

    bytes = ece391_read(fd, buf, 0);
    if (-1 != bytes) {
      ece391_write(fd, buf, bytes);
      count++;
      if (count == COUNT_LIM) {
        break;
      }
    }
  }

  ece391_close(fd);
}
