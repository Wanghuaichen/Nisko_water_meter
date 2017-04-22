/**
* @file at25xx0a.h
* @brief Atmel AT25xx0a  SPI EEPROM driver.
*
* @author Eli Schneider
*
* @version 0.0.1
* @date 24.01.2010
*
*/
#ifndef _AT25XX0A_H
#define _AT25XX0A_H

#include "spimem.h"

int at25xx0Setup(struct spiMem *m, struct spiSlave *spi); 
/*static*/ int at25xx0WaitReady(struct spiMem *mem, uint32_t timeout, void *fIdle, void *idleArg);

#endif
