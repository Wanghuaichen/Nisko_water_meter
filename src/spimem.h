/**
* @file spimem.h
* @brief SPI memory driver.
*
* @author Eli Schneider
*
* @version 0.0.1
* @date 24.01.2010
*
*/
#ifndef _SPIMEM_H
#define _SPIMEM_H

#include <stddef.h>
#include <stdint.h>
//#include "f280xspi.h"
#include "spiLcd.h"

/**
* @struct spiMemParams
* @brief SPI memory device parameters.
*
*/
#define DELTA_SIZE (2)
#define DATA_POINTS (4000)
#define EEPROM_SIZE 8192
#define MAX_DELTA_ADRESS (EEPROM_SIZE-DELTA_SIZE)
#define START_DELTA_ADRESS (EEPROM_SIZE-(DATA_POINTS*DELTA_SIZE))

struct spiMemParams
{
	uint16_t	idcode;				/**< device ID code */
	uint16_t	pageSize;			/**< page size (in bytes) */
	uint16_t	pagesPerSector;		/**< number of pages in a sector */
	uint16_t	nrSectors;			/**< number of sectors */
	const char *name;				/**< device name */
};

/**
* @struct spiMem
* @brief SPI memory device descriptor.
*
*/
struct spiMem
{
	struct spiSlave *spi;			/**< pointer to spi slave descriptor */
	const char *name;				/**< memory device name string */
	uint32_t size;					/**< memory size */
	const struct spiMemParams *params; /**< pointer to memory parameters */
	int (*read)(struct spiMem *m, uint32_t offset, size_t len, char *buf);	/**< pointer to read function */	
	int (*write)(struct spiMem *m, uint32_t offset, size_t len, char *buf, void *fIdle, void *idleArgs);	/**< pointer to write function */
	int (*erase)(struct spiMem *m, uint32_t offset, size_t len);				/**< pointer to erase function */
};

struct spiMemData
{
	unsigned int offset; /**< current data offset (circular mem, n=4000)>*/
	unsigned int writeCycles; /**<( total data points/n)*/
	int delta;/**<   a(n)-a(n-1)   >*/
	unsigned int lastModCount; /**<   a(n-1)   >*/
};

extern struct spiMemData mem1;

int spiMemCmd(struct spiSlave *spi, char cmd, void *response, size_t len);
int spiMemCmdRead(struct spiSlave *spi, const char *cmd, size_t cmdLen, void *data, size_t dataLen);
int spiMemCmdWrite(struct spiSlave *spi, const char *cmd, size_t cmdLen, void *data, size_t dataLen);
int spiMemReadCommon(struct spiMem *m, const char *cmd, size_t cmdLen, void *data, size_t dataLen);



void spiMemWriteDelta(void);
int spiMemReadDelta(unsigned int offset);
void spiMemReadRawDelta(unsigned int offset,char* buff);
void spiMemSendDelta(unsigned int offset,unsigned char* buff);


#endif

