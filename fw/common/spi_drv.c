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

// #define DEBUG_LOGGING
// #define VERBOSE_DEBUG_LOGGING
#include "log.h"

const FlashInfo_t gChipInfo[] = {
// PANO_G2 Rev C, 8 Megabytes
   {8*1024*1024,256,64*1024,64*1024,{0x20,0x20,0x17},"M25P64"},
// PANO_G2 Rev B, 16 Megabytes
   {16*1024*1024,256,256*1024,256*1024,{0x20,0x20,0x18},"M25P128"},
   {0}   // end of table
};

static const FlashInfo_t *gChip;

#define CMD_WRITE_STATUS   0x01
#define CMD_PAGE_PRG       0x02
#define CMD_READ_DATA      0x03     // Max clock 20 Mhz
#define CMD_WRITE_DISABLE  0x04
#define CMD_READ_STATUS    0x05
#define CMD_WRITE_ENABLE   0x06
#define CMD_READ_DATA_FAST 0x0b     // Max clock 50 Mhz
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
   const FlashInfo_t *p = gChipInfo;

   spi_read_device_id(DeviceID);
   while(p->FlashSize != 0) {
      if(memcmp(p->DevId,DeviceID,sizeof(p->DevId)) == 0) {
         ALOG("%s detected\n",p->Desc);
         gChip = p;
         Ret = 0;
         break;
      }
      p++;
   }

   if(Ret != 0) {
      ELOG("Unknown DeviceID ");
      LOG_HEX(DeviceID,sizeof(DeviceID));
   }

   return Ret;
}

int32_t spi_read(uint32_t Adr,uint8_t *Buf,uint32_t Len)
{
   int BytesRead = 0;
   int Bytes2Read;
   const FlashInfo_t *p = gChip;
   VLOG("Called, adr 0x%x size %ld, buf %p\n",Adr,Len,Buf);

   while(BytesRead < Len) {
      Bytes2Read = Len - BytesRead;
      if(Bytes2Read > p->PageSize) {
         Bytes2Read = p->PageSize;
      }
      spi_cs(0);
      spi_sendrecv(CMD_READ_DATA);
      spi_sendrecv((uint8_t) ((Adr >> 16) & 0xff));
      spi_sendrecv((uint8_t) ((Adr >> 8) & 0xff));
      spi_sendrecv((uint8_t) (Adr & 0xff));
      spi_readblock(Buf,Bytes2Read);
      spi_cs(1);
      VLOG_HEX(Buf,Bytes2Read);
      BytesRead += Bytes2Read;
      Buf += Bytes2Read;
      Adr += Bytes2Read;
   }
   return 0;
}

int spi_write(uint32_t Adr,uint8_t *pData,uint32_t Len)
{
   int Bytes2Write;
   int BytesLeftInPage;
   int Wrote = 0;
   const FlashInfo_t *p = gChip;

   do {
      VLOG("Called, adr 0x%x Len %ld, pData %p\n",Adr,Len,pData);
      if(p == NULL) {
         ELOG("Unknown chip\n");
         break;
      }
      if(Adr + Len > p->FlashSize) {
         ELOG("Invalid length 0x%x\n",Len);
         break;
      }
      while(Wrote < Len) {
         Bytes2Write = Len;
         BytesLeftInPage = p->PageSize - (Adr % p->PageSize);
         if(Bytes2Write > BytesLeftInPage) {
            Bytes2Write = BytesLeftInPage;
         }
         VLOG("write %d bytes @ 0x%lx from %p BytesLeftInPage %d\n",
              Bytes2Write,Adr,pData,Bytes2Write);
         spi_write_enable(true);
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
         pData += Bytes2Write;
      }
   } while(false);

   return Wrote;
}

int spi_erase(uint32_t Adr, uint32_t Len)
{
   int i;
   int Ret = -1;  // assume the wrose
   const FlashInfo_t *p = gChip;

   VLOG("Adr 0x%x, size 0x%x\n",Adr,Len);
   do {
      if(p == NULL) {
         ELOG("Unknown chip\n");
         break;
      }
      if((Adr % p->EraseSize) != 0) {
         ELOG("Invalid address 0x%x, not start of erase boundary\n",Adr);
         break;
      }

      if((Adr + Len) > p->FlashSize) {
         ELOG("Invalid length 0x%x\n",Len);
         break;
      }
      spi_write_enable(true);
      for(i = 0; i < (Len / p->EraseSize); i++) {
         spi_cs(0);
         spi_sendrecv(CMD_SECTOR_ERASE);
         spi_sendrecv((uint8_t) ((Adr >> 16) & 0xff));
         spi_sendrecv((uint8_t) ((Adr >> 8) & 0xff));
         spi_sendrecv((uint8_t) (Adr & 0xff));
         spi_cs(1);
         while(spi_read_status() & STATUS_WIP);
         Adr += p->SectorSize;
      }
   } while(false);

   return Ret;
}

const FlashInfo_t *spi_get_flashinfo()
{
   return gChip;
}
