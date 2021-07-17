#ifndef _PCI_H
#define _PCI_H

#include "../lib.h"
#include "terminal.h"

uint32_t pciConfigReadWord (uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
uint16_t pciCheckVendor(uint8_t bus, uint8_t slot);
uint32_t pciGetBar0(uint8_t bus, uint8_t slot);

#endif
