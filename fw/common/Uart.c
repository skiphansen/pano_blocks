// Derived from https://github.com/ultraembedded/core_soc.git
// License GPL2

#include <stdio.h>
#include "uart_lite.h"
#include "uart_lite_defs.h"

// #define DEBUG_LOGGING
#include "log.h"

//-----------------------------------------------------------------
// Uart_init: Initialise UART peripheral
//-----------------------------------------------------------------
void Uart_init(uint32_t Adr)           
{
    uint32_t cfg = 0;
    LOG("Called, base_addr: 0x%x\n",Adr);

    // Soft reset
    cfg += (1 << ULITE_CONTROL_RST_RX_SHIFT);
    cfg += (1 << ULITE_CONTROL_RST_TX_SHIFT);
    cfg += (1 << ULITE_CONTROL_IE_SHIFT);

    LOG("cfg: 0x%x\n",cfg);
    ((volatile uint32_t *)Adr)[ULITE_CONTROL/4]  = cfg;
    LOG("returning\n");
}
//-----------------------------------------------------------------
// Uart_putc: Polled putchar
//-----------------------------------------------------------------
int Uart_putc(uint32_t Adr,char c)
{
    // While TX FIFO full
    while (((volatile uint32_t *)Adr)[ULITE_STATUS/4] & (1 << ULITE_STATUS_TXFULL_SHIFT))
        ;

    ((volatile uint32_t *)Adr)[ULITE_TX/4] = c;

    return 0;
}
//-----------------------------------------------------------------
// Uart_haschar:
//-----------------------------------------------------------------
int Uart_haschar(uint32_t Adr)
{
    return (((volatile uint32_t *)Adr)[ULITE_STATUS/4] & (1 << ULITE_STATUS_RXVALID_SHIFT)) != 0;
}
//-----------------------------------------------------------------
// Uart_getchar: Read character from UART
//-----------------------------------------------------------------
int Uart_getchar(uint32_t Adr)
{
    if (Uart_haschar(Adr))
        return (uint8_t)((volatile uint32_t *)Adr)[ULITE_RX/4];
    else
        return -1;
}
