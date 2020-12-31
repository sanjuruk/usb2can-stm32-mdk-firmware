
#include "CQueue.h"

void InitialQueue(Queue *queue)
{
	q_int i;
	queue->head = 0;
	queue->tail = 0;
	queue->maxsize = MAX_QUEUE_SIZE;
	for(i=0 ; i<MAX_QUEUE_SIZE ; i++)
		queue->pBuf[i]= 0;
	QUEUE_CREATELOCK(queue);
}

void ReleaseQueue(Queue *queue)
{
	QUEUE_RELEASELOCK(queue);
}

q_int QueuePush(Queue *queue, QUEUE_ELEMENT buf)
{
	q_int nRealTail;
	QUEUE_LOCK(queue);

	//(queue->tail + 1) % queue->maxsize
	nRealTail = queue->tail + 1;
	if(nRealTail >= queue->maxsize )
		nRealTail -= queue->maxsize;

	//is full ?
	if(nRealTail == queue->head )
		return 0;
	//OK push data
	queue->pBuf[queue->tail] = buf;
	//queue->tail = (queue->tail + 1) % queue->maxsize;
	queue->tail = nRealTail;

	QUEUE_ULOCK(queue);

	return 1;
}

QUEUE_ELEMENT QueuePop(Queue *queue)
{
	QUEUE_ELEMENT pRet;
	QUEUE_LOCK(queue);

	// is empty?
	if(queue->tail == queue->head)
		return 0;
	// OK pop data
	pRet = queue->pBuf[queue->head];
	//queue->head = (queue->head + 1) % queue->maxsize;
	queue->head++;
	if(queue->head >= queue->maxsize)
		queue->head -= queue->maxsize;

	QUEUE_ULOCK(queue);
	return pRet;
}

q_int QueueSize(Queue *queue)
{
	q_int nRet;
	QUEUE_LOCK(queue);

	//nRet = (m_tail - m_head + m_maxsize) % m_maxsize ;
	nRet = queue->tail - queue->head + queue->maxsize;
	if(nRet >= queue->maxsize)
		nRet -= queue->maxsize;

	QUEUE_ULOCK(queue);
	return nRet;
}
