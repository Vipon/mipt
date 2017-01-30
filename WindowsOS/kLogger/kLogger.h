/*
    Copyright (c) 2016 Valery Konychev. All rights reserved.
	
	If your have comments or corrects please write me on e-mail:
	valerakonychev@gmail.com
*/

#ifndef _K_LOGGER
#define _K_LOGGER

/* winLibs */
#include <ntddk.h>
//#pragma comment(lib, "NtDll.lib")

#include <Wdm.h>
//#pragma comment(lib, "Hal.lib")

/* Defines */
#define print(str, ...) DbgPrint(str, __VA_ARGS__)

//********************************************************************
#define _PROTECTMACRO(macro)		do{macro;}while(0);
#define _PASSERT(point, message)	_PROTECTMACRO(ASSERT(point && message););
#define _CASSERT(cond)				_PROTECTMACRO(ASSERT(cond););
#define _MALLOC(num)				ExAllocatePoolWithTag(NonPagedPool, num, 'Tag1') /* NonPage from any IRQL. */
#define _FREE(point)				ExFreePoolWithTag(point, 'Tag1')
#define _CloseHandle(hand)			ZwClose(hand)
#define _CopyMemory(dest, src, n)	RtlCopyMemory(dest, src, n)
#define _WriteFile(	handle,         \
					iostatus,       \
					buf,            \
					BytesToWrite)   \
				\
				ZwWriteFile(handle,         \
							NULL,           \
							NULL,           \
							NULL,           \
                            iostatus,       \
							buf,            \
							BytesToWrite,   \
							NULL,           \
							NULL)

#define _CreateThread(  phThread,       \
                        threadFunction, \
                        threadParam )   \
                \
                PsCreateSystemThread(   phThread,           \
                                        THREAD_ALL_ACCESS,  \
                                        NULL,               \
                                        NULL,               \
                                        NULL,               \
                                        threadFunction,     \
                                        threadParam )  

#define _CreateFile(    phFile,         \
                        pAttributes,    \
                        pIoStatus   )   \
                \
                ZwCreateFile(   phFile,                         \
                                GENERIC_WRITE,                  \
                                pAttributes,                    \
                                pIoStatus,                      \
                                NULL,                           \
                                FILE_ATTRIBUTE_NORMAL,	        \
                                FILE_SHARE_READ,                \
                                FILE_OVERWRITE_IF,              \
                                FILE_SYNCHRONOUS_IO_NONALERT,   \
                                NULL,                           \
                                0   )

#define _InitializeEvent(	pkEvent,    \
                            event_type, \
                            state   )   \
                \
                KeInitializeEvent(	pkEvent,    \
                                    event_type, \
                                    state   )

#define _InitializeWaitableTimer(pkTimer)   \
                \
                KeInitializeTimerEx(    pkTimer,    \
                    /* TIMER_TYPE */    SynchronizationTimer    );			

#define _SetEvent(Event)	KeSetEvent(Event, IO_NO_INCREMENT ,FALSE)
#define _ResetEvent(Event)	KeResetEvent(Event)
#define _SetWaitableTimer(  pkTimer,    \
        /*LARGE_INTEGER*/   LiDueTime,  \
        /*LONG (msec)  */   lPeriod     )   \
                \
                KeSetTimerEx(   pkTimer,    \
                                LiDueTime,  \
                                lPeriod,    \
                                NULL    )

#define _WaitForSingleObject(pvObject)  \
                KeWaitForSingleObject(  pvObject,   \
                                        Executive,  \
                                        KernelMode, \
                                        FALSE,      \
                                        NULL)

#define _WaitForMultipleObjects(    ulCount,    \
                                    phObjects,  \
                                    WaitType)   \
                \
                KeWaitForMultipleObjects(   ulCount,    \
                                            phObjects,  \
                                            WaitType,   \
                                            Executive,  \
                                            KernelMode, \
                                            TRUE,       \
/* Point to timeout, Null = infinity. */    NULL,       \
                                            NULL)

#define PAGESIZE					4096
//********************************************************************

typedef enum {  EMPTY = 0, 
                PART_FILL, 
                FULL}	KLogState;
typedef enum {  FLUSH_FUNC = 0,
                FLUSH_TIMER,
                FLUSH_EXIT, 
                NUM_FLUSH_EVENTS    /*Must be the last.*/} FlushEvent;

typedef struct {
    CHAR    *buf;

    /*  RingBuf state.  */
    KIRQL   irql;       /*  Work irql.          */
    size_t  size;       /*  Won't be changed.   */
    size_t  read_p;		
    size_t  write_p;
    KLogState state;

    /*  Handlers for read/write synchronization.  */
    LONG    lWriters;
    LONG    lReaders;
    KEVENT  kReadLock;
    KEVENT  kWriteLock;

    /*  Handle for RingBuf Flush Lock. Used in write function.
        If we write we can't flush. */
    KEVENT  kFlushLock;

    /*  Handlers for flush. */
    KEVENT  kExit;          /* If set => that flush thead is dead.  */
    KEVENT  kFlushExit;     /* We want to desctruct klog and exit.  */
    KEVENT  kFlushFunc;     /* We want to flush klog into file.     */
    KTIMER  kFlushTimer;    /* Timer for flush klog.                */
    /* Array of points on the previous events. Need for WaitForMultipleObjects. */ 
    PVOID   pvFlushEvents[NUM_FLUSH_EVENTS];
    HANDLE  hLogFile;       /* Log file Handle for flush.           */ 
    HANDLE  hFlushThread;
} KLog;

KLog *constructKLog(size_t order, const PWSTR log);
void destructKLog(KLog *rb);

size_t writeInKLog(KLog *rb, const void *data, size_t n);
VOID flushKLog(KLog*rb);

#endif  /*  _K_LOGGER   */