//lcd_sigments.h


#ifndef _LCD_SIGMENTS_H
#define _LCD_SIGMENTS_H


extern union updateFlag updateUlpMem;

#define ON 1
#define OFF 0
#define ARROW_RES 0
unsigned char numToSeg(unsigned char dat);


unsigned long ulongToBcd(unsigned long n);
unsigned int uintToBcd(unsigned  int number);

void sendDisplay(void);
unsigned char updateArrow(void);
unsigned char updateFlow(void);
unsigned char updateCounter(void);




struct updeteSigFlag
{
	unsigned int ulp0:1;
	unsigned int ulp1:1;
	unsigned int ulp2:1;
	unsigned int ulp3:1;
	unsigned int ulp4:1;
	unsigned int ulp5:1;
	unsigned int ulp6:1;
	unsigned int ulp7:1;
	unsigned int ulp8:1;
	unsigned int ulp9:1;
	unsigned int ulp10:1;
	unsigned int ulp11:1;
	unsigned int ulp12:1;
	unsigned int ulp13:1;
	unsigned int ulp14:1;
	unsigned int ulp15:1;
};

union updateFlag
{
unsigned int all;
struct updeteSigFlag ulp; 
};


struct ulpChar
{
	unsigned char bit0:1;
	unsigned char bit1:1;
	unsigned char bit2:1;
	unsigned char bit3:1;
	unsigned char bit4:1;
	unsigned char bit5:1;
	unsigned char bit6:1;
	unsigned char bit7:1;
};


union ulpReg
{
unsigned char all;
struct ulpChar ulpBit; 
};



struct sUlpMem
{
union ulpReg ulp0;
union ulpReg ulp1;
union ulpReg ulp2;
union ulpReg ulp3;
union ulpReg ulp4;
union ulpReg ulp5;
union ulpReg ulp6;
union ulpReg ulp7;
union ulpReg ulp8;
union ulpReg ulp9;
union ulpReg ulp10;
union ulpReg ulp11;
union ulpReg ulp12;
union ulpReg ulp13;
union ulpReg ulp14;
union ulpReg ulp15;
};

struct sFlag{
union ulpReg* address;
unsigned char mask;
unsigned char val;
};

struct sFlags
{
unsigned char all;
unsigned char leak:1;
unsigned char Rf:1;
unsigned char LowBatt:1;
unsigned char  Err:1;
};


extern struct sUlpMem ulpMem;
extern unsigned char infoFlags;




/**********************LCD Sigments*******************
******************************************************/


/**********************arrows*************************/
#define ARROW_CENTER(x) (ulpMem.ulp11.ulpBit.bit0=x)
#define T1_ARROW_CIRCLE(x) (ulpMem.ulp0.ulpBit.bit4=x)
#define LEFT_ARROW(x) (ulpMem.ulp9.ulpBit.bit0=x)
#define RIGHT_ARROW(x) (ulpMem.ulp6.ulpBit.bit0=x)
#define UP_ARROW(x) (ulpMem.ulp10.ulpBit.bit0=x)
#define DOWN_ARROW(x) (ulpMem.ulp8.ulpBit.bit0=x)
/**********************flow***************************/
#define LI_MIN(x) (ulpMem.ulp5.ulpBit.bit7=x)
#define GAL_MIN(x) (ulpMem.ulp5.ulpBit.bit6=x)
#define FT3_MIN(x) (ulpMem.ulp5.ulpBit.bit5=x)

#define M3(x) (ulpMem.ulp6.ulpBit.bit3=x)
#define GAL(x) (ulpMem.ulp6.ulpBit.bit2=x)
#define FT3(x) (ulpMem.ulp6.ulpBit.bit1=x)
/**********************info**************************/
#define ERR_DISP(x) (ulpMem.ulp0.ulpBit.bit3=x)
#define RF_DISP(x) (ulpMem.ulp0.ulpBit.bit2=x)
#define LOW_BAT_DISP(x) (ulpMem.ulp0.ulpBit.bit1=x)
#define LEAK_DISP(x) (ulpMem.ulp0.ulpBit.bit0=x)




//#define LI_MIN(x) (ulpMem.ulp5.ulpBit.bit7=x;updateUlpMem.ulp.ulp5=1)

#define ARROW_ULPS (BIT6|BIT8|BIT9|BITA)
#define COUNTER_ULPS (BIT7|BIT8|BIT9|BITA|BITB|BITC)
#define FLOW_ULPS (BIT1|BIT2|BIT3|BIT4|BIT5)




#endif

