
#include <stdint.h>


#ifndef _SPILCD_H
#define _SPILCD_H


extern unsigned char timeOut;
extern unsigned int txCount;
extern unsigned int rxCount;
extern unsigned int spiReady;



#define SPI_XFER_BEGIN		0x01 /**< assert SPI CS signal  */
#define SPI_XFER_END		0x02 /**< deassert SPI CS signal  */

#define SPI_PHASE_IN	0x0010 /**< Output SPI data half cycle before clock edge  */
#define SPI_CLK_INV		0x0020 /**< SPI clock inverted polarity (1 when idle)  */
#define SPI_CS_HOLD		0x0040 /**< SPI chip select hold   */

#define EE_CS 0x00 


#define FOUR_MUX 0x04
#define	NO_MUX 0x00
#define MUX NO_MUX

// SmaRTClock choices
#define CP240X_SELFOSC                          0
#define CP240X_CRYSTAL                          1
#define CP240X_CMOSCLK                          2

#define ALLOW_STOPWATCH_CALLBACKS        0

#define CP240X_SMARTCLOCK_ENABLED        1

#define CP240X_RTC_CLKSRC   CP240X_CMOSCLK

// Crystal Load Capacitance
#define CP240X_LOADCAP_VALUE  9        //  0 =  4.0 pf,  1 =  4.5 pf
                                       //  2 =  5.0 pf,  3 =  5.5 pf
                                       //  4 =  6.0 pf,  5 =  6.5 pf
                                       //  6 =  7.0 pf,  7 =  7.5 pf 
                                       //  8 =  8.0 pf,  9 =  8.5 pf
                                       // 10 =  9.0 pf, 11 =  9.5 pf
                                       // 12 = 10.5 pf, 13 = 11.5 pf
                                       // 14 = 12.5 pf, 15 = 13.5 pf

#define CP240X_NUM_PORTREGS_CONFIG   30

#if CP240X_RTC_CLKSRC == CP240X_SELFOSC
   #define CP240X_RTCCLK   20000//31500
#else
   #define CP240X_RTCCLK   32768
#endif

// Bias mode choices
#define HALF_BIAS                               1
#define ONETHIRD_BIAS                           0


#define CP240X_NUM_PORTREGS_CONFIG   30
#define CP240X_NUM_PORTREGS_READ     5



//-----------------------------------------------------------------------------
// MSCN Management
//-----------------------------------------------------------------------------

#define CP240x_MSCN_SetBits(value)   CP240x_MSCN_Update(CP240x_MSCN_Local | value)
#define CP240x_MSCN_ClearBits(value) CP240x_MSCN_Update(CP240x_MSCN_Local & value)

//-----------------------------------------------------------------------------
// LCD Configuration Options
//-----------------------------------------------------------------------------
//

// Define the LCD. Can be { VIM878 / CUSTOMLCD }
#define LCD                        VIM878

// Define the LCD Refresh Rate in Hz
#define REFRESH_RATE_HZ             60

// Define the mux mode. Can be { STATIC, TWO_MUX, THREE_MUX, FOUR_MUX}
#define MUX_MODE                    FOUR_MUX

// Define the Bias. Can be { HALF_BIAS, ONETHIRD_BIAS }
#define BIAS   ONETHIRD_BIAS


typedef struct CP240X_PORTREGS 
{  
   // Writable Registers
   uint8_t P0OUT_Local;  
   uint8_t P1OUT_Local;  
   uint8_t P2OUT_Local;  
   uint8_t P3OUT_Local;  
   uint8_t P4OUT_Local;  
   uint8_t P0MDI_Local;  
   uint8_t P1MDI_Local;  
   uint8_t P2MDI_Local;  
   uint8_t P3MDI_Local;  
   uint8_t P4MDI_Local;  
   uint8_t P0MDO_Local;  
   uint8_t P1MDO_Local;  
   uint8_t P2MDO_Local;  
   uint8_t P3MDO_Local;  
   uint8_t P4MDO_Local;  
   uint8_t P0DRIVE_Local;
   uint8_t P1DRIVE_Local;
   uint8_t P2DRIVE_Local;
   uint8_t P3DRIVE_Local;
   uint8_t P4DRIVE_Local;
   uint8_t P0MATCH_Local;
   uint8_t P1MATCH_Local;
   uint8_t P2MATCH_Local;
   uint8_t P3MATCH_Local;
   uint8_t P4MATCH_Local;
   uint8_t P0MSK_Local;  
   uint8_t P1MSK_Local;  
   uint8_t P2MSK_Local;  
   uint8_t P3MSK_Local;  
   uint8_t P4MSK_Local;
   
   // Read Only Registers
   uint8_t PMATCHST_Local;  
   uint8_t P0IN_Local;      
   uint8_t P1IN_Local;      
   uint8_t P2IN_Local;      
   uint8_t P3IN_Local;      
   uint8_t P4IN_Local;        

} CP240X_PORTREGS;


//-----------------------------------------------------------------------------
// Global Variables 
//-----------------------------------------------------------------------------
static uint8_t CP240x_CLK;                  // Save CP240x clock setting whenever
                                       // entering ULP mode. Will restore
                                       // CP240x clock whenever exiting ULP. 

extern uint16_t CP240x_Measured_RTCCLK;            // Measured value of RTC

extern uint8_t CP240x_MSCN_Local;                  // Local copy of the MSCN register
extern uint8_t CP240x_RTC0CN_Local;                // Local copy of RTC0CN register

                                       // Local copy of PortIO registers
extern CP240X_PORTREGS  CP240x_PortIO_Registers;
									   






//SPI Command Set
#define REGPOLL (0x01)	//Reads data from a single register. Used for polling a status bit.
#define REGREAD 0x02 //Reads one or more bytes from registers with sequential addresses.
#define REGSET 0x03 //Writes one or more bytes to a single register. Used for generating a waveform on a GPIO pin or updating the SmaRTClock registers.
#define REGWRITE (0x04)	//Writes one or more bytes to registers with sequential addresses.
#define RAMREAD 0x06	//Reads one or more bytes from sequential RAM locations.
#define RAMWRITE 0x08	//Writes one or more bytes to sequential RAM locations.



//define config parameters

#define N_PORT 30
#define MAX_DIGITS 6

// LCD control register
#define SIZE BIT3 //LCD size 32 sigment.  
#define MUX_MODE_4 0x06
#define BIAS_1_3 0x00

//Input Mode
#define INPUT_MODE_0 0x00 //analog mode,Port pins configured for analog mode have their weak pullup, digital driver, and digital receiver disabled.
//LCD contrast
#define CONT_3_02 0x08 //charge pump output voltage 3.02v
#define CONT_3_44 0xFF //charge pump output voltage 3.44v
//LCD Configuration
#define RESERVED 0x9F
//Toggle rate
#define TOGGLE_2 0x02
#define TOGGLE_128 0x08
//ULPMEM
#define SIG_ON 0xFF
#define HALF_SIG 0x55
//MSCN
#define LCDEN BIT0
#define RTCBYP BIT7
#define ULPRST BIT1
#define RTCOD BIT4
//LPM
#define NORMAL 0x00




#ifdef KUKU
/*LCD driver define set*/
#define N_PORT 5
#define MAX_DIGITS 6

//Lcd driver registers
#define LCD0CN 0x95 // LCD control register
#define LCD0DIVH 0x98 
#define LCD0DIVL 0x99
#define P0MDI 0xB5 //Port 0 Input Mode
#define P1MDI 0xB6 //Port 1 Input Mode
#define P2MDI 0xB7 //Port 2 Input Mode
#define P3MDI 0xB8 //Port 3 Input Mode
#define P4MDI 0xB9 //Port 4 Input Mode
#define CONTRAST 0x96 //LCD contrast
#define LCD0CF 0x97 //LCD Configuration
#define LCD0TOGR 0x9A //LCD Toggle Rate
#define LCD0PWR 0x9B //LPM


#define MSCN 0xA0


//debug
#define ULPMEM00 0x81 //ULP MEM
#define ULPMEM01 0x82 //ULP MEM
#define ULPMEM02 0x83 //ULP MEM
#define ULPMEM03 0x84 //ULP MEM
#define ULPMEM04 0x85 //ULP MEM
#define ULPMEM05 0x86 //ULP MEM
#define ULPMEM06 0x87 //ULP MEM





#endif

/**
* @struct spiModeBits
* @brief SPI slave access configuration description bits.
*
*/
struct spiModeBits
{
	unsigned int charLen:4; /**< character length. 0: 1 bit .... 15: 16 bits  */
	unsigned int phaseIn:1; /**< SPI data phase in  */
	unsigned int invClk:1;	/**< SPI clock inverted polarity (1 when idle)  */
	unsigned int csHold:1;	/**< SPI chip select hold  */
	unsigned int rsvd:1;
	unsigned int wDelay:8; /**< FIFO intercharacter delay  */
};

/**
* @union spiMode
* @brief SPI slave access configuration description.
*
*/
union spiMode
{
	unsigned int all;	/**< Slave access mode. All bits as a whole  */
	struct spiModeBits bit;	/**< Slave access mode bits  */
};

/**
* @struct spiSlave
* @brief SPI slave description.
*
*/
struct spiSlave
{
	unsigned int cs;				/**< Chip select pin number.  */
	union spiMode mode;				/**< Slave device char. len./clock/phase/intercharacter delay mode.  */
	unsigned long fClock;			/**< Slave device clock frequency.  */
};




extern unsigned char lcdCont;


void *intToBcd(long int bin,char* bcd, unsigned int len);

void spiInit(void);
void spiLcdInit(void);
void spiTranfer(const void *dout, unsigned int xferLen,uint8_t flags);
int spiXfer(const void *dout, void *din, unsigned int xferLen,uint8_t flags);
void CP240x_RegWrite(unsigned char address, unsigned char data);
uint8_t CP240x_RegRead(unsigned char address);
void CP240x_RegBlockWrite(unsigned char address, unsigned char *data,uint8_t len);
void CP240x_RegBlockRead(unsigned char address, unsigned char *data,uint8_t len);
void CP240x_RTC_Write (uint8_t addr, uint8_t value);
uint8_t CP240x_RTC_Read (uint8_t addr);
void CP240x_PortIO_Configure(void);
void CP240x_PortIO_Read(void);
void CP240x_MSCN_Update (uint8_t value);
void CP240xEnterLpm(void);
void CP240xExitLpm(void);







#endif
