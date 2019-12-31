#ifndef _UART_H_
#define _UART_H_

void Uart_init(uint32_t Adr);
int Uart_putc(uint32_t Adr,char c);
int Uart_haschar(uint32_t Adr);
int Uart_getchar(uint32_t Adr);

#endif // _UART_H_

