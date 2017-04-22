/**
* @file systicks.c
* @brief  system ticker interface.

*
* @author Shlomi Dar
*
* @version 0.0.1
* @date 25.02.2010
*
*/

#include <stdint.h>
#include "systicks.h"
#include "board.h"

/**
* @fn void getIsrTicks(void)
*
@brief this  function  atomically retrieves  IsrTicker value..
*
* @author Shlomi Dar
*
* @date 25.02.2010
*/
uint16_t getIsrTicks(void)
{
	uint16_t ticks;
	__disable_interrupt();
	ticks=TAR;//virtualTimer1;
	__enable_interrupt();
	return ticks;
}
