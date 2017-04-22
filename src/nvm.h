/**
* @file nvm.h
* @brief Non Volatile Memory driver.
*
* @author Eli Schneider
*
* @version 0.0.1
* @date 25.01.2010
*
*/
#ifndef _NVM_H
#define _NVM_H

#include "spimem.h"
#include "at25xx0a.h"

extern struct spiMem nvm1;

int nvmSetup(void); 

#endif

