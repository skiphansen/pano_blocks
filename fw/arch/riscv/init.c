#include "csr.h"
#include "exception.h"

#ifdef CONFIG_UARTLITE_BASE
#include "uart_lite.h"
#endif

#ifdef CONFIG_IRQCTRL_BASE
#include "irq_ctrl.h"
#endif

#ifdef CONFIG_SPILITE_BASE
#include "spi_lite.h"
#endif

#ifdef CONFIG_MALLOC
#include "malloc.h"
static uint8_t _heap[CONFIG_MALLOC_SIZE];
#endif

#include "printf.h"

//-----------------------------------------------------------------
// init:
//-----------------------------------------------------------------
void init(void)
{
#ifdef CONFIG_UARTLITE_BASE
    // Setup serial port
    uartlite_init(CONFIG_UARTLITE_BASE);
#endif

    // Register serial driver with printf
#ifndef ALTERNATE_PRINTF
    printf_register(uartlite_putc);
#endif

#ifdef CONFIG_SPILITE_BASE
    spi_init(CONFIG_SPILITE_BASE);
#endif

#ifdef CONFIG_MALLOC
    malloc_init(_heap, CONFIG_MALLOC_SIZE, 0, 0);
#endif

#ifdef CONFIG_IRQCTRL_BASE
    irqctrl_init(CONFIG_IRQCTRL_BASE);
#endif
}
