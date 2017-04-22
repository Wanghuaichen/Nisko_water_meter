/**
* @file at25xx0a.c
* @brief Atmel AT25xx0a  SPI EEPROM driver.
*
* @author Eli Schneider
*
* @version 0.0.1
* @date 24.01.2010
*
*/
#include "at25xx0a.h"
#include "systicks.h"

#define ATMEL_ID_AT25080A	0
#define ATMEL_ID_AT25160A	1
#define ATMEL_ID_AT25320A	2
#define ATMEL_ID_AT25640A	3

#ifndef EEPROM_ID
#define EEPROM_ID ATMEL_ID_AT25640A
#endif


#define CMD_WRSR	0x01	/**< Write Status Register command code.  */
#define CMD_WRITE	0x02	/**< Write Status Register command code.  */
#define CMD_READ	0x03	/**< Read command code.  */
#define CMD_WRDI	0x04	/**< Write Disable command code.  */
#define CMD_RDSR	0x05	/**< Read Status Register command code.  */
#define CMD_WREN	0x06	/**< Write Enable command code.  */


#define SR_BUSY		0x0001	/**< EEPROM busy mask.  */
#define SR_WEN		0x0002	/**< EEPROM Write Enable mask.  */
#define SR_BP0		0x0004	/**< EEPROM block protection bit 0 mask.  */
#define SR_BP1		0x0008	/**< EEPROM block protection bit 1 mask.  */
#define SR_WPEN		0x0080	/**< EEPROM Hardware Write Protect Enable mask.  */

#define AT25XX0_PROG_TIMEOUT	1638	/**< Device programming timeout.  */ // 1638 ticks=50 ms

static const struct spiMemParams at25xxSpiEepromTable[] =
{
	{
		ATMEL_ID_AT25080A,
		32,
		32,
		1,
		"AT25080A"
	},
	{
		ATMEL_ID_AT25160A,
		32,
		64,
		1,
		"AT25160A"
	},
	{
		ATMEL_ID_AT25320A,
		32,
		128,
		1,
		"AT25320A"
	},
	{
		ATMEL_ID_AT25640A,
		32,
		256,
		1,
		"AT25640A"
	}
};


//static int at25xx0WaitReady(struct spiMem *mem, uint32_t timeout, void *fIdle, void *idleArg);
static int at25xx0Write(struct spiMem *mem, uint32_t offset, size_t len, char *buf, void *fIdle, void *idleArg);
static int at25xx0Read(struct spiMem *mem, uint32_t offset, size_t len, char *buf);


/**
* @fn int at25xx0Setup(struct spiMem *m, struct spiSlave *spi)
*
* @brief This function initializes an at25xx0a EEPROM memory descriptor
* 
* @param m - pointer to EEPROM memory descriptor
* @param spi - pointer spi slave descriptor.
*
* @return 0=success, -1= failure
*
* @author Eli Schneider
*
* @date 24.02.2010
*/
int at25xx0Setup(struct spiMem *m, struct spiSlave *spi)
{
	const struct spiMemParams *params;
	unsigned int idx;
	uint16_t idMatch=EEPROM_ID;
	
	if (m==NULL)
		return -1;
	
	for (idx=0;idx< (sizeof(at25xxSpiEepromTable)/sizeof(at25xxSpiEepromTable[0]));idx++)
	{
		params= &at25xxSpiEepromTable[idx]; 
		if (params->idcode == idMatch)
			break;
	}

	if (idx==(sizeof(at25xxSpiEepromTable)/sizeof(at25xxSpiEepromTable[0])))
	{
		m->params=NULL;
		m->spi=spi;
		m->name=NULL;
		m->size=0;
		m->read=NULL;
		m->write=NULL;
		m->erase=NULL;
		return -1;
	}
	
	m->params=params;
	m->spi=spi;
	
	m->name=params->name;
	m->write=at25xx0Write;
	m->erase=NULL;
	m->read=at25xx0Read;
	m->size=params->pageSize * params->pagesPerSector *params->nrSectors;
	return 0;
}

/**
* @fn int at25xx0WaitReady(struct spiMem *mem, uint32_t timeout, void *fIdle, void *idleArg)
*
* @brief This function waits until at25xx0a EEPROM memory is ready for programming
* 
* @param m - pointer to EEPROM memory descriptor
* @param timeout - timeout duration.
* @param fIdle - pointer to idle function.
* @param idleArg - pointer to idle function arguments.
*
* @return 0=success, -1= failure
*
* @author Eli Schneider
*
* @date 25.02.2010
*/
/*static*/ int at25xx0WaitReady(struct spiMem *mem, uint32_t timeout, void *fIdle, void *idleArg)
{
	int ret;
	unsigned char status;
	uint16_t prevTicks;
	uint16_t ticks;
	uint32_t diffTicks;

	ticks=getIsrTicks();
	do
	{
		ret=spiMemCmd(mem->spi, CMD_RDSR, &status, 1);
		if (ret<0)
			return ret;
		if ((status&SR_BUSY)==0)
			break;
		prevTicks=ticks;
		ticks=getIsrTicks();
		diffTicks=ticks-prevTicks;
		timeout -= (diffTicks<=timeout) ? diffTicks : timeout;
		#ifdef POLL_INT_SPI
		spiReleaseBus(mem->spi);
		if (fIdle)
			(*((void (*)(void *))fIdle))(idleArg);
		ret=spiClaimBus(mem->spi);
		if (ret<0)
			return ret;
		#endif
	} while (timeout);
	if (timeout==0)
		return -1;
	return ret;
}

/**
* @fn int at25xx0Write(struct spiMem *mem, uint32_t offset, size_t len, unsigned char *buf, void *fIdle, void *idleArg)
*
* @brief This function writes at25xx0a EEPROM memory 
* 
* @param m - pointer to EEPROM memory descriptor
* @param offset - memory address.
* @param len - number of bytes to be written.
* @param buf - data to be written.
*
* @return 0=success, -1= failure
*
* @author Eli Schneider
*
* @date 25.02.2010
*/
static int at25xx0Write(struct spiMem *mem, uint32_t offset, size_t len, char *buf, void *fIdle, void *idleArg)
{
	uint16_t pageAddr;
	uint16_t byteAddr;
	uint16_t pageSize;
	size_t chunkLen;
	size_t actual;
	uint16_t ticks;
	int ret;
	char cmd[3];
	unsigned char status;

	pageSize=mem->params->pageSize;
	pageAddr=offset/pageSize;
	byteAddr=offset%pageSize;
	#ifdef CLAIM_BUS
	ret=spiClaimBus(mem->spi);
	if (ret)
		return ret;
	#endif
	ticks=getIsrTicks();
	for ( actual=0; actual<len; actual+=chunkLen)
	{
		chunkLen= ((len-actual)<(pageSize-byteAddr)) ? (len-actual) : (pageSize-byteAddr);
		ret=spiMemCmd(mem->spi, CMD_WREN, NULL, 0);
		if (ret<0)
			break;	// Failure to enable memory for write
		ret=spiMemCmd(mem->spi, CMD_RDSR, &status, 1);
		byteAddr=byteAddr+(pageAddr<<5);
		cmd[0]=CMD_WRITE;
		cmd[1]=(byteAddr>>8) & 0xff;
		cmd[2]=byteAddr & 0xff;
		ret=spiMemCmdWrite(mem->spi, cmd, sizeof(cmd), buf + actual, chunkLen); //send cmd and data to spi mem
		if (ret<0)
			break;	// Failure to write
		ret=at25xx0WaitReady(mem, AT25XX0_PROG_TIMEOUT, fIdle, idleArg);
		if (ret<0)
			break; // Page programming timed out
		pageAddr++;
		offset+=chunkLen;
		byteAddr=0;
	}
	// Successfully programmed
	ticks=getIsrTicks()- ticks;
	#ifdef CLAIM_BUS
	spiReleaseBus(mem->spi);
	#endif
	return ret;
}

/**
* @fn int at25xx0Read(struct spiMem *mem, uint32_t offset, size_t len, unsigned char *buf)
*
* @brief This function writes at25xx0a EEPROM memory 
* 
* @param m - pointer to EEPROM memory descriptor
* @param offset - memory address.
* @param len - number of bytes to be written.
* @param buf - data to be written.
*
* @return 0=success, -1= failure
*
* @author Eli Schneider
*
* @date 25.02.2010
*/
static int at25xx0Read(struct spiMem *mem, uint32_t offset, size_t len, char *buf)
{
	char cmd[3];

	cmd[0]=CMD_READ;
	cmd[1]=(offset>>8) & 0xff;
	cmd[2]=offset & 0xff;
	return spiMemReadCommon(mem, cmd, sizeof(cmd), buf, len);
}


