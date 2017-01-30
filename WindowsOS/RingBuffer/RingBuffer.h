/*
    Copyright (c) 2016 Valery Konychev. All rights reserved.
	
	If your have comments or corrects please write me on e-mail:
	valerakonychev@gmail.com
*/

#ifndef _RING_BUFFER
#define _RING_BUFFER

/* winLibs */
#include <windows.h>
#include <Winternl.h>

/* stdCLibs*/
#include <stdint.h>

typedef enum {  EMPTY = 0, 
                PART_FILL, 
                FULL}	RingBufState;
typedef enum {  FLUSH_FUNC = 0, 
                FLUSH_TIMER, 
                FLUSH_EXIT, 
                NUM_FLUSH_EVENTS	/*Must be the last.*/} FlushEvent;

typedef struct {
	uint8_t *buf;

	/* RingBuf state.	*/
	size_t	size;		/* Won't be changed.	*/
	size_t	read_p;		
	size_t	write_p;
	RingBufState state;

	/* Handlers for read/write synch.	*/
	DWORD	dwWriters;
	DWORD	dwReaders;
	HANDLE	hReadLock;
	HANDLE	hWriteLock;

	/*	Handle for RingBuf Flush Lock. Used in write function.
		If we write we can't flush.	*/
	HANDLE  hFlushLock;

	/* Handle for global RingBuf Lock.	*/
	HANDLE  hLock;

	/* Handlers for flush.	*/
	HANDLE	hExit;
	HANDLE	hLogFile;
	HANDLE	hFlushEvents[NUM_FLUSH_EVENTS];
	HANDLE	hFlushThread;
} RingBuf;

RingBuf *constructRingBuf(size_t order, const TCHAR *log);
void destructRingBuf(RingBuf *rb);

size_t writeInRingBuf(RingBuf *rb, const void *data, size_t n);
BOOL flushRingBuf(RingBuf *rb);

#endif //_RING_BUFFER