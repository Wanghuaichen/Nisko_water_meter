/**
* @file queue.h
* @brief queue
*
* @author Eli Schneider
*
* @version 0.0.1
* @date 15.02.2010
*/
#ifndef _QUEUE_H
#define _QUEUE_H


/**
* @struct sQHeader
* @brief Queue element header.
*
*/
struct sQHeader
{
	struct sQHeader *next;	/**< pointer to next message. NULL points to nothing */	
};

/**
* @struct sQueue
* @brief Queue.
*
*/
struct sQueue
{
	struct sQHeader *head;	/**< queue head */
	struct sQHeader *tail;	/**< queue tail */
};

/**
* @typedef struct sQueue QUEUE
* @brief Queue type.
*
*/
typedef struct sQueue QUEUE;

#define queueEmpty(q) ((q)->head==NULL)/**< queue empty tester */

void queueInit(QUEUE *q);
struct sQHeader *queueOut(QUEUE *q);
void queueIn(QUEUE *q, struct sQHeader *p);


#endif


