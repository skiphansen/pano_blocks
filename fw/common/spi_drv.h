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
#ifndef _SPI_DRV_H_
#define _SPI_DRV_H_

typedef struct {
   uint32_t FlashSize;
   uint32_t PageSize;
   uint32_t SectorSize;
   uint32_t EraseSize;
   uint8_t  DevId[3];
   const char *Desc;
} FlashInfo_t;

uint8_t spi_read_status();
int spi_write_enable(bool bEnable);
int spi_read_device_id(uint8_t *pBuf);
int spi_chip_init(void);
int32_t spi_read(uint32_t Adr,uint8_t *Buf,uint32_t size);
int spi_write(uint32_t Adr,uint8_t *pData,uint32_t Len);
int spi_erase(uint32_t Adr, uint32_t Len);
int spi_chip_init(void);
const FlashInfo_t *spi_get_flashinfo(void);
#endif // _SPI_DRV_H_

