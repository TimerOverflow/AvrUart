#define ENABLE_BIT_DEFINITIONS
#include <iom64.h>
#include <ina90.h>
#include <string.h>
#include "AvrUart.h"
#include "AvrUartBaud.h"

#define __CPU_CLK__       14745600L

#define GPIO_485_ENABLE_PORT          PORTD
#define GPIO_485_ENABLE_PIN           PIND
#define GPIO_485_ENABLE               5

tag_AvrUartCtrl Uart0;
tag_UartBaudControl Uart0Baud;

tU8 Uart0_TxBuf[256];
tU8 Uart0_RxBuf[256];

#pragma vector = USART0_RXC_vect
__interrupt void USART0_RXC_ISR(void)
{
  AvrUartRxQueueControl(&Uart0);
}

#pragma vector = USART0_TXC_vect
__interrupt void USART0_TXC_ISR(void)
{
  AvrUartTxQueueControl(&Uart0);
}

void main( void )
{
  char str[20];
  
  DDRD |= (1 << DDD5);
  //GPIO direction.
  
  UCSR0B = (1 << RXCIE0) | (1 << TXCIE0) | (1 << RXEN0) | (1 << TXEN0);
  //uart enable.
  
  AvrUartLinkRegister(&Uart0, (tU8 *) &UDR0, (tU8 *) &UCSR0A, (tU8 *) &GPIO_485_ENABLE_PORT, GPIO_485_ENABLE);
  AvrUartLinkBuffer(&Uart0, Uart0_TxBuf, sizeof(Uart0_TxBuf), Uart0_RxBuf, sizeof(Uart0_RxBuf));
  AvrUartGeneralInit(&Uart0);
  Uart0.ReceivingDelay = 1000;  //may need adjust..dependent loop tick period
  //uart0 init.
  
  AvrUartBaudControlInit(&Uart0Baud, __CPU_CLK__, (tU8 *) &UBRR0L, (tU8 *) &UBRR0H);
  //uart0baud init.
  
  AvrUartBaudChange(&Uart0Baud, BAUD_9600);
  //setting baudrate
  
  __enable_interrupt();
  
  while(1)
  {
    if((AvrUartCheckRx(&Uart0) > 0) && (AvrUartCheckReceiving(&Uart0) == false))
    {
      //receive more than one byte and wait until receive end.
      
      memset(str, 0, sizeof(str));
      AvrUartGetData(&Uart0, (tU8 *) str, AvrUartCheckRx(&Uart0));
      if(strcmp(str, "CALL") == 0)
      {
        AvrUartPutData(&Uart0, "RESPONE", sizeof("RESPONE"));
        AvrUartStartTx(&Uart0);
      }
      AvrUartClearQueueBuf(&Uart0.RxQueue);
    }
  }
}
