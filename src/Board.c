#include "Board.h"
#include <msp430.h>
#include "dco.h"

struct sMeasCounter adcBuffer;
struct sRTC rtc1;

unsigned int BackTicker=0;
unsigned long isrTicker=0;
unsigned long virtualTimer1=0;
unsigned long virtualTimer2=0;

unsigned char sendLcd=0;
unsigned int sendSerial=0;
unsigned char sendLcdArrow=0;
unsigned char modemState=0;



//unsigned char ulpMemBuf[16];

const unsigned char PowerDownBits[POWER_DOWN_MODES] =/* constant param for set power down mode*/
{
		LPM0_bits, 
		LPM1_bits, 
		LPM2_bits,
		LPM3_bits,
		LPM4_bits
};


const unsigned short BaudRateDiv[BAUD_RATES]= /* constant param for set baud rate*/
{
		BD_1200_DIV,
		BD_2400_DIV,
		BD_9600_DIV,
		BD_19200_DIV,
		BD_38400_DIV,
		BD_57600_DIV,
		BD_76800_DIV,
		BD_115200_DIV
};

const unsigned char BaudRateMod[BAUD_RATES]= /* constant param for set baud rate*/
{
		BD_1200_MOD,
		BD_2400_MOD,
		BD_9600_MOD,
		BD_19200_MOD,
		BD_38400_MOD,
		BD_57600_MOD,
		BD_76800_MOD,
		BD_115200_MOD
};


#ifdef KUKU
/*
** Port 1 initial configuration
*/

#define INIT_P1_OUT (0x00)
#define INIT_P1_DIR (0x01) //p1.0 out
#define INIT_P1_SEL (0x00)
#define INIT_P1_REN (0x00)

/*
** Port 2 initial configuration
*/
#define INIT_P2_OUT (0x00)
#define INIT_P2_DIR (0x00)
#define INIT_P2_SEL (0x00)
#define INIT_P2_REN (0x00)

/*
** Port 3 initial configuration
*/
#define INIT_P3_OUT (0x00)
#define INIT_P3_DIR (0x00)
#define INIT_P3_SEL (0x00)
#define INIT_P3_REN (0x00)
	
/*
** Port 4 initial configuration
*/
#define INIT_P4_OUT (0x00)
#define INIT_P4_DIR (0x00)
#define INIT_P4_SEL (0x00)
#define INIT_P4_REN (0x00)

#endif



void initBoard(void)
{
	//stopWdog();
	WDTCTL = WDTPW + WDTHOLD;                 // Stop watchdog timer


	
	/*
	** Configure system clock
	*/
	#ifdef STATIC_DCO
		BCSCTL1 = CALBC1_8MHZ;					  // Set DCO to 8MHz
		DCOCTL = CALDCO_8MHZ;
	#else
	BCSCTL1=(BCSCTL1 & (RSEL2 | RSEL1 | RSEL0)) | XT2OFF | DIVA_VAL;
	BCSCTL2=SELM_0 | DIVM_0 | DIVS_0; // SMCLK= DCO

	Calibrate_DCO();
	BCSCTL1=(BCSCTL1& ~DIVA_3)|DIVA_0;

	#endif
	

	
	//init ports
	P1SEL |=BIT4; //SMCLK @ p1.4
	P1DIR |=BIT7|BIT6|BIT5|BIT4|BIT3|BIT0; //outputs- P1.0 LED,1.3 LCD rst,1.4 SMCLK,1.5 TP16,1.6 TP17

	P2SEL |=BIT0;//2.0 ACLK to LCD

	P2DIR |=BIT0|BIT4;// 2.0 ACLK,2.4 TP21
	
	P3DIR |= BIT7; //magnetometer power

	#ifdef RF_ACTIVE
	P3SEL = 0x30; // P3.4,5 = USCI_A0 TXD/RXD
	#endif


	TACCR0= TA_MIN_COUNTS;
	adcBuffer.delta=TA_MIN_COUNTS<<ACC_CONST;// *ACC_CONST
	TACCR1=0x3FFF;
	TACCTL0 =CCIE;                // TA0CCR0 toggle, interrupt enabled
	TACCTL1 =CCIE;                // TA0CCR1 toggle, interrupt enabled
	TACCTL2 =CCIE;                // TA0CCR2 toggle, interrupt enabled
	TACTL = TASSEL_1 + MC_2 + TAIE; 		  // ACLK, up to CCR0, interrupt enabled

	
	//init ADC
	ADC10AE0 |= BIT1+BIT2;  // P2.1,2 ADC10 option select
	ADC10CTL1 = INCH_2+ CONSEQ_1 + ADC10SSEL_2; // A1/A2, single sequence , MCLK
	ADC10DTC1 = 0x02;
	ADC10CTL0 = ADC10SHT_0 + MSC + ADC10ON + ADC10IE;

	rtc1.year=rtc1.day=rtc1.hour=rtc1.min=rtc1.sec=0;



}


void rtcUpdate(void)
{
	rtc1.sec++;
	if (rtc1.sec<60)
		return;
	rtc1.sec=0;
	rtc1.min++;
	
	if (rtc1.min<60)
		return;
	rtc1.min=0;
	rtc1.hour++;
	
	if(rtc1.hour<24)
		return;
	rtc1.hour=0;
	rtc1.day++;

	if(rtc1.day<365)
		return;
	rtc1.day=0;
	rtc1.year++;

}
