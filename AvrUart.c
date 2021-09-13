/*********************************************************************************/
/*
 * Author : Jeong Hyun Gu
 * File name : AvrUart.c
*/
/*********************************************************************************/
#include <string.h>
#include "AvrUart.h"
/*********************************************************************************/
#if(AVR_UART_REVISION_DATE != 20170322)
#error wrong include file. (AvrUart.h)
#endif
/*********************************************************************************/
#define INC_BUF_POINTER(__PTR__, __RING_BUF__)	(__PTR__ == &(__RING_BUF__)->Buf[(__RING_BUF__)->Size - 1] ? \
																								__PTR__ = (__RING_BUF__)->Buf : \
																								__PTR__++)
/*********************************************************************************/
/** Global variable **/


/*********************************************************************************/
static char CheckAllOfInit(tag_AvrUartCtrl *Com)
{
	/*
		1) 인수
			- Com : tag_AvrUartCtrl 인스턴스의 주소.

		2) 반환
		  - 0	: 초기화 실패.
			- 1	:	초기화 성공.

		3) 설명
			- 'tag_AvrUartCtrl' 인스턴스의 필수 항목 초기화 여부 확인.
	*/

	return (Com->Bit.InitRegister && Com->Bit.InitBuffer && Com->Bit.InitGeneral) ? true : false;
}
/*********************************************************************************/
char AvrUartLinkRegister(tag_AvrUartCtrl *Com, char *pUDR, char *pUCSRA, char *pEnablePort, char EnablePin)
{
	/*
		1) 인수
			- Com : tag_AvrUartCtrl 인스턴스의 주소.
			- pUDR : UDRn 레지스터의 주소.
			- pUCSRA : UCSRnA 레지스터의 주소.
			- pEnablePort : PORTn 레지스터의 주소. (Tx enable)
			- EnablePin : pin 번호. (Tx enable)

		2) 반환
		  - 0	: 초기화 실패
			- 1	:	초기화 성공

		3) 설명
			- AVR의 UART 관련 기능 레지스터와 물리핀 제어를 위한 GPIO 연결.
	*/

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
	/*
		1) 인수
			- Com : tag_AvrUartCtrl 인스턴스의 주소.
			- TxBuf : 송신 버퍼의 주소
			- TxBufSize : 송신 버퍼의 크기 (Byte)
			- RxBuf : 수신 버퍼의 주소
			- RxBufSize : 수신 버퍼의 크기 (Byte)

		2) 반환
		  - 0	: 초기화 실패
			- 1	:	초기화 성공

		3) 설명
			- tag_AvrUartCtrl 타입 구조체에 송수신 버퍼 연결.
	*/

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
	/*
		1) 인수
			- Com : tag_AvrUartCtrl 인스턴스의 주소.

		2) 반환
		  - 0	: 초기화 실패
			- 1	:	초기화 성공

		3) 설명
			- UART 관리를 위해 송수신 버퍼 초기화.
	*/

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

	/*
		1) 인수
			- Com : tag_AvrUartCtrl 인스턴스의 주소.
			- Buf : 송신할 데이터가 저장된 버퍼의 주소.
			- Length : 길이 (Byte)

		2) 반환
		  - 없음.

		3) 설명
			- 설정한 길이만큼 지정한 버퍼의 데이터를 송신 버퍼에 저장.
			- 본 함수 호출 후 AvrUartStartTx() 함수를 호출해야 송신이 시작됨.
	*/

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

	/*
		1) 인수
			- Com : tag_AvrUartCtrl 인스턴스의 주소.
			- Char : 송신할 데이터 (1Byte)

		2) 반환
		  - 없음.

		3) 설명
			- 1Byte 데이터를 송신 버퍼에 저장.
			- 본 함수 호출 후 AvrUartStartTx() 함수를 호출해야 송신이 시작됨.
	*/

	if(Com->Bit.InitComplete == false)
	{
		return;
	}

	if(TxQue->Ctr < TxQue->Size)
	{
		TxQue->Ctr++;
		*TxQue->InPtr = Char;

		INC_BUF_POINTER(TxQue->InPtr, TxQue);
	}
}
/*********************************************************************************/
void AvrUartTxQueueControl(tag_AvrUartCtrl *Com)
{
	tag_AvrUartRingBuf *TxQue = &Com->TxQueue;

	/*
		1) 인수
			- Com : tag_AvrUartCtrl 인스턴스의 주소.

		2) 반환
		  - 없음.

		3) 설명
			- UART 채널의 송신 관리.
			- 본 함수는 TXC 인터럽트에서 호출.
	*/

	if(Com->Bit.InitComplete == false)
	{
		return;
	}

	if(TxQue->Ctr)
	{
		TxQue->Ctr--;
		*Com->pUDR = *TxQue->OutPtr;

		INC_BUF_POINTER(TxQue->OutPtr, TxQue);
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
	/*
		1) 인수
			- Com : tag_AvrUartCtrl 인스턴스의 주소.

		2) 반환
		  - 없음.

		3) 설명
			- 본 함수를 호출할 경우 송신 버퍼에 저장된 데이터 송신을 시작함.
			- AvrUartPutData(), AvrUartPutChar() 함수를 호출하여 송신 버퍼에 데이터 저장.
	*/

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

	/*
		1) 인수
			- Com : tag_AvrUartCtrl 인스턴스의 주소.

		2) 반환
		  - 없음.

		3) 설명
			- UART 채널의 수신 관리.
			- 본 함수는 RXC 인터럽트에서 호출.
	*/

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

		INC_BUF_POINTER(RxQue->InPtr, RxQue);
		Com->ReceivingCnt = Com->ReceivingDelay;
	}
}
/*********************************************************************************/
int AvrUartCheckRx(tag_AvrUartCtrl *Com)
{
	/*
		1) 인수
			- Com : tag_AvrUartCtrl 인스턴스의 주소.

		2) 반환
		  - 수신한 데이터의 길이 반환 (Byte)

		3) 설명
			- 수신 버퍼에 저장된 데이터의 길이를 반환함.
	*/

	return Com->RxQueue.Ctr;
}
/*********************************************************************************/
int AvrUartCheckTx(tag_AvrUartCtrl *Com)
{
	/*
		1) 인수
			- Com : tag_AvrUartCtrl 인스턴스의 주소.

		2) 반환
		  - 송신할 데이터의 길이 반환 (Byte)

		3) 설명
			- 송신 버퍼에 저장된 데이터의 길이를 반환함.
	*/

	return Com->TxQueue.Ctr;
}
/*********************************************************************************/
void AvrUartGetChar(tag_AvrUartCtrl *Com, char *Char)
{
	tag_AvrUartRingBuf *RxQue = &Com->RxQueue;

	/*
		1) 인수
			- Com : tag_AvrUartCtrl 인스턴스의 주소.
			- Char : 수신 버퍼의 데이터를 저장할 변수의 주소.

		2) 반환
		  - 없음.

		3) 설명
			- 수신 버퍼의 데이터 1Byte를 읽어 지정한 변수에 저장.
			- 본 함수를 실행하면 수신완료 길이가 1Byte 감소.
	*/

	if(Com->Bit.InitComplete == false)
	{
		return;
	}

	if(RxQue->Ctr)
	{
		RxQue->Ctr--;
		*Char = *RxQue->OutPtr;

		INC_BUF_POINTER(RxQue->OutPtr, RxQue);
	}
}
/*********************************************************************************/
void AvrUartGetData(tag_AvrUartCtrl *Com, char *Buf, int Length)
{
	int i;

	/*
		1) 인수
			- Com : tag_AvrUartCtrl 인스턴스의 주소.
			- Buf : 수신 버퍼의 데이터를 저장할 버퍼의 주소.
			- Length : 읽어올 데이터의 길이 (Byte)

		2) 반환
		  - 없음.

		3) 설명
			- 수신 버퍼의 데이터를 지정한 길이 만큼 읽어 지정한 버퍼에 저장.
			- 본 함수를 실행하면 수신완료 길이가 지정한 길이 만큼 감소.
	*/

	if(Com->Bit.InitComplete == false)
	{
		return;
	}

	for(i = 0; i < Length; i++)
	{
		AvrUartGetChar(Com, Buf++);
	}
}
/*********************************************************************************/
void AvrUartClearQueueBuf(tag_AvrUartRingBuf *Queue)
{
	/*
		1) 인수
			- Queue : tag_AvrUartRingBuf 인스턴스의 주소.

		2) 반환
		  - 없음.

		3) 설명
			- 지정한 RingBuf 초기화.
	*/

	Queue->OutPtr = Queue->InPtr = Queue->Buf;
	Queue->Ctr = 0;
}
/*********************************************************************************/
char AvrUartCheckReceiving(tag_AvrUartCtrl *Com)
{
	/*
		1) 인수
			- Com : tag_AvrUartCtrl 인스턴스의 주소.

		2) 반환
		  - 0 : 수신 대기
			- 1 : 수신 중

		3) 설명
			- 해당 채널이 수신 중인지 확인하여 반환.
	*/

	if(Com->Bit.InitComplete == false)
	{
		return false;
	}

	if(Com->ReceivingCnt) Com->ReceivingCnt--;
	return Com->ReceivingCnt ? true : false;
}
/*********************************************************************************/
void AvrUartFixTxEnableFloating(tag_AvrUartCtrl *Com)
{
	/*
		1) 인수
			- Com : tag_AvrUartCtrl 인스턴스의 주소.

		2) 반환
		  - 없음.

		3) 설명
			- 송신완료 후 TX enable pin이 간헐적으로 disable 되지 않는 현상이 있어 추가.
			- AvrUart 모듈을 사용하며 Slave일 경우 반드시 본 함수를 주기적으로 호출.
	*/

	if(Com->Bit.InitComplete == false)
	{
		return;
	}

	if((Com->Bit.DataSend == false) && (*Com->pEnablePort & (1 << Com->EnablePin)))
	{
		*Com->pEnablePort &= ~(1 << Com->EnablePin);
	}
}
/*********************************************************************************/
