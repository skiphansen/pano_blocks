#ifndef I2C_H
#define I2C_H

#include "pano_io.h"

#define MCP23017_I2C_ADR      0x40
#define WM8750L_I2C_ADR       0x34

typedef struct {
   uint32_t GpioBase;
   uint32_t BitSCL;
   uint32_t BitSDA;
// Slow I2C devices need 4.7 microseconds ... but the Pano doesn't have
// Slow devices, but provide an optional delay for external I2C devices that
// might be slow.
   int I2CDelay;  // microseconds
} ContextI2C;

int i2c_init(ContextI2C *pCtx);
void i2c_dly();
void i2c_start(ContextI2C *pContext);
void i2c_stop(ContextI2C *pContext);
unsigned char i2c_rx(ContextI2C *pContext, char ack);
int i2c_tx(ContextI2C *pContext, unsigned char d);
int i2c_write_buf(ContextI2C *pContext, uint8_t addr, uint8_t* data, int len);
int i2c_read_buf(ContextI2C *pContext, uint8_t addr, uint8_t *data, int len);
int i2c_write_reg_nr(ContextI2C *pContext, uint8_t addr, uint8_t reg_nr);
int i2c_write_reg(ContextI2C *pContext, uint8_t addr, uint8_t reg_nr, uint8_t value);
int i2c_write_regs(ContextI2C *pContext, uint8_t addr, uint8_t reg_nr, uint8_t *values, int len);
int i2c_read_reg(ContextI2C *pContext, uint8_t addr, uint8_t reg_nr, uint8_t *value);
int i2c_read_regs(ContextI2C *pContext, uint8_t addr, uint8_t reg_nr, uint8_t *values, int len);

#endif
