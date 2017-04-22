
#include <msp430.h>
#include <stdint.h>
#include <stddef.h>
#include "board.h"
#include "spiLcd.h"
#include "CP240x_def.h"
#include "lcd_sigments.h"

#define LCD_SPI_FRQ (1843200)
//#define LCD_SPI_FRQ 2457600
#define SPI_UCBR_VAL(freq) (((DCO_FRQ)+(((freq)/2)+((freq)%2)))/(freq))

uint16_t CP240x_Measured_RTCCLK;            // Measured value of RTC

#define SPI_CS_ON()  P3OUT&=~0x01
#define SPI_CS_OFF() P3OUT|=0x01

#define LCD_RST_ON() P1OUT&=~BIT3
#define LCD_RST_OFF() P1OUT|=BIT3

#define LCD_INT() (!(P2IN&BIT5))


unsigned int spiReady=1;

//-----------------------------------------------------------------------------
// Global Variables 
//-----------------------------------------------------------------------------

unsigned char lcdCont=0x13;

uint16_t CP240x_Measured_RTCCLK;            // Measured value of RTC

uint8_t CP240x_MSCN_Local;                  // Local copy of the MSCN register
uint8_t CP240x_RTC0CN_Local;                // Local copy of RTC0CN register

                                       // Local copy of PortIO registers
CP240X_PORTREGS  CP240x_PortIO_Registers;


void spiInit(void)
{


	P3SEL |= BIT1+BIT2+BIT3;// P3.1,2,3 USCI_B0 option select
	P3DIR |= BIT0;
	UCB0CTL0 |= UCCKPH + UCMSB + UCMST + UCSYNC ;  // 3-pin, 8-bit SPI master
	UCB0CTL1 |= UCSSEL_2; // SMCLK
	UCB0BR0 = SPI_UCBR_VAL(LCD_SPI_FRQ)&0xff; // div 2
	UCB0BR1 = (SPI_UCBR_VAL(LCD_SPI_FRQ)>>8)&0xff;
	UCB0CTL1 &= ~UCSWRST; // **Initialize USCI state machine**


}



void spiTranfer(const void *dout, unsigned int xferLen,uint8_t flags)
{
	unsigned int txCount=0;
	
	volatile char tmp;// unsigned int tmp;
	volatile unsigned int blen=8;

	if (flags & SPI_XFER_BEGIN)
	{
		LockPD(POWER_ACTIVE,PD_MASK(PD_SPI));// lock power down level to LPM0	
		SPI_CS_ON();	// Select the device
	}
	
	// Flush SPI FIFO
	while (txCount<xferLen)
	{
		if (IFG2 & UCA0TXIFG)//TXBUF ready? //(SpiaRegs.SPIFFTX.bit.TXFFST<16)
		{
			UCB0TXBUF= (dout==NULL) ? 0 : (((unsigned char *)dout)[txCount]); //((unsigned int *)dout)[txCount]<<(15-SpiaRegs.SPICCR.bit.SPICHAR);
			txCount++;			
		}
	}
	while (UCB0STAT&UCBUSY);
	if (flags & SPI_XFER_END)
	{
		SPI_CS_OFF();	// Set to unselected level 
		UnlockPD(POWER_ACTIVE,PD_MASK(PD_SPI));// unlock power down level  LPM0	
	}
}





int spiXfer(const void *dout, void *din, unsigned int xferLen,uint8_t flags)
{
	unsigned int txCount=0;
	unsigned int rxCount=0;
	
	volatile char tmp;// unsigned int tmp;
	volatile unsigned int blen=8;

	if (flags & SPI_XFER_BEGIN)
	{
		LockPD(POWER_ACTIVE,PD_MASK(PD_SPI));// lock power down level to LPM0	
		SPI_CS_ON();	// Select the device
	}
	
	// Flush SPI FIFO
	while ((IFG2 & UCB0RXIFG));// RXBUF ready?
		tmp=UCB0RXBUF;
	while (txCount<xferLen)
	{
		if (IFG2 & UCA0TXIFG)//TXBUF ready? //(SpiaRegs.SPIFFTX.bit.TXFFST<16)
		{
			UCB0TXBUF= (dout==NULL) ? 0 : (((unsigned char *)dout)[txCount]); //((unsigned int *)dout)[txCount]<<(15-SpiaRegs.SPICCR.bit.SPICHAR);
			txCount++;			
		}
		while (UCB0STAT&UCBUSY);
		if (IFG2 & UCB0RXIFG)//(SpiaRegs.SPIFFRX.bit.RXFFST)
		{
			tmp=UCB0RXBUF;// & ((1<<blen)-1);
			if (rxCount<xferLen)
			{
				if (din)
					((unsigned char *)din)[rxCount]=tmp;
				rxCount++;	
			}
		}
	}
	while (rxCount<xferLen)
	{
		if (IFG2 & UCB0RXIFG) //RXBUF ready?
		{
			tmp=UCB0RXBUF;
			if (din)
				((unsigned char *)din)[rxCount]=tmp;
			rxCount++;	
		}
	}
	
	while (UCB0STAT&UCBUSY);
	if (flags & SPI_XFER_END)
	{
		SPI_CS_OFF();	// Set to unselected level 
		UnlockPD(POWER_ACTIVE,PD_MASK(PD_SPI));// unlock power down level  LPM0	
	}

	return 0;

}




void CP240x_RegWrite(unsigned char address, unsigned char data)
{
	unsigned char outBuff[4];

	outBuff[0]=REGWRITE; //command
	outBuff[1]=0x00; //add H
	outBuff[2]=address;//add L
	outBuff[3]=data; //data
	
	spiXfer(outBuff,NULL,4,SPI_XFER_BEGIN|SPI_XFER_END);
}

uint8_t CP240x_RegRead(unsigned char address)
{
	unsigned char outBuff[5];
	unsigned char inBuff[5];

	outBuff[0]=REGREAD;
	outBuff[1]=0x00;
	outBuff[2]=address;
	outBuff[3]=0x00;
	outBuff[4]=0x00;
	
	spiXfer(outBuff,inBuff,5,SPI_XFER_BEGIN|SPI_XFER_END);
	
	return inBuff[4];
}
void CP240x_RegBlockWrite(unsigned char address, unsigned char *data,uint8_t len)
{
	unsigned char outBuff[3];

	outBuff[0]=REGWRITE;
	outBuff[1]=0x00;
	outBuff[2]=address;
	spiXfer(outBuff,NULL,3, SPI_XFER_BEGIN);
	spiXfer(data,NULL,len,SPI_XFER_END);
}

void CP240x_RegBlockRead(unsigned char address, unsigned char *data,uint8_t len)
{
	unsigned char outBuff[4];

	outBuff[0]=REGREAD;
	outBuff[1]=0x00;
	outBuff[2]=address;
	outBuff[3]=0x00;
	
	spiXfer(outBuff,NULL,4, SPI_XFER_BEGIN);
	spiXfer(NULL,data,len,SPI_XFER_END);
}


/*
 Initializing the LCD Segment Driver

1. Configure the LCD size, mux mode, and bias using the LCD0C
2. Configure the Port I/O pins to be used for LCD as Analog I/O.
3. Set the LCD contrast using the CONTRAST register.
4. Write the reserved value of 0x9F to LCD0CF.
5. Set the LCD refresh rate using the LCD0DIVH:LCD0DIVL regis
6. Set the LCD toggle rate using the LCD0TOGR register.
7. Set the LCD power mode using the LCD0PWR register.
8. Write a pattern to the ULP memory.
9. Enable the LCD using the master control (MSCN) register.
*/

void spiLcdInit(void)
{
	int i;
	unsigned char inData;
	spiInit();

	uint8_t  lcd_reg_array[7];
	uint8_t  lcd_reg_array_verify[7];
	uint8_t LCD0CF_value;



	
	#if(ALLOW_STOPWATCH_CALLBACKS)
	uint16_t LCD0DIV;
	#endif

	
	#if(MUX_MODE == STATIC)

	#if(ALLOW_STOPWATCH_CALLBACKS)
	LCD0DIV = (CP240x_Measured_RTCCLK/4/1/REFRESH_RATE_HZ)-1; 
	#else
	#define LCD0DIV  (CP240X_RTCCLK/4/1/REFRESH_RATE_HZ)-1  
	#endif

	#elif (MUX_MODE == TWO_MUX)

	#if(ALLOW_STOPWATCH_CALLBACKS)
	LCD0DIV = (CP240x_Measured_RTCCLK/4/2/REFRESH_RATE_HZ)-1; 
	#else
	#define LCD0DIV  (CP240X_RTCCLK/4/2/REFRESH_RATE_HZ)-1
	#endif

	#elif (MUX_MODE == THREE_MUX)

	#if(ALLOW_STOPWATCH_CALLBACKS)
	LCD0DIV = (CP240x_Measured_RTCCLK/4/3/REFRESH_RATE_HZ)-1; 
	#else
	#define LCD0DIV  (CP240X_RTCCLK/4/3/REFRESH_RATE_HZ)-1   
	#endif

	#elif (MUX_MODE == FOUR_MUX)

	#if(ALLOW_STOPWATCH_CALLBACKS)
	LCD0DIV = (CP240x_Measured_RTCCLK/4/4/REFRESH_RATE_HZ)-1; 
	#else
	#define LCD0DIV  (CP240X_RTCCLK/4/4/REFRESH_RATE_HZ)-1   
	#endif

	#endif

	// Calculate value for LCD0CF
	#if(ALLOW_STOPWATCH_CALLBACKS)
	LCD0CF_value = (CP240x_Measured_RTCCLK/500); 
	#else
	LCD0CF_value = CP240X_RTCCLK/500;   
	#endif

	if(LCD0CF_value > 63) LCD0CF_value = 63;

	// Initialize Port I/O For LCD functionality
	CP240x_PortIO_Registers.P0MDI_Local = 0x00;
	CP240x_PortIO_Registers.P1MDI_Local = 0x00;
	CP240x_PortIO_Registers.P2MDI_Local = 0x00;
	CP240x_PortIO_Registers.P3MDI_Local = 0x00;
	CP240x_PortIO_Registers.P4MDI_Local = 0x00;

	CP240x_PortIO_Configure();

	// Initialize LCD control registers
	lcd_reg_array[0] = (0x08 | MUX_MODE | BIAS);// LCD0CN
	lcd_reg_array[1] = lcdCont;                   // CONTRAST     
	lcd_reg_array[2] = (0x80 | LCD0CF_value);   // LCD0CF
	lcd_reg_array[3] = (LCD0DIV & 0xFF);        // LCD0DIVL  
	lcd_reg_array[4] = ((LCD0DIV>>8) & 0xFF);   // LCD0DIVH
	lcd_reg_array[5] = 0;                       // LCD0TOGR   
	lcd_reg_array[6] = 0x10;                    // LCD0PWR

	// Write the LCD registers
	CP240x_RegBlockWrite(LCD0CN, lcd_reg_array, 7);
	CP240x_RegBlockRead(LCD0CN,lcd_reg_array_verify,7);

	// Enable LCD Functionality
	CP240x_MSCN_SetBits(0x01);


	//LCD size, mux mode, and bias

	CP240x_RegWrite(LCD0CN,(SIZE+ MUX_MODE_4 + 0x01)); //BIAS 1/2

	//Configure the Port I/O pins to be used for LCD as Analog I/O.
	for (i=0;i<N_PORT;i++)
	{
		CP240x_RegWrite(P0MDI+i,INPUT_MODE_0);
	}

	// Set the LCD contrast 
	CP240x_RegWrite(CONTRAST,lcdCont);

	//LCD Configuration;
	CP240x_RegWrite(LCD0CF,0x80|LCD0CF_value);


	//Set the LCD refresh rate using the LCD0DIVH regis
	CP240x_RegWrite(LCD0DIVH,0x00);

	//Set the LCD refresh rate using the LCD0DIVL regis
	CP240x_RegWrite(LCD0DIVL,0x28);

	//Set the LCD toggle rate
	CP240x_RegWrite(LCD0TOGR,0x00);

	//Set the LCD power mode using the LCD0PWR register.
	CP240x_RegWrite(LCD0PWR,NORMAL);

	//LCD enable
	//CP240x_RegWrite(MSCN,LCDEN|RTCBYP);
	CP240x_RegWrite(MSCN,LCDEN|RTCBYP|RTCOD);

}





#if ALLOW_STOPWATCH_CALLBACKS 
//-----------------------------------------------------------------------------
// CP240x_RTC_MeasureFrequency ()
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   : None
//
// This function will configure Timer1 to capture the SmaRTClock oscillator
// A timer 1 interrupt will be generated every 8 RTC cycles and the value 
// of timer 1 at the time of the interrupt will be available for reading
// in the TMR1RLL registers.
//
//-----------------------------------------------------------------------------
uint16_t CP240x_RTC_MeasureFrequency (void)
{
	#define MSB 1
	#define LSB 0
	union
	{
		uint16_t U16;
		uint8_t U8[2];
	} INTEN_save;
	union
	{
		uint16_t U16;
		uint8_t U8[2];
	} INTEN_registers;
   
	union
	{
		uint16_t U16;
		uint8_t U8[2];
	} INT_registers; 
 

	union
	{
		uint16_t U16;
		uint8_t U8[2];
	} period_counts;

	
	union
	{
		uint32_t U32;
		uint16_t U16[2];
		uint8_t U8[4];
	} rtc_frequency;
   
      
   // Save the current value of the interrupt enable registers
   CP240x_RegBlockRead(INT0EN, &INTEN_save.U8[MSB], 2);
      
   // Enable the Timer 1 and RTC Interrupts, disable all others
   INTEN_registers.U16 = 0x1808;
   CP240x_RegBlockWrite(INT0EN, &INTEN_registers.U8[0], 2);

   // Clear All Interrupt Flags
   CP240x_ClearAllFlags();
   CP240x_ClearAllInterrupts();

   CP240x_Timer1_Init(8);              // Timer overflows every 8 counts   
   CP240x_RegWrite(TMR1CN, 0x05);      // Timer counts SmaRTClock / 8

   // Wait for the next interrupt
 	while(!LCD_INT());                         // Wait for reset complete int.    // while(INT);

   // Read the interrupt registers
   CP240x_RegBlockRead(INT0, &INT_registers.U8[MSB], 2);

   // Check for a clock failure or alarm
   if(INT_registers.U8[MSB] != 0x00)
   {
      return UNEXPECTED_SMARTCLOCK_EVENT;
   } 


   //----------------------------------------
   // Obtain first timer reading
   //----------------------------------------

   Stopwatch_Reset();

   // Wait for the next interrupt
  	while(!LCD_INT()); // while(INT);

   Stopwatch_Start();

   // Read the interrupt registers
   CP240x_RegBlockRead(INT0, &INT_registers.U8[MSB], 2);

   // Check for a clock failure or alarm
   if(INT_registers.U8[MSB] != 0x00)
   {
      return UNEXPECTED_SMARTCLOCK_EVENT;
   } 
   

   //----------------------------------------
   // Obtain second timer reading
   //----------------------------------------

   // Wait for the next interrupt
   while(!LCD_INT()); // while(INT);
   
   period_counts.uint16_t = Stopwatch_Stop();

   // Read the interrupt registers
   CP240x_RegBlockRead(INT0, &INT_registers.U8[MSB], 2);
   
   // Check for a clock failure or alarm
   if(INT_registers.U8[MSB] != 0x00)
   {
      return UNEXPECTED_SMARTCLOCK_EVENT;
   } 


   // Restore Interrupts
   CP240x_RegWrite(TMR1CN, 0x00);      // Disable Timer 1
   CP240x_RegBlockWrite(INT0EN, &INTEN_save.U8[MSB], 2);

   //--------------------------------------------
   // Determine the counts in 64 periods
   //--------------------------------------------
   
   //--------------------------------------------
   // Return the SmaRTClock frequency
   //--------------------------------------------

   //#define SYSCLK_CALIBRATED 24500000L
   
   rtc_frequency.U32 = ((SYSCLK*64L)/12L);
   rtc_frequency.U32 /= period_counts.uint16_t;
   
   CP240x_Measured_RTCCLK = rtc_frequency.uint16_t[LSB];

   return rtc_frequency.uint16_t[LSB];

}
#endif

int8_t CP240x_Reset(void)
{
   unsigned long delay=15UL*DCO_FRQ/1000000;
   uint8_t rev;
   //----------------------------------
   // Generate a 15 us pulse on reset 
   //----------------------------------
   LCD_RST_ON();                            // Assert the CP240x Reset

   __delay_cycles(15UL*DCO_FRQ/1000000);                       // Allow the CP240x enough time (15 uSec)
                                       // to recognize the reset event                                                                          

   LCD_RST_OFF();                            // De-Assert the CP240x Reset   
   rev=0;
   while(!LCD_INT());                         // Wait for reset complete int.   


   //----------------------------------
   // Verify that the serial interface
   // is working properly
   //----------------------------------
   rev = CP240x_RegRead(REVID);

   if(rev != 0x01)
   {
      while(1);                        // Check Port I/O Config
   } 

   //---------------------------------
   // Disable all interrupts by 
   // clearing INT0EN and INT1EN
   //---------------------------------
   CP240x_RegBlockWrite(INT0EN, "\0", 2);   


   //---------------------------------
   // Initialize Registers
   //---------------------------------

   
   CP240x_MSCN_Local = 0x00;           // Reset value of the MSCN register
   
   // Read the reset values of all writable Port I/O configuration registers
   CP240x_RegBlockRead(CP_P0OUT, &CP240x_PortIO_Registers.P0OUT_Local, CP240X_NUM_PORTREGS_CONFIG);
   
   CP240x_RegWrite(RTCKEY,0xA5);       // Unlock the SmaRTClock interface
   CP240x_RegWrite(RTCKEY,0xF1);  

   return 0;


}


//-----------------------------------------------------------------------------
// CP240x_RTC_Init ()
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   : None
//
// This function will initialize the smaRTClock. 
//
//-----------------------------------------------------------------------------
void CP240x_RTC_Init ()
{  
	unsigned char rtcoxcn_debug=0;
   uint16_t interrupt_status;
                          
   //----------------------------------------
   // Configure the SmaRTClock to crystal
   // or self-oscillate mode
   //---------------------------------------- 
   
   // Disable SmaRTClock
   CP240x_RTC_Write(RTC0CN,0x00);       

   #if(CP240X_RTC_CLKSRC == CP240X_CRYSTAL)            

      // Configure the smaRTClock oscillator for crystal mode
      // Bias Doubling Enabled, AGC Disabled
      CP240x_RTC_Write (RTC0XCN, 0x60);   
                                       
      // Enable Auto Load Cap Stepping
      CP240x_RTC_Write (RTC0XCF, (0x80 | CP240X_LOADCAP_VALUE));   
   
   #elif(CP240X_RTC_CLKSRC == CP240X_SELFOSC)

      // Configure smaRTClock to self-oscillate mode with bias X2 disabled
      CP240x_RTC_Write (RTC0XCN, 0x00);   
                                      
      // Disable Auto Load Cap Stepping
      CP240x_RTC_Write (RTC0XCF, (0x00 | CP240X_LOADCAP_VALUE));   
   
   #elif(CP240X_RTC_CLKSRC == CP240X_CMOSCLK)
      
      // Configure the smaRTClock oscillator for crystal mode
      // Bias Doubling Enabled, AGC Disabled
      CP240x_RTC_Write (RTC0XCN, 0x60); 

      // Disable Auto Load Cap Stepping
      CP240x_RTC_Write (RTC0XCF, (0x00 | CP240X_LOADCAP_VALUE));   

   #else
      #error "Must select crystal or self oscillate mode"                                   
   #endif

   
   // Enable SmaRTClock oscillator
   CP240x_RTC_Write (RTC0CN, 0x80);    
  
   //----------------------------------------
   // Wait for crystal to start
   // No need to wait in self-oscillate mode
   //----------------------------------------  

   #if (CP240X_RTC_CLKSRC == CP240X_CRYSTAL)

      // Wait > 20 ms
      __delay_cycles((20/1000000L)*DCO_FRQ);

      // Wait for smaRTClock valid
      //while ((CP240x_RTC_Read (RTC0XCN) & 0x10)== 0x00); 
      while((rtcoxcn_debug & 0x10)== 0x00)
      	rtcoxcn_debug=CP240x_RTC_Read (RTC0XCN);
      	 
       // Wait for Load Capacitance Ready     	
      while ((CP240x_RTC_Read(RTC0XCF) & 0x40)== 0x00); 
      
      // Enable Automatic Gain Control and disable bias doubling
      CP240x_RTC_Write (RTC0XCN, 0xC0);   
                                       
   #endif

  
   CP240x_RTC_Write (RTC0CN, 0xC0);    // Enable missing smaRTClock detector
                                       // and leave smaRTClock oscillator
                                       // enabled. 

   __delay_cycles((2/1000000L)*DCO_FRQ);// Wait for missing clock detector 
                                       // to stabilize
   
   CP240x_RTC_Write(RTC0CN, 0xC0);     // Clear the OSCFAIL flag
 
   CP240x_MSCN_SetBits(0x40);          // Clear interrupt flag
   CP240x_RegBlockRead(INT0, (uint8_t*) &interrupt_status, 2);

   CP240x_RTC0CN_Local = 0xC0;         // Initialize local copy of RTC0CN

   
 
}

//-----------------------------------------------------------------------------
// RTC Functions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// CP240x_RTC_Write ()
//-----------------------------------------------------------------------------
//
// Return Value : none
// Parameters   : 1) unsigned char reg - address of RTC register to write
//                2) unsigned char value - the value to write to <reg>
//
// This function will Write one byte to the specified RTC register.
// Using a register number greater that 0x0F is not permited.
//-----------------------------------------------------------------------------
void CP240x_RTC_Write (uint8_t addr, uint8_t value)
{
    union
	{
		uint16_t U16;
		uint8_t U8[2];
	}addr_value;
     
   addr_value.U8[1] = (0x10 | addr) ;
   addr_value.U8[0] = value;   
   
   // Write to the adjacent RTCADR and RTCDAT registers
   CP240x_RegBlockWrite(RTCADR, &addr_value.U8[1], 2); 

}
//-----------------------------------------------------------------------------
// CP240x_RTC_Read
//-----------------------------------------------------------------------------
//
// Return Value : RTC0DAT
// Parameters   : 1) unsigned char reg - address of RTC register to read
//
// This function will read one byte from the specified RTC register.
// Using a register number greater that 0x0F is not permited.
//
//-----------------------------------------------------------------------------
uint8_t CP240x_RTC_Read (uint8_t addr)
{
   uint8_t value;
   
   value = addr;

   // Select the register to read and initiate transfer to the data register
   CP240x_RegWrite(RTCADR, (0x90 | addr));

   // Read the value from the data register
   value = CP240x_RegRead(RTCDAT);                      

   return value; 
}

//-----------------------------------------------------------------------------
// CP240x_PortIO_Configure
//-----------------------------------------------------------------------------
//
// Return Value : none
// Parameters   : none
//
// Transfers the contents of the CP240x_PortIO_Registers array to the writable 
// registers on the CP240x 
//-----------------------------------------------------------------------------
void CP240x_PortIO_Configure(void)
{
   CP240x_RegBlockWrite(CP_P0OUT, &CP240x_PortIO_Registers.P0OUT_Local,
                        CP240X_NUM_PORTREGS_CONFIG);
}

//-----------------------------------------------------------------------------
// CP240x_PortIO_Read
//-----------------------------------------------------------------------------
//
// Return Value : none
// Parameters   : none
//
// Transfers the contents of the PMATCHST and Port I/O Input Registers
// to the CP240x_PortIO_Registers array.
//-----------------------------------------------------------------------------

void CP240x_PortIO_Read(void)
{
   CP240x_RegBlockRead(PMATCHST, &CP240x_PortIO_Registers.PMATCHST_Local,
                        CP240X_NUM_PORTREGS_READ);

}
//-----------------------------------------------------------------------------
// MSCN Management
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// CP240x_MSCN_Update
//-----------------------------------------------------------------------------
//
// Return Value : None
//    
//   
// Parameters   : value
//
// Updates MSCN with the new value and stores a copy in MSCN_Local.
//
//-----------------------------------------------------------------------------
void CP240x_MSCN_Update (uint8_t value)
{
   CP240x_MSCN_Local = (value & ~0x42);
   CP240x_RegWrite(MSCN, value);
}


//-----------------------------------------------------------------------------
// Power Management
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// CP240xEnterLpm()
//-----------------------------------------------------------------------------
void CP240xEnterLpm(void)
{
	__disable_interrupt();


	CP240x_RegWrite(INT0EN,0x00); //enables the SmaRTClock Fail, SmaRTClock Alarm, and Port
  	CP240x_RegWrite(INT1EN,0x19);//Match interrupts and disables all others.

	CP240x_RegWrite(MSCF,0x80);//lowest supply current at the expense of increased ripple in the LCD output voltage.
	SPI_CS_ON();
	CP240x_RegWrite(ULPCN,BIT3+BIT1);// enter mode Ultra Low Power LCD Enable.

	SPI_CS_OFF(); //Rising edge transitions on NSS and PWR disable the internal LDO and place the device into the ultra low power mode

	__enable_interrupt();
}

void CP240xExitLpm(void)
{
	 uint8_t rev=0;
	int count=0;
	unsigned char port_type [5]={0x00,0x00,0x00,0x00,0x00};

	SPI_CS_ON(); ///falling edge transition on NSS or PWR will re-enable the regulator and return the device to  normal power mode
	//__delay_cycles(100000UL*DCO_FRQ/1000000);						// Allow the CP240x enough time (1 uSec)
                                                                      
	#ifdef KUKU
		LCD_RST_ON();                            // Assert the CP240x Rese
		__delay_cycles(15UL*DCO_FRQ/1000000);      // Allow the CP240x enough time (15 uSec) to recognize the reset event                                                                          
		LCD_RST_OFF();                            // De-Assert the CP240x Reset   
		rev=0;
		while(!LCD_INT());  
				
           
	#endif
 
  	//spiLcdInit();
	


	//----------------------------------
	// Verify communication
	//----------------------------------
    rev = CP240x_RegRead(REVID);
	while(rev != 0x01)
	{
		rev = CP240x_RegRead(REVID);
		SPI_CS_ON(); 
		count++;
		if (count>10)
		{
			rev = 0x01;
			ERR_DISP(ON);
		}
	}
 	SPI_CS_OFF();
	CP240x_RegWrite(ULPCN,0x00);
	CP240x_RegBlockWrite(INT0EN, "\0", 2);  
	//CP240x_RegBlockWrite(P0MDI,port_type,5); //Configure the Port I/O pins to be used for LCD as Analog I/O.

}

