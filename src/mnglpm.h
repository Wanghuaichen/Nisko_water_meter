#ifndef __MNGLPM_H
#define	__MNGLPM_H

#include <string.h>


#define PD_MASK(x)	(1<<(x))

#define PD_ADC			0
#define PD_USART		1
#define PD_SPI			2
#define PD_LCD			3


/*
** Power down mode selector table;
*/
#define POWER_DOWN_MODES      5 
#define POWER_ACTIVE 0
extern unsigned char PowerDownLock[POWER_DOWN_MODES+1];

#define PD_LPM(x) ((x)+1)
#define PD_LPM_ACTIVE	3

#define	MAX_ENABLED_PD_MODE PD_LPM(3)

#define InitLPMng() memset(PowerDownLock,0,sizeof(PowerDownLock))



unsigned char MaxPDLevel(unsigned char RequestedPDLevel);

void LockPD(unsigned char Level, unsigned char Mask);
void UnlockPD(unsigned char Level, unsigned char Mask);
void UnlockAllPD(unsigned char Mask);
#endif

