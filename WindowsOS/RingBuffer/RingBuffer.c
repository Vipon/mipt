/* winLibs */
#include <windows.h>
#include <Winternl.h>

/* stdCLibs*/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/* viponHeaders */
#include "RingBuffer.h"

//********************************************************************
#define _PROTECTMACRO(macro)		do{macro;}while(0);
#define _PASSERT(point, message)	_PROTECTMACRO(assert(point && message););
#define _CASSERT(cond)              _PROTECTMACRO(assert(cond););
#define _MALLOC(num)                malloc(num)
#define _FREE(point)                free(point)
#define _CloseHandle(hand)          CloseHandle(hand)
#define _CopyMemory(dest, src, n)	CopyMemory(dest, src, n)
#define _WriteFile( handler,            \
                    pBuf,               \
                    dwBytesToWrite,     \
                    pDwBytesWritten,    \
/* Usually NULL */	pOverlappedStruct)  \
                    \
                    WriteFile(  handler,			\
                                buf,                \
                                dwBytesToWrite,		\
                                pDwBytesWritten,	\
                                pOverlappedStruct)

#define _CreateThread(	/* Usually NULL */	secAttr,		\
                        /*			0	*/	stackSize,		\
                                            threadFunction,	\
                                            threadParam,	\
                        /*			0	*/	createFlags,	\
                                            pThreadId	)	\
                    \
                    CreateThread(   secAttr,		\
                                    stackSize,		\
                                    threadFunction,	\
                                    threadParam,	\
                                    createFlags,	\
                                    pThreadId	)  

#define _CreateFile(    fileName,               \
                        dwDesiredAccess,        \
                        dwShareMode,            \
                        lpSecurityAttributes,   \
                        dwCreationDisposition,  \
                        dwFlagsAndAttributes,   \
                        hTemplateFile)          \
                    \
                    CreateFile(	fileName,               \
                                dwDesiredAccess,        \
                                dwShareMode,            \
                                lpSecurityAttributes,   \
                                dwCreationDisposition,  \
                                dwFlagsAndAttributes,   \
                                hTemplateFile)

#define _CreateEvent(   PSECURITY_ATTRI헧TES,	\
                        bManualReset,			\
                        bInitialState,			\
                        pszName	)				\
                    \
                    CreateEvent(	PSECURITY_ATTRI헧TES,	\
                                    bManualReset,			\
                                    bInitialState,			\
                                    pszName	);
#define _CreateWaitableTimer(	PSECURITY_ATTRI헧TES,	\
                                bManualReset,			\
                                pszName	)				\
                    \
                    CreateWaitableTimer(	PSECURITY_ATTRI헧TES,	\
                                            bManualReset,			\
                                            pszName	)				\

#define _SetEvent(handle)	SetEvent(handle)
#define _ResetEvent(handle)	ResetEvent(handle)
#define _SetWaitableTimer(  hTimer,						\
        /*LARGE_INTEGER*/	pLiDueTime,					\
        /*LONG (msec)  */	lPeriod,					\
        /*NULL*/			pfnCompletionRoutine,		\
        /*NULL*/			pvArgToCompletionRoutine,	\
        /*FALSE*/			bResume	)					\
                    \
                    SetWaitableTimer(   hTimer,						\
                                        pLiDueTime,					\
                                        lPeriod,					\
                                        pfnCompletionRoutine,		\
                                        pvArgToCompletionRoutine,	\
                                        bResume	)

#define _WaitForSingleObject( hHandle, dwMilliseconds)	\
WaitForSingleObject( hHandle, dwMilliseconds)

#define _WaitForMultipleObjects(	dwCount,		\
                                    phObjects,		\
                                    bWaitAll,		\
                                    dwMilliseconds)	\
                    \
                    WaitForMultipleObjects( dwCount,		\
                                            phObjects,		\
                                            bWaitAll,		\
                                            dwMilliseconds)

#define PAGESIZE					4096
//********************************************************************

/* For this your shoul take readLock. */
static BOOL isEmptyRingBuf(const RingBuf *rb)
{
	_PASSERT(rb, "ERROR: Invalid point to buffer."); 

	return (rb->state == EMPTY);
}

/* For this your shoul take readLock. */
static BOOL isFullRingBuf(const RingBuf *rb)
{
	_PASSERT(rb, "isFullBuf ERROR: Invalid point to buffer."); 

	return (rb->state == FULL);
}

/* For this your shoul take readLock. */
static size_t sizeFreeSpaceInRingBuf(const RingBuf *rb)
{
	_PASSERT(rb, "sizeFreeSpaceInBuf ERROR: Invalid point to buffer."); 

	if (rb->read_p > rb->write_p )
		return rb->read_p - rb->write_p;
	else if (isEmptyRingBuf(rb))
		return rb->size;
	else
		return rb->size - (rb->write_p - rb->read_p);
}

/* For this your shoul take readLock. */
static size_t sizeBusySpaceInRingBuf(const RingBuf *rb)
{
	_PASSERT(rb, "ERROR: Invalid point to buffer."); 

	if (rb->read_p > rb->write_p)
		return rb->size - (rb->read_p - rb->write_p);
	else if (isFullRingBuf(rb))
		return 0;
	else
		return rb->write_p - rb->read_p;
}

static HANDLE hCreateAndSetTimer(DWORD sec)
{
	/* Timers Variables */
	LARGE_INTEGER li;
	HANDLE	hWaitableTimer;
	const int nTimerUnitsPerSecond = 10000000;

	/* If li.QuadPart < 0 relative time start.				*/
	li.QuadPart = 0 - sec * nTimerUnitsPerSecond;	
	hWaitableTimer = _CreateWaitableTimer(NULL, FALSE, NULL);
	_SetWaitableTimer(  hWaitableTimer,
                        &li,
                        sec * 1000,
                        NULL,
                        NULL,
                        FALSE	);

	return hWaitableTimer;
}

static void vReadRingBufLock(RingBuf *rb)
{
	_PASSERT(rb, "iReadRingBufLock ERROR: Invalid point to buffer.");

	/* We could read only after write lock have been freed	*/
	if (_WaitForSingleObject(rb->hWriteLock, INFINITE) == WAIT_FAILED) {
		printf("vReadRingBufLock ERROR: WaitForSingleObject fail code %ld\n",
                                                                GetLastError());
		exit(EXIT_FAILURE);
	}
	/* Get readLock and increase numver of readers */
	_CASSERT(_ResetEvent(rb->hReadLock) && "vReadRingBufLock fail");
	++rb->dwReaders;
	/* Free the next readers. */
	_CASSERT(_SetEvent(rb->hWriteLock) && "vReadRingBufLock fail");
}

static void vReadRingBufUnLock(RingBuf *rb)
{
	_PASSERT(rb, "iReadRingBufUnLock ERROR: Invalid point to buffer.");

	/* Wait while action with rb->dwReaders is end */
	if (_WaitForSingleObject(rb->hWriteLock, INFINITE) == WAIT_FAILED) {
		printf("vReadRingBufUnLock ERROR: WaitForSingleObject fail code %ld\n",
                                                                    GetLastError());
		exit(EXIT_FAILURE);
	}
	/* decrease numver of readers */
	--rb->dwReaders;
	/* if there are not readers, we will free readLock.	*/
	if (rb->dwReaders == 0)
		_CASSERT(_SetEvent(rb->hReadLock) && "vReadRingBufUnLock fail");
	/* Now every read and write could starts.	*/
	_CASSERT(_SetEvent(rb->hWriteLock) && "vReadRingBufUnLock fail");
}

static void vWriteRingBufLock(RingBuf *rb)
{
	const DWORD dwNumLock = 2;
	HANDLE HANDLES[2];
	_PASSERT(rb, "iReadRingBufLock ERROR: Invalid point to buffer.");

	HANDLES[0] = rb->hReadLock;
	HANDLES[1] = rb->hWriteLock;
	/* We could write only after read and write lock have been freed	*/
	if (_WaitForMultipleObjects(dwNumLock, HANDLES, TRUE, INFINITE) == WAIT_FAILED) {
		printf("vWriteRingBufLock ERROR: WaitForMultipleObjects fail code %ld\n", 
                                                                    GetLastError());
		exit(EXIT_FAILURE);
	}
}

static void vWriteRingBufUnLock(RingBuf *rb)
{
	_PASSERT(rb, "iReadRingBufUnLock ERROR: Invalid point to buffer.");

	/* Nobody starts.	*/
	_CASSERT(_SetEvent(rb->hReadLock) && "vWriteRingBufUnLock fail");
	/* Could Start Evebody.	*/
	_CASSERT(_SetEvent(rb->hWriteLock) && "vWriteRingBufUnLock fail");
}

static void vRingBufLock(RingBuf *rb)
{
	_PASSERT(rb, "vRingBufLock ERROR: Invalid point to buffer.");

	if (_WaitForSingleObject(rb->hLock, INFINITE) == WAIT_FAILED) {
		printf("vRingBufLock ERROR: WaitForSingleObject fail code %ld\n",
                                                            GetLastError());
		exit(EXIT_FAILURE);
	}
}

static void vRingBufUnLock(RingBuf *rb)
{
	_PASSERT(rb, "vRingBufUnLock ERROR: Invalid point to buffer.");

	_CASSERT(_SetEvent(rb->hLock) && "vRingBufUnLock fail");
}

static size_t iTakeFromRingBuf(void *dest, RingBuf *rb, size_t num)
{
	size_t	count	= 0;
	size_t	write_p = 0;
	size_t	read_p	= 0;
	BOOL	bufEmpty= FALSE;
	_PASSERT(dest, "iCopyFromRingBuffer ERROR: Invalid point to destination.");
	_PASSERT(rb, "iCopyFromRingBuffer ERROR: Invalid point to RingBuf.");

	/* Get synch for atomic read buffer state.	*/
	vReadRingBufLock(rb);
	bufEmpty= isEmptyRingBuf(rb);
	write_p = rb->write_p;
	read_p  = rb->read_p;
	count = sizeBusySpaceInRingBuf(rb);
	/* Free synch.	*/ 
	vReadRingBufUnLock(rb);

	if (bufEmpty)
		return 0;

	if (num > count) {
		/* We can't to copy more than busy bytes. */
		num = count;
	}

	_WaitForSingleObject(rb->hFlushLock, INFINITE);
	if (write_p > read_p) {
		/* write point >= read point	*/
		count = write_p - read_p;
		if (count >= num) {
			/*	There are characters >= than num in the RingBuffer.
				So we can to copy num character from buffer.	*/
			count = num;
		}

		_CopyMemory(dest, &rb->buf[read_p], count);
		read_p += count;
		read_p %= rb->size;
	} else {
		/* write point <= read point && buffer  isn't empty	*/
		count = rb->size - read_p;
		if (count >= num) {
			/*	There are characters >= than num between end of buffer and read
				pointer. So we can to copy num character from read pointer.	*/
			_CopyMemory(dest, &rb->buf[read_p], num);
			read_p += num;
			read_p %= rb->size;
		} else {
			_CopyMemory(dest, &rb->buf[read_p], count);
			dest = (void*)((size_t)dest + count);
			count = num - count;
			read_p = count;
			_CopyMemory(dest, &rb->buf[0], count);
			
		}
	}

	/* Get synch for atomic write new buffer state.	*/
	vWriteRingBufLock(rb);
	rb->read_p = read_p;
	if (write_p == read_p)
		rb->state = EMPTY;
	else
		rb->state = PART_FILL;
	/* Free synch.	*/
	vWriteRingBufUnLock(rb);

	return num;
}

static DWORD WINAPI FlushThreadFunction(LPVOID lpParam)
{
	size_t i = 0;
	DWORD dwEvent = 0;
	RingBuf* rb = (RingBuf*) lpParam;
	/* Create or Open Files Variables */
	uint8_t buf[PAGESIZE] = {'\0'};
	BOOL bErrorFlag = FALSE;
	DWORD dwBytesWritten = 0;
	DWORD dwBytesToWrite = PAGESIZE;
	_PASSERT(lpParam, "FlushThreadFunction ERROR: Invalid point to paramets."); 
	
	/* Create Auto Reset WaitableTimer. Every 1 seconds.	*/
	rb->hFlushEvents[FLUSH_TIMER] = hCreateAndSetTimer(1);

	for(;;) {
		dwBytesWritten = 0;
		dwEvent = WaitForMultipleObjects(	
                                        NUM_FLUSH_EVENTS,   /* Number Objects	*/	
                                        rb->hFlushEvents,   /* Objects Array	*/
                                        FALSE,              /* bWaitAll	*/
                                        INFINITE	);
		if (dwEvent == WAIT_FAILED) {
			printf("WriteFile ERROR: code %ld\n", GetLastError());
			exit(EXIT_FAILURE);
		}

		dwBytesToWrite = iTakeFromRingBuf(buf, rb, PAGESIZE);
		while (dwBytesWritten != dwBytesToWrite) {
			dwBytesToWrite -= dwBytesWritten;
			bErrorFlag = _WriteFile(rb->hLogFile,	
                                    buf,			
                                    dwBytesToWrite,	
                                    &dwBytesWritten,
                                    NULL);          

			if (bErrorFlag == FALSE) {
				printf("WriteFile ERROR: code %ld\n", GetLastError());
				exit(EXIT_FAILURE);
			}
		}

		if (dwEvent == WAIT_OBJECT_0 + FLUSH_EXIT) {
			/* We need to free destruct thread */
			_SetEvent(rb->hExit);
			return 0;
		}
	}
}

static void createFlushThread(RingBuf *rb)
{
	LPVOID lpThreadParam;

	_PASSERT(rb, "createFlushThread ERROR: Invalid point to buffer."); 
	
	lpThreadParam = (LPVOID) rb;
	rb->hFlushThread = _CreateThread(   NULL,               // default security attributes
                                        0,                  // use default stack size  
                                        FlushThreadFunction,// thread function name
                                        lpThreadParam,      // argument to thread function 
                                        0,                  // use default creation flags 
                                        NULL	);          // returns the thread identifier 

	if(rb->hFlushThread == NULL) {
		_FREE(rb->buf);
		_FREE(rb);
		printf("createFlushThread ERROR: CreateThread fail code %ld\n",
                                                                        GetLastError());
		exit(EXIT_FAILURE);
	}

}

RingBuf *constructRingBuf(size_t order, const TCHAR *log)
{
	RingBuf *rb;
	_PASSERT(log, "ERROR: Invalid point to logFile name.");

	/* Allocate memory for buffer structer. */
	rb = (RingBuf*) _MALLOC(sizeof(RingBuf));
	_PASSERT(rb, "ERROR: Failed to allocate memory.");

	rb->size = PAGESIZE << order;
	rb->buf = (uint8_t*) _MALLOC(rb->size);
	if (rb->buf == NULL) {
		_FREE(rb);
		_PASSERT(NULL, "ERROR: Failed to allocate memory for rb->buf.");
	}
	
	rb->hLogFile = _CreateFile( log,
                                GENERIC_WRITE,
                                FILE_SHARE_READ,
                                NULL,
                                OPEN_ALWAYS,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL	);

	if (rb->hLogFile == INVALID_HANDLE_VALUE) {
		printf("ERROR CreateFile code: %ld\n", GetLastError());
		exit(EXIT_FAILURE);
	}

	/* Initialization other fields */
	rb->read_p = 0;
	rb->write_p = 0;
	rb->dwReaders = 0;
	rb->dwWriters = 0;
	rb->state = EMPTY;
	rb->hExit = _CreateEvent(NULL, FALSE, FALSE, NULL);
	rb->hFlushEvents[FLUSH_FUNC] = _CreateEvent(NULL, FALSE, FALSE, NULL);
	rb->hFlushEvents[FLUSH_EXIT] = _CreateEvent(NULL, FALSE, FALSE, NULL);
	rb->hReadLock = _CreateEvent(NULL, FALSE, TRUE, NULL);
	rb->hWriteLock = _CreateEvent(NULL, FALSE, TRUE, NULL);
	rb->hFlushLock = _CreateEvent(NULL, TRUE, TRUE, NULL);
	rb->hLock =_CreateEvent(NULL, FALSE, TRUE, NULL);

	/* Create flush thread. */
	createFlushThread(rb);
	return rb;
}

void destructRingBuf(RingBuf *rb)
{
	int i = 0;
	_PASSERT(rb, "ERROR: Invalid point to buffer want to free."); 

	_SetEvent(rb->hFlushEvents[FLUSH_EXIT]);
	_WaitForSingleObject(rb->hExit, INFINITE);
	for (i = 0; i < NUM_FLUSH_EVENTS; ++i)
		_CloseHandle(rb->hFlushEvents[i]);

	_CloseHandle(rb->hLock);
	_CloseHandle(rb->hReadLock);
	_CloseHandle(rb->hWriteLock);
	_CloseHandle(rb->hFlushLock);
	_CloseHandle(rb->hFlushThread);
	_CloseHandle(rb->hLogFile);
	_CloseHandle(rb->hExit);
	rb->dwWriters = 0;
	rb->dwReaders = 0;
	rb->write_p = 0;
	rb->read_p = 0;
	rb->state = EMPTY;
	rb->size = 0;
	_FREE(rb->buf);
	_FREE(rb);
}

static void vGetPlaceInRingBuf(RingBuf *rb,  size_t n)
{
	_PASSERT(rb, "ERROR: Invalid point to buffer.");
	rb->write_p += n;
	rb->write_p %= rb->size;
}

size_t writeInRingBuf(RingBuf *rb, const void *data, size_t n)
{
	size_t size_ew = 0;	/* rb->size - rb->write_p	*/
	size_t read_p  = 0;
	size_t write_p = 0;

	if (rb && data == NULL || n > rb->size) {
		_CASSERT("ERROR: Invalid argument in writeInBuf function\n" == 0);
	}

	if (rb->size < n)
		n = rb->size;
	/* We are in the loop while we have not resources */
	while (TRUE) {
		/* Get synch for take and change a ringBuf state.	*/
		vWriteRingBufLock(rb);
		if (sizeFreeSpaceInRingBuf(rb) >= n) {
			read_p  = rb->read_p;
			write_p = rb->write_p;
			/*	Indicates that somebody is writing in buffer and we can't 
				flush.	*/
			++rb->dwWriters;
			_ResetEvent(rb->hFlushLock);

			vGetPlaceInRingBuf(rb, n);
			if (rb->write_p == rb->read_p)
				rb->state = FULL;
			else
				rb->state = PART_FILL;
			break;
		}
		
		flushRingBuf(rb);
		vWriteRingBufUnLock(rb);
		/* Sleep for 100 msec.	*/
		Sleep(100);
	}
	vWriteRingBufUnLock(rb);

	/* Now we have enough space. And we have consistent state. */
	if (write_p >= read_p) {
		/* write point > read point	*/
		size_ew = rb->size - write_p;
		if (size_ew >= n) {
			/*	There is enough space in buffer between write point and end of
				buffer.	*/
			_CopyMemory(&rb->buf[write_p], data, n);
		} else {
			_CopyMemory(&rb->buf[write_p], data, size_ew);
			n -= size_ew;
			data = (const void*)((size_t)data + size_ew);
			_CopyMemory(rb->buf, data, n);
		}
	} else {
		/*	There is enough space in buffer between read point and write
			point.	*/
		_CopyMemory(&rb->buf[write_p], data, n);	
	}

	vWriteRingBufLock(rb);
	--rb->dwWriters;
	if (rb->dwWriters == 0)
		_SetEvent(rb->hFlushLock);
	// free synch
	vWriteRingBufUnLock(rb);
	return n;
}

BOOL flushRingBuf(RingBuf *rb)
{
	_PASSERT(rb, "flush ERROR: Invalid point to buffer.");

	return _SetEvent(rb->hFlushEvents[FLUSH_FUNC]);
}