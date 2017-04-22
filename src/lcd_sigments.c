


//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "lcd_sigments.h"
#include "spiLcd.h"
#include "board.h"
#include "CP240x_def.h"





union updateFlag updateUlpMem;



//-----------------------------------------------------------------------------
// Segment constants
//-----------------------------------------------------------------------------
//
// This section changes if the mapping between LCD segment/driver pins changes.
//
// LCD Segment Constants (Bit Mapping)

#define A_seg BIT7
#define B_seg BIT6
#define C_seg BIT5
#define D_seg BIT4
#define E_seg BIT1
#define F_seg BIT3
#define G_seg BIT2




struct sUlpMem ulpMem;
unsigned char infoFlags;



unsigned char ALPHA_ARRAY[] = 
{	
/*A*/	A_seg + B_seg + C_seg + E_seg + F_seg ,   
/*b*/	C_seg + D_seg + E_seg + F_seg + G_seg ,			
/*C*/	A_seg + F_seg + E_seg + D_seg , 
/*d*/	B_seg + C_seg + D_seg + E_seg + G_seg ,
/*E*/	A_seg + D_seg + E_seg + F_seg + G_seg ,
/*F*/	A_seg + E_seg + F_seg + G_seg
};


unsigned char NUMBER_ARRAY[] = 
{
/*0*/	A_seg + B_seg + C_seg + D_seg + E_seg + F_seg,   
/*1*/	B_seg + C_seg,				
/*2*/	A_seg + B_seg + D_seg + E_seg + G_seg,
/*3*/	A_seg + B_seg + C_seg + D_seg + G_seg,
/*4*/	B_seg + C_seg + F_seg + G_seg,
/*5*/	A_seg + C_seg + D_seg + F_seg + G_seg,
/*6*/	A_seg + C_seg + D_seg + E_seg + F_seg + G_seg ,
/*7*/	A_seg + B_seg + C_seg,
/*8*/	A_seg + B_seg + C_seg + D_seg + E_seg + F_seg + G_seg ,
/*9*/	A_seg + B_seg + C_seg + D_seg + F_seg + G_seg,
/*A*/	A_seg + B_seg + C_seg + E_seg + F_seg +G_seg,   
/*b*/	C_seg + D_seg + E_seg + F_seg + G_seg , 		
/*C*/	A_seg + F_seg + E_seg + D_seg , 
/*d*/	B_seg + C_seg + D_seg + E_seg + G_seg ,
/*E*/	A_seg + D_seg + E_seg + F_seg + G_seg ,
/*F*/	A_seg + E_seg + F_seg + G_seg

};



//-----------------------------------------------------------------------------
// LCD Specific Functions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// LCD_OutChar
//-----------------------------------------------------------------------------
//
// Parameter: none
// Return Value: ASCII Character
//
// Prints the character to the LCD
//

unsigned char numToSeg(unsigned char dat)
{
	unsigned char  seg_char=0;
	if(dat >= 0x00 && dat <= 0x0F)
	{
		seg_char = NUMBER_ARRAY[dat];
	}
	else
	{	
      seg_char = 0;	
	}
 return seg_char;
}


unsigned long ulongToBcd(unsigned long n)
{
	unsigned long bcd=0;
	unsigned char sb=0;//=sizeof(bcd)*8;
	
	while (n)
	{
		//sb-=4;
		bcd|=(n%10)<<sb;
		n/=10;
		sb+=4;
	}
	//bcd>>=sb;
	return bcd;	
}

unsigned int uintToBcd(unsigned  int number)
{
	unsigned int bcd=0;
	unsigned char sb=0;//=sizeof(bcd)*8;
	
	while (number)
	{
		//sb-=4;
		bcd|=(number%10)<<sb;
		number/=10;
		sb+=4;
	}
	//bcd>>=sb;
	return bcd;	
}



void sendDisplay(void)
{

	
	if (updateFlow())
	{
		if(updateCounter())
		{
			updateArrow();
			CP240x_RegBlockWrite(ULPMEM00,(unsigned char *)&ulpMem,13); //apdate counter+flow+arrows
		}
		else
		{
			if(updateArrow())
				CP240x_RegBlockWrite(ULPMEM00,(unsigned char *)&ulpMem,11); //apdate flow+arrows
			else
				CP240x_RegBlockWrite(ULPMEM00,(unsigned char *)&ulpMem,6); //apdate flow only
		}

	}


	else if (updateCounter())
	{
		if(updateArrow())
			CP240x_RegBlockWrite(ULPMEM06,(unsigned char *)&ulpMem.ulp6,7); //apdate counter+arrows
		else
			CP240x_RegBlockWrite(ULPMEM07,(unsigned char *)&ulpMem.ulp7,6); //apdate counter only
	}
	else if (updateArrow())
	{
		CP240x_RegBlockWrite(ULPMEM06,(unsigned char *)&ulpMem.ulp6,5); //apdate arrows only
	}



}




/**
* @fn void updateArrow(void)
*
* @brief This function turn on lcd arrow segments acording to quad state and defined resulution "ARROW_RES"
*   to save cpu time, use only if calcCount()=1 (quad changed) 
* @param - none
*
* @author Shlomi Dar
*
* @date 14.06.2011
*/


unsigned char updateArrow(void)
{
	unsigned char newArrowQuad;

	newArrowQuad=(adcBuffer.counter>>ARROW_RES)&0x03;

	if (newArrowQuad==adcBuffer.arrowQuad)
		return 0;
	
	adcBuffer.arrowQuad=newArrowQuad;
	
	DOWN_ARROW(OFF);
	LEFT_ARROW(OFF);
	UP_ARROW(OFF);
	RIGHT_ARROW(OFF);

	

	switch (newArrowQuad)
	{
		case 0:
			DOWN_ARROW(ON);
			break;
		case 1:
			LEFT_ARROW(ON);
			break;
		case 2:
			UP_ARROW(ON);
			break;
		case 3:
			RIGHT_ARROW(ON);
			break;	
	}

	#ifdef UPDATE_ULPS
	updateUlpMem.ulp.all=ARROW_ULPS;
	#endif
	return 1;
}


/**
* @fn Void updateFlow(void)
*
* @brief This function turn on lcd arrow segments acording to quad state and defined resulution "ARROW_RES"
*   
* @param - none
*
* @author Shlomi Dar
*
* @date 14.06.2011
*/

unsigned char updateFlow(void)
{
	union ulpReg *address;
	unsigned char ulpTemp1;
	unsigned char ulpTemp2;
	unsigned long bcdCounter;
	unsigned char segNum;
	unsigned char lastSegNum=0;



	if (adcBuffer.dispFlow==adcBuffer.oldDispFlow)
		return 0;

	if (adcBuffer.dispFlow>0&&adcBuffer.oldDispFlow>0)
	{
		LEAK_DISP(OFF);
	}
	else if (adcBuffer.dispFlow<0)
	{
		adcBuffer.dispFlow=-adcBuffer.dispFlow;
		LEAK_DISP(ON);
	}
	
	adcBuffer.oldDispFlow=adcBuffer.dispFlow;
	
	
	
	//save flags segmenet
	ulpTemp1=ulpMem.ulp0.all;
	ulpTemp2=ulpMem.ulp5.all;
	
	//update numbers
	address=&ulpMem.ulp5;
	bcdCounter=ulongToBcd(adcBuffer.dispFlow); //convert long to bcd1
	while ((address+1)>&(ulpMem.ulp1))
	{
		segNum=numToSeg((bcdCounter&0x0F));
		(*address).all&=BIT4;// all but flags segments are zero 
		(*address).all|=(segNum>>4); //apdate segments, flip MSB with LSB)
		(*address).all|=(lastSegNum<<4);
		bcdCounter=(bcdCounter>>4);//next number
		address--;
		lastSegNum=segNum;
	}
	(*address).all|=(lastSegNum<<4);
	
	//restore flags
	//ulpMem.ulp0.all&=~(BIT0|BIT1|BIT2|BIT3|BIT4);
	//ulpMem.ulp0.all|=ulpTemp1&(BIT0|BIT1|BIT2|BIT3|BIT4);
	ulpMem.ulp5.all|=ulpTemp2&(BIT5|BIT6|BIT7);

	return 1;
}




/**
* @fn updateCounter(void)
*
* @brief This function turn on lcd arrow segments acording to quad state and defined resulution "ARROW_RES"
*  
* @param - none
*
* @author Shlomi Dar
*
* @date 14.06.2011
*/

unsigned char updateCounter(void)
{

	union ulpReg *address;
	unsigned long bcdCounter;
	unsigned char segNum;

	if (adcBuffer.oldDispCounter==adcBuffer.dispCounter)
		return 0;

	adcBuffer.oldDispCounter=adcBuffer.dispCounter;


	/*display main counter*/
	
	bcdCounter=ulongToBcd(adcBuffer.dispCounter); //convert long to bcd
	
	segNum=numToSeg((bcdCounter&0x0F));
	ulpMem.ulp7.all&=BIT4;
	ulpMem.ulp7.all|=(segNum<<4)|(segNum>>4);
	bcdCounter=(bcdCounter>>4);

	address=&ulpMem.ulp12; //start address
	while (address>&(ulpMem.ulp7))
	{
		segNum=numToSeg((bcdCounter&0x0F));
		(*address).all&=0x01;
		(*address).all|=segNum;
		bcdCounter=(bcdCounter>>4);
		address--;
	}

	return 1;
}

