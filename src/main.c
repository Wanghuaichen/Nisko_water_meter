//******************************************************************************
// Nisko Magnetic Water Meter
//
//     MSP430f2132
//
//  Shlomi Dar
//  Redler Computers
//  January 2011
//  Built with CCE for MSP430 Version: 4.0
//******************************************************************************

#include <msp430.h>
#include <stdint.h>
#include <stddef.h>

#include "mnglpm.h"
#include "Board.h"
#include "spiLcd.h"
#include "CP240x_def.h"
#include "msp430f2132.h"
#include "modem.h"
#include "lcd_sigments.h"
#include "serial.h"
#include "nvm.h"
#include "spimem.h"
#include "Flow_counter.h"



#define HYSTERESIS (0x3ff/20)
#define NEG ((0x3FF/2)-HYSTERESIS)
#define POS ((0x3FF/2)+HYSTERESIS)

//extern const unsigned short BaudRateDiv[BAUD_RATES];
//extern const unsigned char BaudRateMod[BAUD_RATES];



/*remove after debug*/
unsigned char PowerDownLevel=POWER_ACTIVE;
/*******************************************/
volatile unsigned int i;                    // volatile to prevent optimization
unsigned char Data;
long int binIn;
unsigned char decOut[6];
long int gal_5sec_old=0;
char tmpBuf[6]={0,1,2,3,4,5};

//debug lcd
uint16_t rtc_frequency;
unsigned int enterLMP=0;
unsigned int debugLcd=1;


unsigned char Framelength=31;


//#define NULL (void*)0
#define ISR_ARROW 1

void main(void)
{

 	initBoard();			//Initialize board
 	#ifdef RF_ACTIVE
 	SerialInit(768); ///768
	#endif
	
	InitLPMng();			// Initialize low power mode manager

	spiInit();

#ifdef LCD_DRIVER
	CP240x_Reset();

  	#ifdef RTC_ENABLE
	
		CP240x_RTC_Init();
		
		prevLcdCont=lcdCont^0x1;
		#if ALLOW_STOPWATCH_CALLBACKS
		rtc_frequency = CP240x_RTC_MeasureFrequency();	
		#endif
	#endif
		
	spiLcdInit();

	
	//memset(ulpMemBuf, 0x00, sizeof(ulpMemBuf));
	memset(&ulpMem, 0x00, sizeof(ulpMem));
	memset(&inBuf,0x00,sizeof(inBuf));
	memset(&TxBuf,0x00,sizeof(TxBuf));


	
	/*display Flags*/
	ARROW_CENTER(ON);
	T1_ARROW_CIRCLE(ON);
	GAL_MIN(ON);
	GAL(ON);
	
	CP240x_RegBlockWrite(ULPMEM00,(unsigned char *)&ulpMem,13);

	#ifdef LCD_LPM
	CP240xEnterLpm(); //LCD LPM
	#endif
#endif
	
	#ifdef RF_ACTIVE
		LockPD(POWER_ACTIVE,PD_MASK(PD_USART));			// lock power down level to LPM0					
#endif



	memset(&adcBuffer,0x00,sizeof(adcBuffer));
	adcBuffer.oldDispCounter=adcBuffer.dispCounter^0x01;
	adcBuffer.oldDispFlow=adcBuffer.dispFlow^0x01;

	BackTicker=0;
	adcBuffer.counter=0;
	//adcBuffer.CWcounter=0;
	//adcBuffer.CCWcounter=0;
	adcBuffer.oldDispCounter=0x01;

	_bis_SR_register(GIE); 
	//ADC10CTL0 = ADC10SHT_0 + MSC + ADC10ON + ADC10IE;
 // #define DEBUG_LCD 1

	nvmSetup();	
	mem1.offset=START_DELTA_ADRESS;

	TXMode=UDP_TX_MODE;

		if(TXMode==UDP_TX_MODE)
			{
				while(protocol_state!=CONNECTED)
					Association();	
			}
  for(;;)
  {		
   		
		BackTicker++;
	
		#ifdef DEBUG_LOGGER
		
		if (adcBuffer.dispCounter==4000)
		{
			int buf[32];

			__disable_interrupt();
			while (adcBuffer.dispCounter)
			{
				spiMemReadRawDelta(mem1.offset,(char *)buf);
				adcBuffer.dispCounter-=16;
				mem1.offset+=32;
			}
			__enable_interrupt();
		}
		
		#endif
			
		#ifdef RF_ACTIVE
//			#ifdef ADHOC
//			initAdHoc();
//			#else
//			initTcpIp();
//			#endif

			#ifdef RF_LPM
			if(sendSerial)  //if((protocol_state==0x55)&&sendSerial)
			{
			sendSerial=0;
			FrameTX(0x01,8888,Framelength);
			UnlockPD(POWER_ACTIVE,PD_MASK(PD_USART));
			}
			#else
			if((protocol_state==0x55)&&sendSerial)
			{
				sendData(0xEE,3,4,adcBuffer.galCounter);
				sendSerial=0;
				UnlockPD(POWER_ACTIVE,PD_MASK(PD_USART));
			}
			#endif
		#endif

		#ifdef SPI_LCD
			if (sendLcd==1)
			{
				#ifdef LCD_LPM
				CP240xExitLpm();
				#endif
				sendDisplay();
				sendLcd=0;
				#ifdef LCD_LPM
				CP240xEnterLpm();
				#endif
			}
			else if(updateArrow()&&sendLcdArrow)
			{
				#ifdef LCD_LPM
				CP240xExitLpm();
				#endif
				CP240x_RegBlockWrite(ULPMEM06,(unsigned char *)&ulpMem.ulp6,5); //apdate arrows only
				sendLcdArrow=0;
				#ifdef LCD_LPM
				CP240xEnterLpm();
				#endif
			}
		#endif
			
		#ifndef DISABLE_LPM
		//UnlockPD(POWER_ACTIVE,PD_MASK(PD_LPM_ACTIVE));
			
		PowerDownLevel=MaxPDLevel(MAX_ENABLED_PD_MODE);	 //Calculate lowest enable level of low power mode
		if (PowerDownLevel)
			__bis_SR_register(PowerDownBits[PowerDownLevel-1] + GIE);  //Insert to power down level
		#endif

  }
}


// Timer0_A0 interrupt service routine
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{

	if (adcBuffer.delta>TA_MAX_COUNTS)
	{
		TA0CCR0 += TA_MAX_COUNTS;
	}
	else if (adcBuffer.delta>TA_MIN_COUNTS)
	{
		TA0CCR0 += adcBuffer.delta;
	}
	else
	{
		TA0CCR0 +=TA_MIN_COUNTS;
	}


	isrTicker++;
 	//start ADC cycle
	sampleWaterMeter();
}


// Timer0_A1 Interrupt Vector (TAIV) handler
#pragma vector=TIMER0_A1_VECTOR
__interrupt void Timer_A1(void)
{
  switch (__even_in_range(TA0IV, 10))        // Efficient switch-implementation
  {
    case  2: TACCR1 +=MS_TO_ACLK_COUNTS(100) ;                // Add Offset to TA0CCR1 100ms
   			 virtualTimer1++;
			 #ifdef RF_LPM
			if (modemState==ON)//modem is on?
			{
				//sendSerial=1;
				__bic_SR_register_on_exit(LPM0_bits);
			}
			#endif
			 #ifdef DISP_SHOW
			 adcBuffer.counter++;
			 if (adcBuffer.counter>=GAL_MOD)
				{
					adcBuffer.dispCounter++;
					adcBuffer.counter-=GAL_MOD;
				}
			 #endif

			#ifdef DEBUG_LOGGER
			//adcBuffer.dispCounter++;
			nvm1.write(&nvm1, 0, 6, &tmpBuf[0], NULL, NULL); //write delta 
			//spiMemWriteDelta();
			#endif
			
			  sendLcdArrow=1;
             break;
    case  4: TACCR2 += MS_TO_ACLK_COUNTS(1000);               // Add Offset to TA0CCR2 1s
    
    		sendSerial=1;
    		modemState^=0x01;
			rtcUpdate();

		#ifdef RF_LPM
			
			#ifdef DEBUG_LPM
			//send packet
			sendSerial=1;
			__bic_SR_register_on_exit(LPM0_bits);
			#else
			if (rtc1.sec==1)
			{
				//send packet
				//sendSerial=1;
				__bic_SR_register_on_exit(LPM0_bits);
			}
			#endif
		#else
			if((virtualTimer2&0x01)==0x01)
				{
					sendSerial=1;
					__bic_SR_register_on_exit(LPM0_bits);
				}
		#endif


			waitForFlag++;
			virtualTimer2++;
			sendLcd=1;
             break;
    case 10:                 // Timer_A3 overflow
    	
    	#ifdef DISP_SHOW
    	adcBuffer.dispFlow=55;
		#else
		adcBuffer.dispFlow=(60*adcBuffer.flow)/(GAL_MOD);
		#endif
		adcBuffer.flow=0;
		//sendSerial++;
		


        break;
  }

}

// ADC10 interrupt service routine
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void)
{	
#ifdef DISP_SHOW
	__bic_SR_register_on_exit(LPM0_bits);
#elif DEBUG_LOGGER
	__bic_SR_register_on_exit(LPM0_bits);
#else
	if (calcCount())
	{
		timeStepCalc();
		adcBuffer.stallCounter=0;
		__bic_SR_register_on_exit(LPM0_bits);
		sendLcd=1;
	}
	else
		stallCheck();
#endif




	
//	LockPD(POWER_ACTIVE,PD_MASK(PD_LCD));			// lock power down level to LPM0

	

}


// USCI A0/B0 Transmit ISR
#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCI0TX_ISR(void)
{
SerialTxIsr();
	
//if (SerialTxIsr())
//{
	//__bic_SR_register_on_exit(LPM0_bits+GIE);
	//LockPD(POWER_ACTIVE,PD_MASK(PD_USART));			// lock power down level to LPM0					
	//_BIC_SR_IRQ( 0x00f0 );						//Return from int to HS
//}

}

// USCI A0/B0 Receive ISR
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
//	SerialRxIsr();
if (SerialRxIsr())
	_BIC_SR_IRQ( 0x00f0 );						//Return from int to HS
}

