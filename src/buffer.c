/**
* @file buffer.c
* @brief buffers pool
*
* @author Eli Schneider
*
* @version 0.0.1
* @date 22.02.2010
*/
#include <stddef.h>
#include "buffer.h"




/**
* @fn void poolInit(BUF_POOL *pool, void *mem, size_t bufSize, size_t nBuffers)
*
* @brief This function initializes a buffers pool.
* 
* @param pool - pointer to a buffer pool. A NULL pointer means no pool is accessed.
* @param mem - pointer to a buffer memory area. A NULL pointer means no memory is assigned.
* @param bufSize - buffer size.
* @param nBuffers - number of buffers.
*
* @author Eli Schneider
*
* @date 22.02.2010
*/
void poolInit(BUF_POOL *pool, void *mem, size_t bufSize, size_t nBuffers)
{
	unsigned int intSave;
	struct sBufHeader *p;
	size_t idx;
	
	if (pool==NULL)
		return;
	
	intSave=__disable_interrupts();
	pool->top=NULL;
	__restore_interrupts(intSave);
	if ((mem==NULL) || (bufSize==0))
		return;
	for (p=(struct sBufHeader *)mem, idx=0; idx<nBuffers; idx++) 
	{
		intSave=__disable_interrupts();
		p->next=pool->top;
		pool->top=p;
		__restore_interrupts(intSave);
		p=(struct sBufHeader *)&((char *)p)[bufSize];
	}
		
	
}

/**
* @fn void *getBuf(BUF_POOL *pool)
*
* @brief This function gets a buffer from a pool
* 
* @param pool - pointer to a buffer pool. A NULL pointer means no pool is accessed.
*
* @return - pointer to buffer. NULL= no buffers available.
*
* @author Eli Schneider
*
* @date 22.02.2010
*/
void *getBuf(BUF_POOL *pool)
{
	unsigned int intSave;
	struct sBufHeader *p;

	if (pool==NULL)
		return NULL;
	
	intSave=__disable_interrupts();
	p=pool->top;
	if (p)
		pool->top=p->next;
	__restore_interrupts(intSave);
	
	return (void *)p;
}

/**
* @fn void *retBuf(BUF_POOL *pool, void *p)
*
* @brief This function returns a buffer to a pool
* 
* @param pool - pointer to a buffer pool. A NULL pointer means no pool is accessed.
*
* @return - pointer to buffer. NULL= no buffers available.
*
* @author Eli Schneider
*
* @date 22.02.2010
*/
void retBuf(BUF_POOL *pool, void *p)
{
	unsigned int intSave;
	if (pool==NULL)
		return;
	if (p==NULL)
		return;
	intSave=__disable_interrupts();
	((struct sBufHeader *)p)->next=pool->top;
	pool->top=(struct sBufHeader *)p;
	__restore_interrupts(intSave);
}


