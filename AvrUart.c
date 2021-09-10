/*********************************************************************************/
/*
 * Author : Jung Hyun Gu
 * File name : AvrUart.c
*/
/*********************************************************************************/
#include <string.h>
#include "AvrUart.h"
/*********************************************************************************/
#if(AVR_UART_REVISION_DATE != 20161117)
#error wrong include file. (AvrUart.h)
#endif
/*********************************************************************************/
/** Global variable **/


/*********************************************************************************/
static char CheckAllOfInit(tag_AvrUartCtrl *Com)
{
	return (Com->Bit.InitRegister && Com->Bit.InitBuffer && Com->Bit.InitGeneral) ? true : false;
}
/*********************************************************************************/
static char* MoveBufPointer(char *Ptr, tag_AvrUartRingBuf *Que, int Move, enum_AvrUartMoveDirection Direction)
{
	int Distance;
	
	switch(Direction)
	{
		case	AVR_UART_FORWARD	:
		
			Distance = &Que->Buf[Que->Size - 1] - Ptr;
			if(Move > Distance)
			{
				Move--;
				Ptr = Que->Buf + (Move - Distance);
			}
			else
			{
				Ptr += Move;
			}
		break;
		
		case	AVR_UART_BACKWARD	:
		
			Distance = Ptr - Que->Buf;
			if(Move > Distance)
			{
				Move--;
				Ptr = &Que->Buf[Que->Size - 1] - (Move - Distance);
			}
			else
			{
				Ptr -= Move;
			}
		break;
	}
	
	return Ptr;
}
/*********************************************************************************/
char AvrUartLinkRegister(tag_AvrUartCtrl *Com, char *pUDR, char *pUCSRA, char *pEnablePort, char EnablePin)
{
	Com->pUDR = pUDR;
	Com->pUCSRA = pUCSRA;
	Com->pEnablePort = pEnablePort;
	Com->EnablePin = EnablePin;
	
	Com->Bit.InitRegister = true;
	Com->Bit.InitComplete = CheckAllOfInit(Com);
	
	return Com->Bit.InitRegister;
}
/*********************************************************************************/
char AvrUartLinkBuffer(tag_AvrUartCtrl *Com, char *TxBuf, int TxBufSize, char *RxBuf, int RxBufSize)
{
	Com->TxQueue.Buf = TxBuf;
	Com->TxQueue.Size = TxBufSize;
	
	Com->RxQueue.Buf = RxBuf;
	Com->RxQueue.Size = RxBufSize;
	
	Com->Bit.InitBuffer = true;
	Com->Bit.InitComplete = CheckAllOfInit(Com);
	
	return Com->Bit.InitBuffer;
}
/*********************************************************************************/
char AvrUartGeneralInit(tag_AvrUartCtrl *Com)
{
	if(Com->Bit.InitBuffer == true)
	{
		Com->TxQueue.InPtr = Com->TxQueue.OutPtr = Com->TxQueue.Buf;
		memset(Com->TxQueue.Buf, 0, Com->TxQueue.Size);
		Com->TxQueue.Ctr = 0;
		
		Com->RxQueue.InPtr = Com->RxQueue.OutPtr = Com->RxQueue.Buf;
		memset(Com->RxQueue.Buf, 0, Com->RxQueue.Size);
		Com->RxQueue.Ctr = 0;
		
		Com->Bit.InitGeneral = true;
	}

	Com->Bit.InitComplete = CheckAllOfInit(Com);
	
	return Com->Bit.InitGeneral;
}
/*********************************************************************************/
void AvrUartPutData(tag_AvrUartCtrl *Com, char *Buf, int Length)
{
	int i;
	
	if(Com->Bit.InitComplete == false)
	{
		return;
	}
	
	for(i = 0; i < Length; i++)
	{
		AvrUartPutChar(Com, Buf[i]);
	}
}
/*********************************************************************************/
void AvrUartPutChar(tag_AvrUartCtrl *Com, char Char)
{
	tag_AvrUartRingBuf *TxQue = &Com->TxQueue;
	
	if(Com->Bit.InitComplete == false)
	{
		return;
	}
	
	if(TxQue->Ctr < TxQue->Size)
	{
		TxQue->Ctr++;
		*TxQue->InPtr = Char;
		
		TxQue->InPtr = MoveBufPointer(TxQue->InPtr, TxQue, 1, AVR_UART_FORWARD);
	}
}
/*********************************************************************************/
void AvrUartTxQueueControl(tag_AvrUartCtrl *Com)
{
	tag_AvrUartRingBuf *TxQue = &Com->TxQueue;
	
	if(Com->Bit.InitComplete == false)
	{
		return;
	}
	
	if(TxQue->Ctr)
	{
		TxQue->Ctr--;
		*Com->pUDR = *TxQue->OutPtr;
		
		TxQue->OutPtr = MoveBufPointer(TxQue->OutPtr, TxQue, 1, AVR_UART_FORWARD);
	}
	else
	{
		Com->Bit.DataSend = false;
		*Com->pEnablePort &= ~(1 << Com->EnablePin);
	}
}
/*********************************************************************************/
void AvrUartStartTx(tag_AvrUartCtrl *Com)
{
	if(Com->Bit.InitComplete == false)
	{
		return;
	}
	
	while((*Com->pUCSRA & 0x20) == 0);
	Com->Bit.DataSend = true;
	*Com->pEnablePort |= (1 << Com->EnablePin);
	AvrUartTxQueueControl(Com);
}
/*********************************************************************************/
void AvrUartRxQueueControl(tag_AvrUartCtrl *Com)
{
	volatile char Temp;
	tag_AvrUartRingBuf *RxQue = &Com->RxQueue;
	
	if(Com->Bit.InitComplete == false)
	{
		return;
	}
	
	if((Com->Bit.DataSend == true) || (RxQue->Ctr >= RxQue->Size))
	{
		Temp = *Com->pUDR;
	}
	else
	{
		RxQue->Ctr++;
		*RxQue->InPtr = *Com->pUDR;
		
		RxQue->InPtr = MoveBufPointer(RxQue->InPtr, RxQue, 1, AVR_UART_FORWARD);
		Com->ReceivingCnt = Com->ReceivingDelay;
	}
}
/*********************************************************************************/
int AvrUartCheckRx(tag_AvrUartCtrl *Com)
{
	return Com->RxQueue.Ctr;
}
/*********************************************************************************/
int AvrUartCheckTx(tag_AvrUartCtrl *Com)
{
	return Com->TxQueue.Ctr;
}
/*********************************************************************************/
void AvrUartGetChar(tag_AvrUartCtrl *Com, char *Char)
{
	tag_AvrUartRingBuf *RxQue = &Com->RxQueue;
	
	if(Com->Bit.InitComplete == false)
	{
		return;
	}
	
	if(RxQue->Ctr)
	{
		RxQue->Ctr--;
		*Char = *RxQue->OutPtr;
		
		RxQue->OutPtr = MoveBufPointer(RxQue->OutPtr, RxQue, 1, AVR_UART_FORWARD);
	}
}
/*********************************************************************************/
void AvrUartGetData(tag_AvrUartCtrl *Com, char *Buf, int BufSize)
{
	int i;
	
	if(Com->Bit.InitComplete == false)
	{
		return;
	}
	
	for(i = 0; i < BufSize; i++)
	{
		AvrUartGetChar(Com, Buf++);
	}
}
/*********************************************************************************/
void AvrUartClearRx(tag_AvrUartCtrl *Com)
{
	if(Com->Bit.InitComplete == false)
	{
		return;
	}
	
	Com->RxQueue.OutPtr = Com->RxQueue.InPtr = Com->RxQueue.Buf;
	Com->RxQueue.Ctr = 0;
}
/*********************************************************************************/
char AvrUartViewRxBuf(tag_AvrUartCtrl *Com, int Move, enum_AvrUartMoveDirection Direction)
{
	tag_AvrUartRingBuf *RxQue = &Com->RxQueue;
	char *TagetBuf = RxQue->OutPtr;
	
	if(Com->Bit.InitComplete == false)
	{
		return null;
	}
	
	TagetBuf = MoveBufPointer(TagetBuf, RxQue, Move, Direction);

	return *TagetBuf;
}
/*********************************************************************************/
char AvrUartCheckReceiving(tag_AvrUartCtrl *Com)
{
	if(Com->Bit.InitComplete == false)
	{
		return false;
	}
	
	if(Com->ReceivingCnt) Com->ReceivingCnt--;
	return Com->ReceivingCnt ? true : false;
}
/*********************************************************************************/














