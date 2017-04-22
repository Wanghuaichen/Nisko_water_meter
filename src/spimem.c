/**
* @file spimem.c
* @brief SPI memory driver.
*
* @author Eli Schneider
*
* @version 0.0.1
* @date 24.01.2010
*
*/
#include "spimem.h"
#include "board.h"
#include "nvm.h"



struct spiMemData mem1;

/**
* @fn int spiMemCmd(struct spiSlave *spi, char cmd, void *response, size_t len)
*
* @brief This function sends a command to spi memory.
* 
* @param spi - pointer spi slave descriptor.
* @param cmd - command
* @param response - pointer response buffer.
* @param len - response length.
*
* @return 0=success, -1= failure
*
* @author Eli Schneider
*
* @date 24.02.2010
*/
int spiMemCmd(struct spiSlave *spi, char cmd, void *response, size_t len)
{
	unsigned int flags=SPI_XFER_BEGIN;
	int ret;

	if (len==0)
		flags|= SPI_XFER_END;

	// send command
	ret= spiXfer(&cmd,NULL ,1, flags);
	if (ret)
		return ret;
	// get response
	if (len)
		ret=spiXfer( NULL, response,len, SPI_XFER_END);
	return ret;
}

/**
* @fn int spiMemCmdRead(struct spiSlave *spi, const char *cmd, size_t cmdLen, void *data, size_t dataLen)
*
* @brief This function reads a memory block from spi memory
* 
* @param spi - pointer spi slave descriptor.
* @param cmd - pointer to command
* @param cmdLen - command length.
* @param data - pointer to data buffer.
* @param dataLen - data length.
*
* @return 0=success, -1= failure
*
* @author Eli Schneider
*
* @date 24.02.2010
*/
int spiMemCmdRead(struct spiSlave *spi, const char *cmd, size_t cmdLen, void *data, size_t dataLen)
{
	unsigned int flags=SPI_XFER_BEGIN;
	int ret;

	if (dataLen==0)
		flags|= SPI_XFER_END;
	ret=spiXfer(cmd, NULL, cmdLen,flags);
	if (ret==0)
	{
		if (dataLen)
			ret=spiXfer(NULL, data, dataLen, SPI_XFER_END);
	}
	return ret;
}

/**
* @fn int spiMemCmdWrite(struct spiSlave *spi, const char *cmd, size_t cmdLen, void *data, size_t dataLen)
*
* @brief This function writes a memory block to spi memory
* 
* 
* @param spi - pointer spi slave descriptor.
* @param cmd - pointer to command
* @param cmdLen - command length.
* @param data - pointer to data buffer.
* @param dataLen - data length.
*
* @return 0=success, -1= failure
*
* @author Eli Schneider
*
* @date 24.02.2010
*/
int spiMemCmdWrite(struct spiSlave *spi, const char *cmd, size_t cmdLen, void *data, size_t dataLen)
{
	unsigned int flags=SPI_XFER_BEGIN;
	int ret;

	if (dataLen==0)
		flags|= SPI_XFER_END;
	ret=spiXfer(cmd, NULL,cmdLen, flags);
	if (ret==0)
	{
		if (dataLen)
			ret=spiXfer(data, NULL, dataLen, SPI_XFER_END);
	}
	return ret;
}

/**
* @fn int spiMemReadCommon(struct spiSlave *spi, const char *cmd, size_t cmdLen, void *data, size_t dataLen)
*
* @brief This function reads a memory block from spi memory
* 
* @param spi - pointer spi slave descriptor.
* @param cmd - pointer to command
* @param cmdLen - command length.
* @param data - pointer to data buffer.
* @param dataLen - data length.
*
* @return 0=success, -1= failure
*
* @author Eli Schneider
*
* @date 24.02.2010
*/
int spiMemReadCommon(struct spiMem *m, const char *cmd, size_t cmdLen, void *data, size_t dataLen)
{
	struct spiSlave *spi = m->spi;
	int ret;

//	spiClaimBus(spi);
	ret=spiMemCmdRead(spi, cmd, cmdLen, data, dataLen);
//	spiReleaseBus(spi);
	
	return ret;
}


void spiMemWriteDelta(void)
{
	unsigned char tmpBuf[2];
	
	mem1.delta=adcBuffer.dispCounter-mem1.lastModCount; //calc delta
	tmpBuf[1]=(mem1.delta>>8)&0xFF;
	tmpBuf[0]=(mem1.delta)&0xFF;

	if ((mem1.offset)>=MAX_DELTA_ADRESS)	
		mem1.offset=START_DELTA_ADRESS;

	nvm1.write(&nvm1, mem1.offset, 2, tmpBuf, NULL, NULL); //write delta 
	mem1.offset+=2;
}


int spiMemReadDelta(unsigned int offset)
{
	unsigned char tmpBuf[2];
	int val;
	
	nvm1.read(&nvm1, offset, 2, tmpBuf);
	tmpBuf[1]=(mem1.delta>>8)&0xFF;
	tmpBuf[0]=(mem1.delta)&0xFF;

	val=(tmpBuf[1]<<8)+tmpBuf[0];
	return val;
}

void spiMemReadRawDelta(unsigned int offset,char* buff)
{
	unsigned int dataLen=32;
	unsigned int dataOffset=offset;
	unsigned int idx=0;
	unsigned int chankLen;

	if ((offset-32)<MAX_DELTA_ADRESS)
	{
		dataLen=(MAX_DELTA_ADRESS-offset);
		chankLen=32-dataLen;
	}
		
	if (dataLen)
	{
		nvm1.read(&nvm1, dataOffset, dataLen, buff);
	}

	if (chankLen)
	{
		nvm1.read(&nvm1, START_DELTA_ADRESS, chankLen, &buff[dataLen]);
	}
	
}

void spiMemSendDelta(unsigned int offset,unsigned char* buff)
{
}

