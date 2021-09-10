/*********************************************************************************/
/*
 * Author : Jung Hyun Gu
 * File name : AvrUart.h
*/
/*********************************************************************************/
#ifndef __AVR_UART_H__
#define	__AVR_UART_H__
/*********************************************************************************/
#define AVR_UART_REVISION_DATE		20161117
/*********************************************************************************/
/** REVISION HISTORY **/
/*
	2016. 11. 17.					- 'AVR_REGISTER' 타입 삭제 -> 'char *' 타입으로 변경.
	Jung Hyun Gu

	2016. 11. 08.					- 변수명 변경. AvrUartLinkRegister() 함수의 인수 'pTxPort' -> 'pEnablePort'
	Jung Hyun Gu					- 변수명 변경. AvrUartLinkRegister() 함수의 인수 'TxPin' -> 'EnablePin'
												- revision valid check 추가.

	2016. 11. 02.					- 변수명 변경. 'ReceivingDelay_us' -> 'ReceivingDelay'
	Jung Hyun Gu

	2016. 10. 28.					- 초기버전.
	Jung Hyun Gu
*/
/*********************************************************************************/
/**Define**/

#define	false				0
#define	true				1
#define null				0

/*********************************************************************************/
/**Enum**/

typedef enum
{
	AVR_UART_FORWARD = 0,
	AVR_UART_BACKWARD,
}enum_AvrUartMoveDirection;
/*********************************************************************************/
/**Struct**/

typedef struct
{
	char *Buf;
	char *InPtr;
	char *OutPtr;
	int Ctr;
	int Size;
}tag_AvrUartRingBuf;

typedef struct
{
	struct
	{
		char InitRegister			:			1;
		char InitBuffer				:			1;
		char InitGeneral			:			1;
		char InitComplete			:			1;
		
		char DataSend					:			1;
	}Bit;
	
	char *pUDR;
	char *pUCSRA;
	char *pEnablePort;
	char EnablePin;
	
	long ReceivingDelay;
	long ReceivingCnt;
	
	tag_AvrUartRingBuf TxQueue;
	tag_AvrUartRingBuf RxQueue;
}tag_AvrUartCtrl;

/*********************************************************************************/
/**Function**/

char AvrUartLinkRegister(tag_AvrUartCtrl *Com, char *pUDR, char *pUCSRA, char *pEnablePort, char EnablePin);
char AvrUartLinkBuffer(tag_AvrUartCtrl *Com, char *TxBuf, int TxBufSize, char *RxBuf, int RxBufSize);
char AvrUartGeneralInit(tag_AvrUartCtrl *Com);


void AvrUartPutData(tag_AvrUartCtrl *Com, char *Buf, int Length);
void AvrUartPutChar(tag_AvrUartCtrl *Com, char Char);


void AvrUartTxQueueControl(tag_AvrUartCtrl *Com);
void AvrUartRxQueueControl(tag_AvrUartCtrl *Com);

void AvrUartStartTx(tag_AvrUartCtrl *Com);
int AvrUartCheckTx(tag_AvrUartCtrl *Com);
int AvrUartCheckRx(tag_AvrUartCtrl *Com);

void AvrUartGetChar(tag_AvrUartCtrl *Com, char *Char);
void AvrUartGetData(tag_AvrUartCtrl *Com, char *Buf, int BufSize);


void AvrUartClearRx(tag_AvrUartCtrl *Com);
char AvrUartViewRxBuf(tag_AvrUartCtrl *Com, int Move, enum_AvrUartMoveDirection Direction);
char AvrUartCheckReceiving(tag_AvrUartCtrl *Com);



/*********************************************************************************/
#endif //__AVR_UART_H__











