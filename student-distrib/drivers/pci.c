#include "pci.h"

uint32_t pciConfigReadWord (uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t addr;
    uint32_t lbus  = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;
 
    /* create configuration address as per Figure 1 */
    addr = (uint32_t)((lbus << 16) | (lslot << 11) |
              (lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));
 
    /* write out the address */
    outl(addr, 0xCF8);
    /* read in the data */
    /* (offset & 2) * 8) = 0 will choose the first word of the 32 bits register */
    uint32_t tmp = inl(0xCFC);
    return tmp;
}

uint16_t pciCheckVendor(uint8_t bus, uint8_t slot) {
    uint16_t vendor, device;
    /* try and read the first configuration register. Since there are no */
    /* vendors that == 0xFFFF, it must be a non-existent device. */
    if ((vendor = pciConfigReadWord(bus,slot,0,0)) != 0xFFFF) {
       device = pciConfigReadWord(bus,slot,0,2);
       printft("DEVICE FOUND: %x\n", device);
    } 
    printft("VENDOR: %x\n", vendor);
    return (vendor);
}

uint32_t pciGetBar0(uint8_t bus, uint8_t slot){

    uint32_t addr = 0x80001010;
    outl(addr, 0xCF8);
    uint32_t tmp = inl(0xCFC);
    return tmp;

}
