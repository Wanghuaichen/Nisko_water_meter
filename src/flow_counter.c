


#include <math.h>
#include "Flow_counter.h"
#include "board.h"
#include "lcd_sigments.h"




#define HYSTERESIS (0x3ff/20)
#define NEG ((0x3FF/2)-HYSTERESIS)
#define POS ((0x3FF/2)+HYSTERESIS)
unsigned char calcCount(void)
{
	unsigned int newQuad=0;
	unsigned int oldQuad;
	unsigned char sin=0;
	unsigned char cos=0;	
	unsigned char state=0;

	
	adcBuffer.oldQuad=(adcBuffer.counter)&0x03;
	


	/*find A0 quadrant*/
	
	if (adcBuffer.A0>POS) // 	> (6/10)
	{
		sin= 0x01;
		adcBuffer.oldSin=sin;
	}
	else if (adcBuffer.A0>NEG) // 	>(4/10)
	{
		sin=adcBuffer.oldSin; //hysteresis
	}
	else
		adcBuffer.oldSin=0x00;
	
	
	/*find A1 quadrant*/	
	if (adcBuffer.A1>POS) // 	> (6/10)
	{
		cos= 0x01;
		adcBuffer.oldCos=cos;
		
	}
	else if (adcBuffer.A1>NEG) // 	>(4/10)
	{
		cos=adcBuffer.oldCos; //hysteresis
	}
	else
		adcBuffer.oldCos=0x00;
	
	/*encoded quadrant*/
	state=sin+(cos<<1);
	switch (state)
	{
		case 1:
			adcBuffer.newQuad=1;
			break;
		case 2:
			adcBuffer.newQuad=3;
			break;
		case 3:
			adcBuffer.newQuad=2;
			break;
		default:
			adcBuffer.newQuad=0;
			break;				
	}
	
	/*calc counter*/
	if (adcBuffer.newQuad==adcBuffer.oldQuad)
		return 0; //no counter or LCD update
	else if ((adcBuffer.oldQuad==3)&&(adcBuffer.newQuad==0))
	{
		(adcBuffer.CWcounter)++;
		(adcBuffer.counter)++;
		adcBuffer.flow++;
	}
	else if (adcBuffer.oldQuad==0&&adcBuffer.newQuad==3)
	{
		(adcBuffer.CCWcounter)--;
		(adcBuffer.counter)--;
		adcBuffer.flow--;
	}
	else
	{
		adcBuffer.counter&=~(0x0003);
		adcBuffer.counter|=adcBuffer.newQuad;
		if((adcBuffer.newQuad&0x01)==(adcBuffer.oldQuad&0x01))
			ERR_DISP(ON);
			
	}

	/*update Gal counter*/
	if (adcBuffer.counter>GAL_MOD)
	{
		adcBuffer.galCounter++;
		adcBuffer.counter-=GAL_MOD;
	}
	else if (adcBuffer.counter<(-GAL_MOD))
	{
		adcBuffer.galCounter--;
		adcBuffer.counter+=GAL_MOD	;
	}


	if (adcBuffer.galCounter>=0)
		adcBuffer.dispCounter=adcBuffer.galCounter;
	else if (adcBuffer.galCounter>-1000000)
		adcBuffer.dispCounter=(unsigned long)(1000000)+(adcBuffer.galCounter);
	else
		adcBuffer.counter=adcBuffer.galCounter=0;
	
	return 1; //the counter was updated
}




/**********ADC DTC conversion **********
magnetometer sensor set time - 5us
sample time 4 ADC clk
conversation time 13 ADC clk

***************************************/
void sampleWaterMeter(void)
{
	P1OUT |= BIT5+BIT6;	//TP16+TP17 ON

	__delay_cycles(2*DCO_FRQ/1000000); // 2usec dealy

	
	/**********start ADC DTC conversion 3.3usec***********/
	ADC10CTL0 &= ~ENC;
    while (ADC10CTL1 & BUSY);    // Wait if ADC10 core is active
  	ADC10SA =(unsigned int)&adcBuffer;  // Data buffer start
   	ADC10CTL0 |= ENC + ADC10SC + ADC10IE;// Sampling and conversion start
	/*************************************************/
	__delay_cycles(21); // S&H (4 clk) +conv (13 clk)+ S&H (4 clk)=
	P1OUT &=~(BIT5+BIT6); //TP16+TP17 OFF
}




void timeStepCalc(void)
{
	long delta;
	unsigned int counter;
	int direction;

	counter=TAR;
	

	delta=counter-adcBuffer.timerStamp;
		
	if (delta>0x7FFF)
	{
		direction=1; //CCW
		delta=0xFFFF-delta;
	}
	else if (-delta>0x7FFF)
	{
		direction=0; //CW
		delta=0xFFFF+delta;
	}
	else if (delta<0)
	{
		direction=1; //CCW
		delta=-delta;
	}
	else 
		direction=0; //CW

	
	adcBuffer.timerStamp=counter;
	
	adcBuffer.delta=(unsigned int)delta>>ACC_CONST;



	
}

void stallCheck(void)
{
	unsigned int delta;
	unsigned int counter;

	counter=TAR;
	
	delta=abs(counter-adcBuffer.timerStamp);
	if(delta<0x3FFF)
		return;

	adcBuffer.timerStamp=counter+0xc000; 
	adcBuffer.delta=TA_MAX_COUNTS;

}
