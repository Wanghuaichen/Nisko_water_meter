#ifndef BOARD_H_
#define BOARD_H_

#include <msp430.h>
#include "mnglpm.h"
#include "serial.h"
#include <stddef.h>
#include <stdint.h>




extern struct sMeasCounter adcBuffer;
extern unsigned int BackTicker;
extern unsigned long isrTicker;
extern unsigned long virtualTimer1;
extern unsigned long virtualTimer2;
extern unsigned char sendLcd;
extern unsigned int sendSerial;
extern unsigned char sendLcdArrow;
extern struct sRTC rtc1;
extern unsigned char modemState;
//#define NULL (void*)0

//extern unsigned char ulpMemBuf[16];

#define UA_2500

#ifdef UA_2500
#define DCO_FRQ		7372800
#else
#define DCO_FRQ		3686400
#endif

#define ACLK_FRQ	32768
#define ACLK_DIV	4


#define MAX_WATER_METER_FRQ 100 //Hz
#define TA_INT_FRQ (MAX_WATER_METER_FRQ*4)
#define TA_MIN_FRQ (5)

#define TA_INT_TIME_USEC (1000000/TA_INT_FRQ) //in u sec

#define TA_MIN_COUNTS (ACLK_FRQ/TA_INT_FRQ) 
#define TA_MAX_COUNTS (ACLK_FRQ/TA_MIN_FRQ*4)

#define MS_TO_ACLK_COUNTS(x) (ACLK_FRQ*x/1000)



#define ACLK_TICKER_MS(x) ((x*1000)/TA_INT_TIME_USEC) //x msec

#define ACC_CONST 3;


/*
** A WDT timer step is of 0.25 Sec
*/

#ifdef ACLK_DIV
#if (ACLK_DIV==1)
	#define DIVA_VAL	DIVA_0
	/*
	** WDT configured for 1/4 sec interval
	*/
	#define WDT_ISVAL	WDTIS0
	#define	RTC_PRESCALER	1
#elif (ACLK_DIV==2)
	#define DIVA_VAL	DIVA_1
	/*
	** WDT configured for 1/8 sec interval
	*/
	#define WDT_ISVAL	WDTIS0
	#define RTC_PRESCALER	2
#elif (ACLK_DIV==4)
	#define	DIVA_VAL	DIVA_2
	/*
	** WDT configured for 1/16 sec interval
	** RTC configured for 1/16 sec interval
	*/
	#define WDT_ISVAL	WDTIS1
	#define RTC_PRESCALER	1
#elif (ACLK_DIV==8)
	#define	DIVA_VAL	DIVA_3
	/*
	** WDT configured for 1/8 sec interval
	*/
	#define WDT_ISVAL	WDTIS1
	#define	RTC_PRESCALER	2
#else
	#error "Invalid ACLK_DIV value"
#endif

#else
#error "ACLK_DIV should be defined"
#endif
#ifndef DIVA_VAL
#error "DIVA_VAL is not defined"
#endif


#define LITER_MOD 4
#define GAL_MOD 24
#define FT3_MOD 4


struct sMeasCounter
{
	uint16_t	A0;
	uint16_t	A1;

	long CWcounter;
	long CCWcounter;
	long counter;
	long oldCounter;

	unsigned char oldQuad;
	unsigned char newQuad;

	unsigned char arrowQuad;
	
	long dispCounter;
	long oldDispCounter;

	long dispFlow;
	long oldDispFlow;
	int  flow;
	
	long literCounter;
	long ft3Counter;
	long galCounter;

	unsigned char oldSin;
	unsigned char oldCos;

	unsigned int timerStamp;
	unsigned int stallCounter;
	unsigned int delta;
};



struct sRTC
{
	uint8_t	sec;
	uint8_t	 min;
	uint8_t hour;
	uint16_t day;
	uint16_t year;
};



#define startWdog() (WDTCTL = WDT_ADLY_1000)
#define stopWdog() (WDTCTL = WDTPW | WDTHOLD)


//port  define

#define LED_1_0 BIT0; //port 1.0 LED
#define MAG_PWR BIT7; //magnetometer power, port 3.7


extern const unsigned char PowerDownBits[POWER_DOWN_MODES];

#define	BD_1200			0
#define	BD_2400			1
#define	BD_9600         2
#define	BD_19200		3
#define	BD_38400		4
#define	BD_57600		5
#define	BD_76800		6
#define	BD_115200		7
#define BAUD_RATES		(BD_115200+1)

#define BAUD_RATES_UART0_RS_232 (BD_2400 )  
#define BAUD_RATES_UART1 (BD_1200)     			

#ifdef CRYSTAL_1
#define	BD_1200_DIV		0x0BF2      // for ACLK=3670016Hz
#define	BD_2400_DIV		0x05F9
#define	BD_9600_DIV		0x017E
#define	BD_19200_DIV	0x00BF
#define	BD_38400_DIV	0x005F
#define	BD_57600_DIV	0x0037
#define	BD_76800_DIV	0x002F
#define	BD_115200_DIV	0x001F

#define BD_1200_MOD		0x8A   // Error 0.02% 
#define BD_2400_MOD		0x44   // Error 0.09%
#define BD_9600_MOD		0x22   // Error 0.17%
#define BD_19200_MOD	0x44   // Error 0.73%
#define BD_38400_MOD	0xAE   // Error 1.65%
#define BD_57600_MOD	0x7E   // Error 2.69%
#define BD_76800_MOD	0xFE   // Error 3.74%
#define BD_115200_MOD	0xFE   // Error 5.383%

#else
#define	BD_1200_DIV		3072      // for SCLK=3686400Hz
#define	BD_2400_DIV		1536
#define	BD_9600_DIV		384
#define	BD_19200_DIV	192
#define	BD_38400_DIV	96
#define	BD_57600_DIV	64
#define	BD_76800_DIV	48
#define	BD_115200_DIV	32

#define BD_1200_MOD		0   // Error 0.02% 
#define BD_2400_MOD		0   // Error 0.09%
#define BD_9600_MOD		0   // Error 0.17%
#define BD_19200_MOD	0   // Error 0.73%
#define BD_38400_MOD	0   // Error 1.65%
#define BD_57600_MOD	0   // Error 2.69%
#define BD_76800_MOD	0   // Error 3.74%
#define BD_115200_MOD	0   // Error 5.383%

#endif

void rtcUpdate(void);
void initBoard(void);


#endif /*BOARD_H_*/
