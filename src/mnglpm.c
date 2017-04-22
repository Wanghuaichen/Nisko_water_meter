#include "mnglpm.h"

unsigned char PowerDownLock[POWER_DOWN_MODES+1];


/************************* MaxPDLevel***********************
Name			  MaxPDLevel
Input:			  Requested Power down level
Called by:		  main()
Call to:		  ---
Returns:		  ---

Description:	  This function calculates lowest enable level of low power mode
				  
					
***********************************************************/

unsigned char MaxPDLevel(unsigned char RequestedPDLevel)
{
	unsigned char idx;
	unsigned char test_idx;
	if (MAX_ENABLED_PD_MODE<RequestedPDLevel)
		RequestedPDLevel=MAX_ENABLED_PD_MODE;
	for (idx=0;idx<=POWER_DOWN_MODES;idx++)
	{
		test_idx=PowerDownLock[idx];
		if (PowerDownLock[idx])  
			break;
	}
	if (idx<(POWER_DOWN_MODES+1))
	{
		if (idx==1)
			idx=0;
		else if (RequestedPDLevel<idx)
			idx=RequestedPDLevel;
	}
	else
		idx=RequestedPDLevel;

	return idx;
	
}
/************************* LockPD***********************
Name			  LockPD
Input:			  Requested for lock level, Mask for this level
Called by:		  main()
Call to:		  ---
Returns:		  ---

Description:	  This function locks power down level
				  
					
***********************************************************/
void LockPD(unsigned char Level, unsigned char Mask)
{
	if (Level<(POWER_DOWN_MODES+1))
		PowerDownLock[Level] |= Mask;
}
/************************* UnlockPD***********************
Name			  UnlockPD
Input:			  Requested for lock level, Mask for this level
Called by:		  main()
Call to:		  ---
Returns:		  ---

Description:	  This function unlocks power down level
				  
					
***********************************************************/

void UnlockPD(unsigned char Level, unsigned char Mask)
{
	if (Level<(POWER_DOWN_MODES+1))
		PowerDownLock[Level] &= ~Mask;
}

void UnlockAllPD(unsigned char Mask)
{
	unsigned char idx;

	for (idx=0;idx<(POWER_DOWN_MODES+1);idx++)
		(PowerDownLock[idx]) &= ~Mask;
}

