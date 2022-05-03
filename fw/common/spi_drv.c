/*
 *  Copyright (C) 2022  Skip Hansen
 * 
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms and conditions of the GNU General Public License,
 *  version 2, as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 *  more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "spi_lite.h"
#include "spi_drv.h"

#define DEBUG_LOGGING
#define VERBOSE_DEBUG_LOGGING
#include "log.h"

#ifdef PANO_G2
#define PAGE_SIZE          (256)
#define SECTOR_SIZE        (256*1024)
#define FLASH_SIZE         (16*1024*1024)
#elif defined(PANO_G2_C)
#define SECTOR_SIZE        (64*1024)
#define FLASH_SIZE         (8*1024*1024)
#else
#error Unknown platform
#endif

#define CMD_WRITE_STATUS   0x01
#define CMD_PAGE_PRG       0x02
#define CMD_READ_DATA      0x03
#define CMD_WRITE_DISABLE  0x04
#define CMD_READ_STATUS    0x05
#define CMD_WRITE_ENABLE   0x06
#define CMD_READ_DATA_FAST 0x0b
#define CMD_SECTOR_ERASE   0xd8
#define CMD_BULK_ERASE     0xc7
#define CMD_READ_ID        0x9f

// bits in status register
#define STATUS_WIP         0x01
#define STATUS_WEL         0x02
#define STATUS_BP0         0x04
#define STATUS_BP1         0x08
#define STATUS_BP2         0x10
#define STATUS_SRWD        0x80

uint8_t spi_read_status()
{
   uint8_t Ret;

   spi_cs(0);
   spi_sendrecv(CMD_READ_STATUS);
   spi_readblock(&Ret,1);
   spi_cs(1);

   return Ret;
}

int spi_write_enable(bool bEnable)
{
   spi_cs(0);
   spi_sendrecv(bEnable ? CMD_WRITE_ENABLE : CMD_WRITE_DISABLE);
   spi_cs(1);
}

int spi_read_device_id(uint8_t *pBuf)
{
   spi_cs(0);
   spi_sendrecv(CMD_READ_ID);
   spi_readblock(pBuf,3);
   spi_cs(1);

   return 0;
}

int spi_chip_init()
{
   int Ret = -1;  // assume the worse
   uint8_t DeviceID[3];
   const uint8_t M25P128_ID[3] = {0x20,0x20,0x18};

   spi_read_device_id(DeviceID);
   if(memcmp(DeviceID,M25P128_ID,3) == 0) {
      ALOG("M25P128 detected\n");
      Ret = 0;
   }
   else {
      ELOG("Unknown DeviceID ");
      LOG_HEX(DeviceID,sizeof(DeviceID));
   }

   return Ret;
}

int32_t spi_read(uint32_t Adr,uint8_t *Buf,uint32_t size)
{
   VLOG("Called, adr 0x%x size %ld, buf %p\n",Adr,size,Buf);
   spi_cs(0);
   spi_sendrecv(CMD_READ_DATA);
   spi_sendrecv((uint8_t) ((Adr >> 16) & 0xff));
   spi_sendrecv((uint8_t) ((Adr >> 8) & 0xff));
   spi_sendrecv((uint8_t) (Adr & 0xff));
   spi_readblock(Buf,1);
   spi_readblock(Buf,size);
   spi_cs(1);
   VLOG_HEX(Buf,size);
   return 0;
}

int spi_write(uint32_t Adr,uint8_t *pData,uint32_t Len)
{
   int Bytes2Write;
   int BytesLeftInPage;
   int Wrote = 0;

   do {
      if(Adr + Len > FLASH_SIZE) {
         ELOG("Invalid length 0x%x\n",Len);
         break;
      }
      spi_write_enable(true);
      while(Wrote < Len) {
         Bytes2Write = Len;
         BytesLeftInPage = PAGE_SIZE - (Adr % PAGE_SIZE);
         if(Bytes2Write > BytesLeftInPage) {
            Bytes2Write = BytesLeftInPage;
         }
         spi_cs(0);
         spi_sendrecv(CMD_PAGE_PRG);
         spi_sendrecv((uint8_t) ((Adr >> 16) & 0xff));
         spi_sendrecv((uint8_t) ((Adr >> 8) & 0xff));
         spi_sendrecv((uint8_t) (Adr & 0xff));
         spi_writeblock(pData,Bytes2Write);
         spi_cs(1);
         while(spi_read_status() & STATUS_WIP);
         Adr += Bytes2Write;
         Wrote += Bytes2Write;
      }
   } while(false);

   return Wrote;
}

int spi_erase(uint32_t Adr, uint32_t Len)
{
   int i;
   int Ret = -1;  // assume the wrose

   VLOG("Adr 0x%x, size 0x%x\n",Adr,Len);
   do {
      if((Adr % SECTOR_SIZE) != 0) {
         ELOG("Invalid address 0x%x, not start of sector\n",Adr);
         break;
      }

      if(Adr + Len > FLASH_SIZE) {
         ELOG("Invalid length 0x%x\n",Len);
         break;
      }
      spi_write_enable(true);
      for(i = 0; i < Len / SECTOR_SIZE; i++) {
         VLOG("Erasing sector %d @ 0x%x\n",i + 1,Adr);
         spi_cs(0);
         spi_sendrecv(CMD_SECTOR_ERASE);
         spi_sendrecv((uint8_t) ((Adr >> 16) & 0xff));
         spi_sendrecv((uint8_t) ((Adr >> 8) & 0xff));
         spi_sendrecv((uint8_t) (Adr & 0xff));
         spi_cs(1);
         while(spi_read_status() & STATUS_WIP);
         Adr += SECTOR_SIZE;
      }

   } while(false);

   return Ret;
}

