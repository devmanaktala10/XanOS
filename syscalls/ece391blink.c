#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"

int main ()
{
    int32_t fd, cnt;
    uint8_t buf[1024];

    int rtc_fd = ece391_open((uint8_t*)"rtc");

    uint8_t * vmem_base_addr;

    if(ece391_vidmap(&vmem_base_addr) == -1) {
        return -1;
    }

    int ret_val, garbage;

    ret_val = 32;
    ret_val = ece391_write(rtc_fd, &ret_val, 4);

    int i = 0;
    int val = 0xFF;

    while(1){
        
        int j;
        for(j = 0; j < 80; j++){
            *(vmem_base_addr + i) = val;
            i++;
            if(i == 80*25*16){
                i = 0;
                val = ~val;
            }
        }
        
        ece391_read(rtc_fd, &garbage, 4);

    }

    return 0;
}

