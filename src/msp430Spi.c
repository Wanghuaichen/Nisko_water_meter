/**
* @file msp430Spi.c
* @brief SPI driver
*
* @author Shlomi Dar
*
* @version 0.0.1
* @date 11.07.2011

Initializing or Re-Configuring the USCI Module
The recommended USCI initialization/re-configuration process is:
1. Set UCSWRST (BIS.B #UCSWRST,&UCxCTL1)
2. Initialize all USCI registers with UCSWRST=1 (including UCxCT
3. Configure ports
4. Clear UCSWRST via software (BIC.B #UCSWRST,&UCxCTL1)
5. Enable interrupts (optional) via UCxRXIE and/or UCxTXIE

*/


#include <msp430.h>
//#include "f280xbmsk.h"
#include "msp430Spi.h"
#include "board.h"
#include "buffer.h"
//#include "parameter.h"

//#define DEF_SPI_FREQUENCY 25000000


struct spiIoPacket spiIoPktMem[N_SPI_IO_PACKETS];

struct spiDev spiDevice = {NULL,{NULL,NULL},{NULL,NULL}};

// Functions that will be run from RAM need to be assigned to 
// a different section.  This section will then be mapped to a load and 
// run address using the linker cmd file.

#pragma CODE_SECTION(spiIntSend, "ramfuncs");
#pragma CODE_SECTION(spiIntSendReceive, "ramfuncs");




/**
* @fn void spiInit(void)
*
* @brief This function initalizes SPI-Serial Peripheral Interface  in master mode.
*
* @author Eli Schneider
*
* @date 14.02.2010
*/
void spiInit(void)
{
	struct spiIoPacket *p;
	unsigned int intSave;

	P3SEL |= 0x0F; //for port 3.0,1,2,3, Primary peripheral module function is selected
	P3DIR |= 0x0B;// 3.0,1,3 is out
	UCB0CTL0 |=UCMST + UCMODE_2 + UCSYNC;
	UCB0CTL1 |=UCSSEL_2+UCSWRST;
	UCB0BR0 |=0x04;
	UCB0BR1 |=0x00;

	
	intSave=__disable_interrupts();
	spiDevice.currentSlave=NULL;
	queueInit(&spiDevice.sendQ);
	queueInit(&spiDevice.freeQ);
	__restore_interrupts(intSave);
	
	for (p=spiIoPktMem;p!= &spiIoPktMem[N_SPI_IO_PACKETS];p= &p[1])
	{
		p->h.next=NULL;
		intSave=__disable_interrupts();
		queueIn(&spiDevice.freeQ, &p->h);
		__restore_interrupts(intSave);
		
	}
	

}


int spiLcdTranfer(unsigned int len, void *dout)
{
	unsigned int txCount;
	unsigned int rxCount;
	volatile char tmp;
	volatile unsigned int blen=8;

	#ifdef CS_HOLD
	P3OUT &=~BIT3;	// Set CS to selected level
	#endif
			
	txCount=0;

	len=(len/blen) + ((len%blen) ? 1 : 0);
	
	while (txCount<len)
	{
		if (IFG2 & UCA0TXIFG)//TXBUF ready? //(SpiaRegs.SPIFFTX.bit.TXFFST<16)
		{
			tmp= (dout==NULL) ? 0 : ((unsigned char *)dout)[txCount])//((unsigned int *)dout)[txCount]<<(15-SpiaRegs.SPICCR.bit.SPICHAR);
			UCB0TXBUF=tmp;
			txCount++;			
		}
	}

	#ifdef CS_HOLD
	P3OUT |= BIT3;	// Set to unselected level
	#endif
	
	return 0;
}


/**
* @fn int spiClaimBus(struct spiSlave *slave)
*
* @brief This function claims a SPI bus.
*
* @param slave - pointer to the slave device description structure
*
* @return - 0 = success, -1 = fail 
*
* @author Eli Schneider
*
* @date 15.02.2010
*
*/
int spiClaimBus(struct spiSlave *slave)
{
	if (slave==NULL)
		return -1;
	if (SPI_N_CS<=slave->cs)
		return -1;
	if (spiDevice.currentSlave)
		return -1;
	spiDevice.currentSlave=slave;


	// Set SPI configuration
	SpiaRegs.SPICCR.bit.SPICHAR=slave->mode.bit.charLen;
	SpiaRegs.SPICCR.bit.CLKPOLARITY=slave->mode.bit.invClk;
	SpiaRegs.SPICTL.bit.CLK_PHASE=slave->mode.bit.phaseIn;
	SpiaRegs.SPIFFCT.bit.TXDLY=slave->mode.bit.wDelay;
	
	// Set SPI speed
	SpiaRegs.SPIBRR =(Uint16)(lspClkRate()/((slave->fClock) ? slave->fClock : DEF_SPI_FREQUENCY))-1;										

	GpioDataRegs.GPASET.bit.GPIO19=1;	// Set to unselected level
	if (slave->mode.bit.csHold)
	{
		EALLOW;
		GpioCtrlRegs.GPADIR.bit.GPIO19=1;	 // Set as output
		GpioCtrlRegs.GPAMUX2.bit.GPIO19=0;	// Select GPIOs to be GPIO pins	 - STE (CS) A
		EDIS;
	}
	else
	{
		EALLOW;
		GpioCtrlRegs.GPAMUX2.bit.GPIO19=1;	// Select GPIOs to be SPI pins	 - STE (CS) A
		GpioCtrlRegs.GPADIR.bit.GPIO19=0;	 // Set as input
		EDIS;
	}
		
	// Select device chip select
	switch (slave->cs)
	{
	 case SPI_CS0:
		GpioDataRegs.GPBCLEAR.all=BMASK(bGPIO33)|BMASK(bGPIO32); // Select SPI_CS0
	 	break;
	 case SPI_CS1:
		GpioDataRegs.GPBSET.all=BMASK(bGPIO33); // Select SPI_CS1
		GpioDataRegs.GPBCLEAR.all=BMASK(bGPIO32);
	 	break;
	 case SPI_CS2:
		GpioDataRegs.GPBSET.all=BMASK(bGPIO33)|BMASK(bGPIO32); // Select SPI_CS2
	 	break;
	 default:
		GpioDataRegs.GPBSET.all=BMASK(bGPIO33)|BMASK(bGPIO32); // Select SPI_CS2
	 	break;
	}

	return 0;
}


