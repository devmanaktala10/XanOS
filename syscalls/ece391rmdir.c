#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"

int main ()
{
    uint8_t buf[1024];

    if (0 != ece391_getargs (buf, 1024)) {
        ece391_fdputs (1, (uint8_t*)"could not read arguments\n");
	return 3;
    }

    if(-1 == rmdir(buf)){
        ece391_fdputs (1, (uint8_t*)"Fatal error: directory may be curropted");
        return 2;
    }

    return 0;
}

