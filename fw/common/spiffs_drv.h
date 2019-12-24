#ifndef _SPIFFS_DRV_H_
#define _SPIFFS_DRV_H_

spiffs *SpiffsMount(void);
bool SpiffsFormat(void);
void SpiffsUnmount(void);

#endif   //_SPIFFS_DRV_H_

