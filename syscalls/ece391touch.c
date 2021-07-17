#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"

int main ()
{
    int32_t fd, cnt;
    uint8_t buf[1024];

    if (0 != ece391_getargs (buf, 1024)) {
        ece391_fdputs (1, (uint8_t*)"could not read arguments\n");
	return 3;
    }


    if(0 != create(buf)){
        ece391_fdputs(1, (uint8_t*)"could not create file\n");
        return 2;
    }
    
    return 0;
}

