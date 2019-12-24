#include <stdbool.h>

#include "spiffs.h"
#include "spi_lite.h"

// #define DEBUG_LOGGING
// #define VERBOSE_DEBUG_LOGGING
#include "log.h"

#define MAX_OPEN_FILES 4

spiffs gFs;
uint8_t gWorkBuf[SPIFFS_CFG_LOG_PAGE_SZ(x) * 2];
uint8_t gFds[32*MAX_OPEN_FILES];

static s32_t api_spiffs_read(u32_t addr, u32_t size, u8_t *Buf)
{
   u32_t TempAdr = addr;

   VLOG("Called, adr 0x%x size %ld, buf %p\n",addr,size,Buf);
   spi_cs(0);
   spi_sendrecv(0x03);
   spi_sendrecv((uint8_t) ((TempAdr >> 16) & 0xff));
   spi_sendrecv((uint8_t) ((TempAdr >> 8) & 0xff));
   spi_sendrecv((uint8_t) (TempAdr & 0xff));
   spi_readblock(Buf,1);
   spi_readblock(Buf,size);
   spi_cs(1);
   VLOG_HEX(Buf,size);
   return SPIFFS_OK;
}

static s32_t api_spiffs_write(u32_t addr, u32_t size, u8_t *src)
{
   return SPIFFS_ERR_NOT_WRITABLE;
}

static s32_t api_spiffs_erase(u32_t addr, u32_t size)
{
   return SPIFFS_ERR_NOT_WRITABLE;
}

//implementation
int SpiffsTryMount()
{
   spiffs_config cfg;

   memset(&cfg,0,sizeof(cfg));

   cfg.hal_read_f = api_spiffs_read;
   cfg.hal_write_f = api_spiffs_write;
   cfg.hal_erase_f = api_spiffs_erase;

   return SPIFFS_mount(&gFs,&cfg,gWorkBuf,gFds,sizeof(gFds),NULL,0,NULL);
}

spiffs *SpiffsMount()
{
   spiffs *Ret = NULL;

   VLOG("Called\n");
   if(SPIFFS_mounted(&gFs)) {
      VLOG("FS already mounted\n");
      Ret = &gFs;
   }
   else {
      int res = SpiffsTryMount();
      if(res != SPIFFS_OK) {
         ELOG("SPIFFS mount failed %d\n",res);
      }
      else {
         VLOG("SpiffsTryMount succeeded\n");
         Ret = &gFs;
      }
   }

   return Ret;
}

bool SpiffsFormat()
{
   SpiffsMount(false);
   SPIFFS_unmount(&gFs);
   int formated = SPIFFS_format(&gFs);
   if(formated != SPIFFS_OK) {
      return false;
   }
   return(SpiffsTryMount() == SPIFFS_OK);
}

void SpiffsUnmount()
{
   if(SPIFFS_mounted(&gFs)) {
      SPIFFS_unmount(&gFs);
   }
}


