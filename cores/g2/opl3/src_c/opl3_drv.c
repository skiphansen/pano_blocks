#include <stdio.h>
#include <stdint.h>

#include "pano_io.h"
#include "opl3_drv.h"

// #define DEBUG_LOGGING
// #define VERBOSE_DEBUG_LOGGING
#include "log.h"

const uint8_t OpOffset[] = {
   0x0,0x1,0x2,0x3,0x4,0x5,
   0x8,0x9,0xa,0xb,0xc,0xd,
   0x10,0x11,0x12,0x13,0x14,0x15
};

void Opl3WriteReg(uint8_t Chip,uint16_t RegOffset,uint8_t Data)
{
   volatile uint32_t *p;

   p = (volatile uint32_t *)(OPL3_BASE + (Chip << 10) + (RegOffset << 2));
   VLOG("0x%02x -> 0x%02x ( 0x%p)\n",Data,RegOffset,p);
   *p = Data;
}

