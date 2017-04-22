/**
* @file msp430Spi.h
* @brief SPI driver
*
* @author Shlomi Dar
*
* @version 0.0.1
* @date 11.07.2011
*/

#ifndef _MSP430SPI_H
#define _MSP430SPI_H

#include <stdint.h>
#include <stddef.h>
#include "queue.h"

#define SPI_N_CS 3
#define N_SPI_IO_PACKETS 16

#define SPI_XFER_BEGIN		0x0001 /**< assert SPI CS signal  */
#define SPI_XFER_END		0x0002 /**< deassert SPI CS signal  */

#define SPI_PHASE_IN	0x0010 /**< Output SPI data half cycle before clock edge  */
#define SPI_CLK_INV		0x0020 /**< SPI clock inverted polarity (1 when idle)  */
#define SPI_CS_HOLD		0x0040 /**< SPI chip select hold   */


#define SPI_LEN(x) ((x)-1)
#define SPI_MK_WDELAY(x) (((x)&0xff)<<8)

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


/**
* @struct spiDev
* @brief SPI device description.
*
*/
struct spiDev
{
	struct spiSlave *currentSlave;	/**< Current selected slave. NULL=bus is free.  */
	QUEUE sendQ;					/**< Xfer queue  */
	QUEUE freeQ;					/**< free Xfer commands queue  */
};

/**
* @struct spiIoPacket
* @brief SPI IO packet description.
*
*/
struct spiIoPacket
{
	struct sQHeader	h;				/**< link to next IO packet.  */
	struct spiSlave *slave;			/**< link to slave destination.  */
	unsigned int len;				/**< length of IO packet.  */
	unsigned int flags;				/**< length of IO packet transfer flags.  */
	const void * dout;				/**< pointer to data out buffer.  */
	const void *doutPool;			/**< link to pool. NULL = no pool. */		
	void * din;						/**< pointer to data in buffer.  */
	const void *dinPool;			/**< link to pool. NULL = no pool. */
	void (*cbFunc)();				/**< pointer to end of transfer callback function. NULL=no callback. */
	uint32_t arg1;					/**< argument 1 for callback function. */
	void *arg2;						/**< argument 2 for callback function. */
};

#ifdef __cplusplus
extern "C" {
#endif

int spiClaimBus(struct spiSlave *slave);
int spiReleaseBus(struct spiSlave *slave);
int spiXfer(struct spiSlave *slave, unsigned int len, const void *dout, void *din, unsigned int flags);
int spiIntSend(struct spiSlave *slave, unsigned int len, const void *dout, unsigned int flags, const void *doutPool); 
int spiIntSendReceive(struct spiSlave *slave, unsigned int len, const void *dout, void *din, unsigned int flags, const void *doutPool, const void *dinPool, void *cbFunc, uint32_t arg1, void *arg2);

#ifdef __cplusplus
}
#endif /* extern "C" */

#endif




