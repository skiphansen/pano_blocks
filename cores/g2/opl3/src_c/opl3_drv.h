#ifndef _OPL3_DRV_H_
#define _OPL3_DRV_H_

#define NUM_OPS_PER_BANK   18
#define NUM_BANKS          2
#define REG_ADR(x,y)       ((x << 8) + y)

// Lookup table starting register offset indexed by operator number (0->17)
extern const uint8_t OpOffset[NUM_OPS_PER_BANK];

void Opl3WriteReg(uint8_t Chip,uint16_t RegOffset,uint8_t Data);

#endif   // _OPL3_DRV_H_

