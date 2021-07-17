#include "ata.h"

#define LOW_8(x) ((x) & 0xFF)
#define NE1_8(x) (((x) >> 8) & 0xFF)
#define NE2_8(x) (((x) >> 16) & 0xFF)
#define NE3_4(x) (((x) >> 24) & 0x0F)

int read_sector(uint32_t lba, char * buffer){

    unsigned long flags;
    cli_and_save(flags);

    int i;
    int flag = 0;
    
    unsigned char lba_low = LOW_8(lba);
    unsigned char lba_mid = NE1_8(lba);
    unsigned char lba_high = NE2_8(lba);
    unsigned char lba_top4 = NE3_4(lba);

    // outb(0x02, 0x3F6);
    outb(0x00, IDE_BASE_PORT + ERROR_REGISTER);
    outb(0x01, IDE_BASE_PORT + SELECT_COUNT_REGISTER);
    outb(lba_low, IDE_BASE_PORT + SELECT_NUMBER_REGISTER);
    outb(lba_mid, IDE_BASE_PORT + SELECT_LOW_REGISTER);
    outb(lba_high, IDE_BASE_PORT + SELECT_HIGH_REGISTER);
    outb((0xF0 | lba_top4), IDE_BASE_PORT + SELECT_DRIVE_REGISTER);
    outb(0x20, IDE_BASE_PORT + SELECT_STATUS_COMMAND_REGISTER);

    flag = 0;

    for(i = 0; i < 100; i++){
        char stat = inb(0x1F7);
        if(stat & 0x08){ // 80 for busy 08 for DRQ
            flag = 1;
            break;
        }
    }

    if(flag == 0){
        restore_flags(flags);
        return -1;
    }

    for (i = 0; i < 256; i++){

        uint16_t word = inw(0x1F0);

        buffer[i * 2] = (char)word;

        buffer[i * 2 + 1] = (char)(word >> 8);
    
    }

    for(i = 0; i < 5; i++){
        char stat = inb(0x1F7);
        if(stat & 0x21) return -1;
    }

    restore_flags(flags);

    return 0;

}

int write_sector(uint32_t lba, char * buffer){

    unsigned long flags;
    cli_and_save(flags);

    int i;

    unsigned char lba_low = LOW_8(lba);
    unsigned char lba_mid = NE1_8(lba);
    unsigned char lba_high = NE2_8(lba);
    unsigned char lba_top4 = NE3_4(lba);

    // outb(0x02, 0x3F6);
    outb(0x00, IDE_BASE_PORT + ERROR_REGISTER);
    outb(0x01, IDE_BASE_PORT + SELECT_COUNT_REGISTER);
    outb(lba_low, IDE_BASE_PORT + SELECT_NUMBER_REGISTER);
    outb(lba_mid, IDE_BASE_PORT + SELECT_LOW_REGISTER);
    outb(lba_high, IDE_BASE_PORT + SELECT_HIGH_REGISTER);
    outb((0xF0 | lba_top4), IDE_BASE_PORT + SELECT_DRIVE_REGISTER);
    outb(0x30, IDE_BASE_PORT + SELECT_STATUS_COMMAND_REGISTER);

    int flag = 0;

    for(i = 0; i < 100; i++){
        char stat = inb(0x1F7);
        if(stat & 0x08){
            flag = 1;
            break;
        }
    }

    if(flag == 0){
        restore_flags(flags);
        return -1;
    }

    for (i = 0; i < 256; i++){

        uint16_t t1 = 0x00;
        t1 += buffer[i * 2];
        uint16_t t2 = 0x00;
        t2 += buffer[i * 2 + 1];
        t2 = t2 << 8;

        uint16_t tmpword = (t1 & 0x00FF) + (t2 & 0xFF00);

        outw(tmpword, 0x1F0);

    }

    // while(1){

    //     char stat = inb(0x1F7);
    //     if(stat & 0x80 == 0x00) break;

    // }

    for(i = 0; i < 4; i++){
        char stat = inb(0x1F7);
        if(stat & 0x21) return -1;
    }

    outb(0xE7, IDE_BASE_PORT + SELECT_STATUS_COMMAND_REGISTER);

    // flag = 0;

    // for(i = 0; i < 100; i++){
    //     char stat = inb(0x1F7);
    //     if(stat & 0x21) return -1;
    //     if(stat & 0x80 == 0x00){
    //         flag = 1;
    //         break;
    //     }
    // }

    // if(flag == 0){
    //     restore_flags(flags);
    //     return -1;
    // }

    restore_flags(flags);

    return 0;

}
