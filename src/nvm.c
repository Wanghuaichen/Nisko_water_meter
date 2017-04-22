/**
* @file nvm.c
* @brief Non Volatile Memory driver.
*
* @author Eli Schneider
*
* @version 0.0.1
* @date 25.01.2010
*
*/
#include "board.h"
#include "nvm.h"
#include "at25xx0a.h"


struct spiSlave nvmSpiSlave1;
struct spiMem nvm1;


/**
* @fn int nvmSetup(void)
*
* @brief This function initializes NVM interface
* 
* @return 0=success, -1= failure
*
* @author Eli Schneider
*
* @date 25.02.2010
*/
int nvmSetup(void)
{
	nvmSpiSlave1.cs=EE_CS;
	nvmSpiSlave1.mode.all=SPI_CS_HOLD+SPI_PHASE_IN+(8-1);
	nvmSpiSlave1.fClock=10000000;
	
	return at25xx0Setup(&nvm1, &nvmSpiSlave1);
}


