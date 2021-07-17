#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"

volatile int exit_var = 0;

void segfault_handler() {
  ece391_fdputs(1, (uint8_t*) "2nd Signal works\n");
}

void handler() {
  ece391_fdputs(1, (uint8_t*) "LEZ GO\n");
  exit_var = 1;
}

int main () {
  ece391_set_handler(2, handler);
  ece391_set_handler(0, segfault_handler);
  while(1) {
    if (exit_var == 1) {
      break;
    }
  }

  ece391_fdputs(1, (uint8_t*) "SIGRETURN WORKS\n");

  int x = 0;
  int y = 1/x;

  return 0;
}
