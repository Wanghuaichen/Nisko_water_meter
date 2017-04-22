#include "board.h"
#include "dco.h"


/*
** Calibrates the DCO from previous set point
*/
#define DELTA_LB (DCO_FRQ/(ACLK_FRQ/ACLK_DIV))
#define DELTA_UB (DELTA_LB+((DCO_FRQ%(ACLK_FRQ/ACLK_DIV))!=0))
#define ALLOWABLE_DEVIATION	1
#define DELTA_LOW_WATERMARK (DELTA_LB-ALLOWABLE_DEVIATION)
#define DELTA_HIGH_WATERMARK (DELTA_UB+ALLOWABLE_DEVIATION)
#ifdef DEBUG_DCO
unsigned int trace_count=0;
#endif

void Calibrate_DCO (void)
{
	unsigned int Compare, Oldcapture = 0;
	unsigned int count=0;
	
	/* For debug */
	//P5OUT |= 0x08;
	/* end of debug */

	CCTL2 = CCIS0 + CM0 + CAP;            // Define CCR2, CAP, ACLK
	TACTL = TASSEL1 + TACLR + MC1;        // SMCLK, continous mode

    while ((CCTL2 & CCIFG) != CCIFG);   // Wait until capture occured!
    CCTL2 &= ~CCIFG;                    // Capture occured, clear flag
    Oldcapture=CCR2;
	for (count=1;;count++)
	{
    	while ((CCTL2 & CCIFG) != CCIFG);   // Wait until capture occured!
    	CCTL2 &= ~CCIFG;                    // Capture occured, clear flag
	    Compare = CCR2;                     // Get current captured SMCLK
		Compare = Compare - Oldcapture;     // SMCLK difference
		Oldcapture = CCR2;                  // Save current captured SMCLK
		if (DELTA_HIGH_WATERMARK < Compare)           // DCO is too fast, slow it down
		{
			DCOCTL--;
			if (DCOCTL == 0xFF)               // Did DCO role under?
				BCSCTL1--;                      // Select next lower RSEL
		}
		else if (Compare < DELTA_LOW_WATERMARK)
		{
			DCOCTL++;
			if (DCOCTL == 0x00)               // Did DCO role over?
				BCSCTL1++;                      // Select next higher RSEL
		}
		else
		{
		   	/* For debug */

		   	/* end of debug */
			break;                            // if in range, leave "while(1)"
		}

	}
	#ifdef DEBUG_DCO
	trace_count=count;
	#endif
	CCTL2 = 0;                              // Stop CCR2 function
	TACTL = 0;                              // Stop Timer_A
}

