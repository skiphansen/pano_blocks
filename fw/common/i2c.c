#include <stdint.h>
#include <stdio.h>
#include "gpio_defs.h"
#include "timer.h"
#include "i2c.h"

#define DEBUG_LOGGING
#include "log.h"

#define REG_WR(reg, wr_data)       *((volatile uint32_t *)(reg)) = (wr_data)
#define REG_RD(reg)                *((volatile uint32_t *)(reg))

static void i2c_set_bit(ContextI2C *pCtx, int Value, int Bit)
{
   uint32_t Temp = REG_RD(pCtx->GpioBase + GPIO_DIRECTION);
   if(Value) {
   // set direction to input for high bit
      Temp &= ~Bit;
   }
   else {
   // set direction to output for low bit
      Temp |= Bit;
   }
   REG_WR(pCtx->GpioBase + GPIO_DIRECTION,Temp);
}

static void i2c_set_scl(ContextI2C *pCtx, int Value)
{
   i2c_set_bit(pCtx,Value,pCtx->BitSCL);
}

static void i2c_set_sda(ContextI2C *pCtx, int Value)
{
   i2c_set_bit(pCtx,Value,pCtx->BitSDA);
}


static int i2c_get_bit(ContextI2C *pCtx,int Bit)
{
   uint32_t Temp = REG_RD(pCtx->GpioBase + GPIO_INPUT);
   return (Temp & Bit) ? 1 : 0;
}

static int i2c_get_scl(ContextI2C *pCtx)
{
   return i2c_get_bit(pCtx,pCtx->BitSCL);
}

static int i2c_get_sda(ContextI2C *pCtx)
{
   return i2c_get_bit(pCtx,pCtx->BitSDA);
}

int i2c_init(ContextI2C *pCtx)
{
   int Ret = 0;   // assume the best
   int i;

   i2c_set_sda(pCtx,1);
   i2c_set_scl(pCtx,1);
// Precondition output data to zeros for SCL, SDA bits
   REG_WR(pCtx->GpioBase + GPIO_OUTPUT_CLR,pCtx->BitSCL);
   REG_WR(pCtx->GpioBase + GPIO_OUTPUT_CLR,pCtx->BitSDA);
   timer_sleep_us(pCtx->I2CDelay);
   if(i2c_get_scl(pCtx) != 1) {
      ELOG("Failed, SCL not high after init\n");
      Ret = 1;
   }
   if(i2c_get_sda(pCtx) != 1) {
      LOG("SDA not high after init\n");
   // Thump the clock a bunch to attempt to recover
      for(i = 0; i < 32; i++) {
         i2c_set_scl(pCtx,0);
         i2c_set_scl(pCtx,1);
      }
      if(i2c_get_sda(pCtx) != 1) {
         ELOG("Failed, SDA not high after init\n");
         Ret = 1;
      }
      else {
         LOG("SDA high after generating clocks\n");
      }
   }

   return Ret;
}

void i2c_start(ContextI2C *pCtx)
{
   i2c_set_sda(pCtx,1);             // i2c start bit sequence
   timer_sleep_us(pCtx->I2CDelay);
   i2c_set_scl(pCtx,1);
   timer_sleep_us(pCtx->I2CDelay);
   i2c_set_sda(pCtx,0);
   timer_sleep_us(pCtx->I2CDelay);
   i2c_set_scl(pCtx,0);
   timer_sleep_us(pCtx->I2CDelay);
}

void i2c_stop(ContextI2C *pCtx)
{
   i2c_set_sda(pCtx,0);             // i2c stop bit sequence
   timer_sleep_us(pCtx->I2CDelay);
   i2c_set_scl(pCtx,1);
   timer_sleep_us(pCtx->I2CDelay);
   i2c_set_sda(pCtx,1);
   timer_sleep_us(pCtx->I2CDelay);
}

unsigned char i2c_rx(ContextI2C *pCtx, char ack)
{
   char x, d=0;

   i2c_set_sda(pCtx,1);

   for(x=0; x<8; x++) {
      d <<= 1;

      i2c_set_scl(pCtx,1);
      timer_sleep_us(pCtx->I2CDelay);

      // wait for any i2c_set_scl clock stretching
      while(i2c_get_scl(pCtx) == 0);

      d |= i2c_get_sda(pCtx);
      i2c_set_scl(pCtx,0);
      timer_sleep_us(pCtx->I2CDelay);
   }

   if(ack) {
      i2c_set_sda(pCtx,0);
   }
   else {
      i2c_set_sda(pCtx,1);
   }

   i2c_set_scl(pCtx,1);
   timer_sleep_us(pCtx->I2CDelay);         // send (N)ACK bit

   i2c_set_scl(pCtx,0);
   timer_sleep_us(pCtx->I2CDelay);         // send (N)ACK bit

   i2c_set_sda(pCtx,1);
   return d;
}

// return 1: ACK, 0: NACK
int i2c_tx(ContextI2C *pCtx, unsigned char d)
{
   char x;
   int bit;

   for(x=8; x; x--) {
      i2c_set_sda(pCtx,(d & 0x80)>>7);
      d <<= 1;
      timer_sleep_us(pCtx->I2CDelay);
      i2c_set_scl(pCtx,1);
      timer_sleep_us(pCtx->I2CDelay);
      i2c_set_scl(pCtx,0);
   }
   i2c_set_sda(pCtx,1);
   timer_sleep_us(pCtx->I2CDelay);
   timer_sleep_us(pCtx->I2CDelay);
   bit = i2c_get_sda(pCtx);         // possible ACK bit
   i2c_set_scl(pCtx,1);
   timer_sleep_us(pCtx->I2CDelay);

   i2c_set_scl(pCtx,0);
   timer_sleep_us(pCtx->I2CDelay);

   return !bit;
}

// return 1: ACK, 0: NACK
int i2c_write_buf(ContextI2C *pCtx, uint8_t ADR, uint8_t* data, int len)
{
   int ack;

   i2c_start(pCtx);
   ack = i2c_tx(pCtx,ADR);
   if(!ack) {
      i2c_stop(pCtx);
      ELOG("Error: No ack on adr\n");
      return 0;
   }


   int i;
   for(i=0;i<len;++i) {
      ack = i2c_tx(pCtx,data[i]);
      if(!ack) {
         ELOG("Error: No data byte %d\n",i);
         i2c_stop(pCtx);
         return 0;
      }
   }

   i2c_stop(pCtx);

   return 1;
}

int i2c_read_buf(ContextI2C *pCtx, uint8_t ADR, uint8_t *data, int len)
{
   int ack;

   i2c_start(pCtx);

   ack = i2c_tx(pCtx,ADR | 1);
   if(!ack) {
      i2c_stop(pCtx);
      return 0;
   }

   int i;
   for(i=0; i < len; ++i) {
      data[i] = i2c_rx(pCtx,i != len-1);
   }
   i2c_stop(pCtx);

   return 1;
}

int i2c_write_reg_nr(ContextI2C *pCtx, uint8_t ADR, uint8_t reg_nr)
{
   return i2c_write_buf(pCtx,ADR, &reg_nr, 1);
}

// return 1: ACK, 0: NACK
int i2c_write_reg(ContextI2C *pCtx, uint8_t ADR, uint8_t reg_nr, uint8_t value)
{
   uint8_t data[2] = { reg_nr, value };

   return i2c_write_buf(pCtx,ADR, data, 2);
}

int i2c_write_regs(ContextI2C *pCtx, uint8_t ADR, uint8_t reg_nr, uint8_t *values, int len)
{
   int ack;

   i2c_start(pCtx);

   ack = i2c_tx(pCtx,ADR);
   if(!ack) {
      i2c_stop(pCtx);
      return 0;
   }

   ack = i2c_tx(pCtx,reg_nr);
   if(!ack) {
      i2c_stop(pCtx);
      return 0;
   }

   int i;
   for(i=0;i<len;++i) {
      ack = i2c_tx(pCtx,values[i]);
      if(!ack) {
         i2c_stop(pCtx);
         return 0;
      }
   }

   i2c_stop(pCtx);

   return 1;
}


int i2c_read_reg(ContextI2C *pCtx, uint8_t ADR, uint8_t reg_nr, uint8_t *value)
{
   int result;

   // Set ADRess to read
   result = i2c_write_buf(pCtx,ADR, &reg_nr, 1);
   if(!result)
      return 0;

   result = i2c_read_buf(pCtx,ADR, value, 1);
   if(!result)
      return 0;

   return 1;
}

int i2c_read_regs(ContextI2C *pCtx, uint8_t ADR, uint8_t reg_nr, uint8_t *values, int len)
{
   int result;

   // Set ADRess to read
   result = i2c_write_buf(pCtx,ADR, &reg_nr, 1);
   if(!result) {
      return 0;
   }

   result = i2c_read_buf(pCtx,ADR, values, len);
   if(!result) {
      return 0;
   }

   return 1;
}

