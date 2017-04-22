#include <stdio.h>
#include <msp430.h>

#include "Serial.h"
#include "mnglpm.h"
#include "spiLcd.h"
#include "lcd_sigments.h"
#include "CP240x_def.h"

//#include "msp430f2132.h"
//#include "board.h"

//define u0ctl bits

#define URXSE BIT3
#define SPB BIT5
#define CHAR BIT4
#define SWRST BIT0

unsigned char TxBuf[TX_BUF_SIZE_0];
unsigned short RxBuf[RX_BUF_SIZE_0];
unsigned char inBuf[RX_BUF_SIZE_0];


unsigned char TxCnt;
unsigned char TxInIdx;
unsigned char TxOutIdx;
unsigned char RxCnt;
unsigned char RxInIdx;
unsigned char RxOutIdx;
unsigned char RxInProgress;
unsigned char TxErr=0;
unsigned char crCnt=0;
unsigned char protocol_state=0;





/************************* Initialization Serial communication UARTX ***********************
Name              SerialInit
Input:            channel,baud_div,Mod
Called by:        All
Call to:          ---
Returns:          ---

Description:       Initialization Serial communication .
				  
***********************************************************/



void SerialInit(unsigned short baud_div)
{
	RxInProgress=0;
	TxCnt=TxInIdx=TxOutIdx=RxCnt=RxInIdx=RxOutIdx=0;
	// Set baud
	
	UCA0CTL1|=SWRST; //software reset
	UCA0BR0=baud_div&0xff; 
	UCA0BR1=baud_div>>8;
	UCA0MCTL=0; //mod
	UCA0CTL0=0;// stop bit = 1, char len =8 //U0CTL=SPB|CHAR|SWRST; //stop bit = 2, char len =8, reset
	UCA0CTL1=UCSSEL_2 /*| URXSE*/ ; //smclk
	UCA0STAT=0;//U0RCTL=0; //status
	//ME1=UTXE0/*|URXE0*/; module enable
	UCA0CTL1&= ~SWRST; 
	IE2 &= ~ UCA0TXIE;
	IE2|=UCA0RXIE;
	
	
	#ifdef KUKU
	P3SEL = 0x30;                             // P3.4,5 = USCI_A0 TXD/RXD
	UCA0CTL1 |= UCSSEL_2;                     // SMCLK
	UCA0BR0 = 768;                              // 1MHz 115200
	UCA0BR1 = 0;                              // 1MHz 115200
	UCA0MCTL = 0;               // Modulation UCBRSx = 0
	UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
	IE2 |= UCA0RXIE;                          // Enable USCI_A0 RX interrupt
	#endif
#ifdef KUKU
	UCA0CTL0=UCSSEL_2 //smclk
	UCA0BR0=baud_div&0xff;
	UCA0BR1=baud_div>>8;
	UCA0MCTL=Mod;
	IE2=UCA0RXIE|UCA0TXIE // enable interrupt
#endif
	
}




/************************* SerialWrite ***********************
Name              SerialWrite
Input:            transmit channel , transmit char 
Called by:        All
Call to:          ---
Returns:          ---

Description:       Write data to UARTX  
		  
***********************************************************/

void SerialWrite(unsigned char ch)
{

	/*see if circ buffer isnt full*/
	while (1) 
	{
		__disable_interrupt();

		if (TxCnt<sizeof(TxBuf)) //if buffer isnt full
		{
			__enable_interrupt();
			break;
		}
		else if (sizeof(TxBuf)<TxCnt) //buffer full, err, clear tx counter
		{
			TxErr++;
			TxInIdx=TxOutIdx=TxCnt=0;
			__enable_interrupt();
			break;
		}
		else
			__enable_interrupt();
	}

		__disable_interrupt();



	/*send char to buffer*/
	TxBuf[TxInIdx]=ch; //insert char to circ buf
	if ((TxInIdx+1)<sizeof(TxBuf)) 
		TxInIdx++; //point to next empty space in buffer 
	else
		TxInIdx=0; //circ buf return to zero at the end
	TxCnt++; //increment number of chars in buffer



	/*activate uart isr*/
	if (IE2 & UCA0TXIE)//The interrupt is enabled
	{
		__enable_interrupt(); 
	}
	else if (TxCnt) //if there is chars in buffer
	{
		/*
		** The interrupt is disabled, there is at least 1 character to transmit in the buffer
		*/
		UCA0TXBUF=TxBuf[TxOutIdx];  //send char to txbuf
		TxCnt--; //decrement number of chars in buffer
		
		if ((TxOutIdx+1)<sizeof(TxBuf))
			TxOutIdx++; //point to next char to send
		else
			TxOutIdx=0; //circ buf return to zero at the end
		IE2 |=UCA0TXIE; //enable TX int
		__enable_interrupt();
	}
	else
		__enable_interrupt();




}

/************************* SerialTxIsr0 ***********************
Name              SerialTxIsr0
Input:            ---
Called by:        interrupt
Call to:          ---
Returns:          transmit status

Description:       UART 0 transmit interrupt 
		  
***********************************************************/

unsigned char SerialTxIsr(void)
{
	if (TxCnt) //at least one char in buffer
	{
		if (TxCnt<=sizeof(TxBuf)) //number of chars is less then size of TxBuf
		{
			UCA0TXBUF=TxBuf[TxOutIdx]; //send next char to TXbuf
			if ((TxOutIdx+1)<sizeof(TxBuf))
				TxOutIdx++; //point to next char  to send
			else
				TxOutIdx=0; //circ buff return to zero
			TxCnt--; //decrement  number of chars in buffer
		}
		else //number of chars must be less then size of TxBuf
		{
			TxCnt=TxInIdx=TxOutIdx=0;
			UC0IE&= ~UCA0TXIE;
			return 1;			// exit low power mode
		}
		return 0;
	}
	else //no chars in buffer
	{
		IE2&= ~UCA0TXIE; //disble int
		return 1;			// exit low power mode
	}
}


/************************* SerialRxIsr0 ***********************
Name              SerialRxIsr0
Input:            ---
Called by:        interrupt
Call to:          ---
Returns:          transmit status

Description:       UART 0 receive interrupt 
		  
***********************************************************/
unsigned char SerialRxIsr(void)
{

	register unsigned short tmp;


	tmp=(UCA0STAT<<8);
	tmp |= UCA0RXBUF;
	if (UCA0RXBUF=='\r')
		crCnt++;

	if (RxCnt<(sizeof(RxBuf)/sizeof(RxBuf[0]))) //buffer not full
	{
		RxBuf[RxInIdx]=tmp; //send to buffer
		if ((RxInIdx+1)<(sizeof(RxBuf)/sizeof(RxBuf[0]))) //test end of buffer
			RxInIdx++; 
		else
			RxInIdx=0; //circ buf return to zero at the end
		RxCnt++; //increment rx counter
	}
	RxInProgress=0;

	return 1;

}


unsigned short SerialRead(void)
{
	unsigned short ch;
	while(!RxCnt);
	ch=RxBuf[RxOutIdx]; // RxOutIdx pointing to first in char
	if ((RxOutIdx+1)<(sizeof(RxBuf)/sizeof(RxBuf[0]))) //inc buff pointer to next char
		RxOutIdx++;
	else
		RxOutIdx=0;
	RxCnt--;
	return ch;
}


void PrintString(char *s)
{
	while (*s)
	{
		SerialWrite(*s);
		s++;
	}
}

unsigned char SerialRxReady(void)
{
	return (RxCnt!=0);
}

unsigned char readStringToBuf()
{
	unsigned char i=0;

	if (SerialRxReady())
	{
		memset(&inBuf,0x00,sizeof(inBuf));
		while (RxCnt)
		{
			inBuf[i]=(unsigned char)(0x00FF&(SerialRead()));
			if (inBuf[i]=='\r')
			{
				crCnt--;
				__disable_interrupt();
				if (RxCnt==0)
					crCnt=0;
				__enable_interrupt(); 
				return 1;
			}
			i++;
		}
	}
		
	__disable_interrupt();
	if (RxCnt==0)
		crCnt=0;
	__enable_interrupt(); 

	return 0; //err no CR
}


void PrintHexDigit(unsigned char digit)
{
	digit=(digit<10) ? digit + '0' : digit + ('A'-10);
	SerialWrite(digit);
}

void PrintHexLong(unsigned long val)
{
	union {
		unsigned char c[5];
		unsigned long s;
		} tmp;

	tmp.s=val;
	//PrintHexDigit(tmp.c[2]>>4);
	PrintHexDigit(tmp.c[2]&0xf);
	PrintHexDigit(tmp.c[1]>>4);
	PrintHexDigit(tmp.c[1]&0xf);
	PrintHexDigit(tmp.c[0]>>4);
	PrintHexDigit(tmp.c[0]&0xf);
}

void PrintHexInt(unsigned int val)
{
	union {
		unsigned char c[2];
		unsigned long s;
		} tmp;

	tmp.s=val;
	PrintHexDigit(tmp.c[1]>>4);
	PrintHexDigit(tmp.c[1]&0xf);
	PrintHexDigit(tmp.c[0]>>4);
	PrintHexDigit(tmp.c[0]&0xf);
}

void IntToHex(unsigned int val,unsigned char *data)
{
	union {
		unsigned char c[2];
		unsigned long s;
		} tmp;

	tmp.s=val;
	data[3]=tmp.c[1]>>4;
	data[2]=tmp.c[1]&0xf;
	data[1]=tmp.c[0]>>4;
	data[0]=tmp.c[0]&0xf;

}

void longToHex(unsigned int val,unsigned char *data)
{
	union {
		unsigned char c[3];
		unsigned long s;
		} tmp;

	tmp.s=val;

	data[4]=tmp.c[2]&0xf;
	data[3]=tmp.c[1]>>4;
	data[2]=tmp.c[1]&0xf;
	data[1]=tmp.c[0]>>4;
	data[0]=tmp.c[0]&0xf;
}


