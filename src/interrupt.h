#ifndef __INTERRUPT_H
#define __INTERRUPT_H

#include "board.h"

#pragma interrupt_handler PORT1_VECTOR_code:PORT1_VECTOR
#pragma interrupt_handler PORT2_VECTOR_code:PORT2_VECTOR
#pragma interrupt_handler ADC12_VECTOR_code:ADC_VECTOR
#pragma interrupt_handler TIMERA1_VECTOR_code:TIMERA1_VECTOR
#pragma interrupt_handler TIMERA0_VECTOR_code:TIMERA0_VECTOR
#pragma interrupt_handler WDT_VECTOR_code:WDT_VECTOR
#pragma interrupt_handler NMI_VECTOR_code:NMI_VECTOR
#pragma interrupt_handler USART0TX_VECTOR_code:USART0TX_VECTOR
#pragma interrupt_handler USART0RX_VECTOR_code:USART0RX_VECTOR
#pragma interrupt_handler TIMERB1_VECTOR_code:TIMERB1_VECTOR
#pragma interrupt_handler TIMERB0_VECTOR_code:TIMERB0_VECTOR
#pragma interrupt_handler COMPARATORA_code:COMPARATORA_VECTOR
 
#endif

