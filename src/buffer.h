/**
* @file buffer.h
* @brief SPI driver
*
* @author Eli Schneider
*
* @version 0.0.1
* @date 22.02.2010
*/
#ifndef _BUFFER_H
#define _BUFFER_H

#include <stddef.h>

/**
* @struct sBufHeader
* @brief Buffer header.
*
*/
struct sBufHeader
{
	struct sBufHeader *next;	/**< pointer to next buffer. NULL points to nothing */	
};

/**
* @struct sBufPool
* @brief Buffers pool .
*
*/
struct sBufPool
{
	struct sBufHeader *top;	/**< pool top element */
};

/**
* @typedef struct sBufPool BUF_POOL
* @brief Queue type.
*
*/
typedef struct sBufPool BUF_POOL;

void poolInit(BUF_POOL *pool, void *mem, size_t bufSize, size_t nBuffers);
void *getBuf(BUF_POOL *pool);
void retBuf(BUF_POOL *pool, void *p);


#endif

