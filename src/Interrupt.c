#include "interrupt.h"
#include "mnglpm.h"
#include "serial.h"



void USART0TX_VECTOR_code( void )
{	
	#ifdef USE_SERIAL
	if (SerialTxIsr())
		_BIC_SR_IRQ( 0x00f0 );						//Return from int to HS
	#endif
}

void USART0RX_VECTOR_code( void )
{
	#ifdef USE_SERIAL
	if (SerialRxIsr())
		_BIC_SR_IRQ( 0x00f0 );						//Return from int to HS
	#endif
}


