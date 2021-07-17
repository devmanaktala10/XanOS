#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"

#define MAX_FILESIZE  (512 * 1024)
#define BUFSIZE 128

int main() {

  int file_fd;
  char buf[BUFSIZE];
  int protocol = 2;

  ece391_fdputs(1, (uint8_t*)"Client: 0, Host: 1\n");
  if (-1 == ece391_read(0, buf, BUFSIZE-1)) {
      ece391_fdputs(1, (uint8_t*) "Can't read the number from keyboard.\n");
   return 3;
  }

  if (buf[0] == '0') {
    protocol = 0;
  } else if (buf[0] == '1') {
    protocol = 1;
  } else {
    ece391_fdputs(1, (uint8_t*) "Invalid Input\n");
    return 0;
  }

  if (protocol == 0) {
    int32_t bytes;
    int32_t udp_fd;

    ece391_fdputs(1, (uint8_t*) "Enter file name to write to...\n");
    bytes = ece391_read(0, buf, BUFSIZE-1);
    if (-1 == bytes) {
        ece391_fdputs(1, (uint8_t*) "Can't read the file name.\n");
     return 3;
    }

    if (buf[bytes - 1] == '\n') {
      buf[bytes - 1] = '\0';
    }

    if (-1 == (file_fd = ece391_open((uint8_t*) buf))) {
      ece391_fdputs(1, (uint8_t*) "Unable to Open File\n");
      ece391_fdputs(1, (uint8_t*) "Make sure the file is created\n");
      return 0;
    }

    ece391_fdputs(1, (uint8_t*) "Enter udp port to read from...\n");
    bytes == ece391_read(0, buf, BUFSIZE-1);
    if (-1 == bytes) {
        ece391_fdputs(1, (uint8_t*) "Can't read the udp path.\n");
     return 3;
    }

    if (buf[bytes - 1] == '\n') {
      buf[bytes - 1] = '\0';
    }

    if (-1 == (udp_fd = ece391_open((uint8_t*) buf))) {
        ece391_fdputs (1, (uint8_t*)"Could not open port\n");
        return 2;
    }

    char data[MAX_FILESIZE];
    bytes = ece391_read(udp_fd, data, 0);
    ece391_write(file_fd, data, bytes);

    ece391_fdputs(1, (uint8_t*) "Data successfully written to file\n");


  } else if (protocol == 1) {
    int32_t bytes;
    int32_t udp_fd;

    ece391_fdputs(1, (uint8_t*) "Enter file name to read from...\n");
    bytes = ece391_read(0, buf, BUFSIZE-1);
    if (-1 == bytes) {
        ece391_fdputs(1, (uint8_t*) "Can't read the file name.\n");
     return 3;
    }

    if (buf[bytes - 1] == '\n') {
      buf[bytes - 1] = '\0';
    }

    if (-1 == (file_fd = ece391_open((uint8_t*) buf))) {
      ece391_fdputs(1, (uint8_t*) "Unable to Open File\n");
      return 0;
    }

    ece391_fdputs(1, (uint8_t*) "Enter udp port to write to...\n");
    bytes = ece391_read(0, buf, BUFSIZE-1);
    if (-1 == bytes) {
        ece391_fdputs(1, (uint8_t*) "Can't read the udp path.\n");
     return 3;
    }

    if (buf[bytes - 1] == '\n') {
      buf[bytes - 1] = '\0';
    }

    if (-1 == (udp_fd = ece391_open((uint8_t*) buf))) {
        ece391_fdputs (1, (uint8_t*)"Could not open port\n");
        return 2;
    }

    char udp_sender_port[6];
    ece391_fdputs(1, (uint8_t*) "Enter udp sender port...\n");
    bytes = ece391_read(0, udp_sender_port, 5);
    if (bytes == -1) {
        ece391_fdputs(1, (uint8_t*) "Can't read the udp path.\n");
     return 3;
    }

    udp_sender_port[bytes] = '\0';
    udp_sender_port[bytes - 1] = '\0';

    if (-1 == ioctl(udp_fd, 0x1, (unsigned long) udp_sender_port)) {
      ece391_fdputs(1, (uint8_t*)"Could not change sender port\n");
      return 0;
    }

    char data[MAX_FILESIZE];
    bytes = ece391_read(file_fd, data, MAX_FILESIZE);
    ece391_write(udp_fd, data, bytes);

    ece391_fdputs(1, (uint8_t*) "Data successfully sent from file\n");

  }


  return 0;
}
