#include <stdio.h>
#include <stdint.h>

#include "log.h"

#ifndef LOGGING_DISABLED
void LogHex(int LogFlags,void *Data,int Len)
{
   int i;
   int SentNl;
   uint8_t *cp = (uint8_t *) Data;

   for(i = 0; i < Len; i++) {
      if(i != 0 && (i & 0xf) == 0) {
         _LOG(LogFlags,"\n");
      }
      else if(i != 0) {
         _LOG(LogFlags," ");
      }
      _LOG(LogFlags,"%02x",cp[i]);
   }
   if(i == 1 || ((i - 1) & 0xf) != 0) {
      _LOG(LogFlags,"\n");
   }
}
#endif

