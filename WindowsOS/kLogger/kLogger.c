/* viponHeaders */
#include "kLogger.h"

/* For this your shoul take readLock. */
static BOOLEAN isEmptyKLog(const KLog *kl)
{
    _PASSERT(kl, "isEmptyKLog ERROR: Invalid point to KLog."); 

    return (kl->state == EMPTY);
}

/* For this your shoul take readLock. */
static BOOLEAN isFullKLog(const KLog *kl)
{
    _PASSERT(kl, "isFullKLog ERROR: Invalid point to KLog ."); 

    return (kl->state == FULL);
}

/* For this your shoul take readLock. */
static size_t sizeFreeSpaceInKLog(const KLog *kl)
{
    _PASSERT(kl, "sizeFreeSpaceInKLog ERROR: Invalid point to KLog."); 

    if (kl->read_p > kl->write_p )
        return kl->read_p - kl->write_p;
    else if (isEmptyKLog(kl))
        return kl->size;
    else
        return kl->size - (kl->write_p - kl->read_p);
}

/* For this your shoul take readLock. */
static size_t sizeBusySpaceInKLog(const KLog *kl)
{
    _PASSERT(kl, "sizeBusySpaceInKLog ERROR: Invalid point to KLog."); 

    if (kl->read_p > kl->write_p)
        return kl->size - (kl->read_p - kl->write_p);
    else if (isFullKLog(kl))
        return 0;
    else
        return kl->write_p - kl->read_p;
}

/* Function must be called only on PASSIVE_LEVEL.   */
static VOID pkInitializeAndSetTimer(LONG sec, PKTIMER pkWaitableTimer)
{
    /* Timers Variables */
    LARGE_INTEGER li;
    const int nTimerUnitsPerSecond = 10000000;

    print("KLog(FLUSH): start pkInitializeAndSetTimer\n");
    /* If li.QuadPart < 0 relative time start.  */
    li.QuadPart = 0 - sec * nTimerUnitsPerSecond;	
    _InitializeWaitableTimer(pkWaitableTimer);
    _SetWaitableTimer(  pkWaitableTimer,
                        li,
                        sec * 1000  );
                        
    print("KLog(FLUSH): end pkInitializeAndSetTimer\n");
}

static VOID vReadKLogLock(KLog *kl)
{
    KIRQL irql;
    NTSTATUS status;
    _PASSERT(kl, "iReadKLogLock ERROR: Invalid point to KLog.");

    print("KLog: start vReadKLogLock\n");
    
    /* We could read only after write lock have been freed	*/
    irql = KeGetCurrentIrql();
    KeLowerIrql(PASSIVE_LEVEL);
    status = _WaitForSingleObject(&(kl->kWriteLock));
    if (!NT_SUCCESS(status)) {
        print("vReadKLogLock ERROR: WaitForSingleObject fail code %ld\n",
                                                                    status);
        _CASSERT(status == STATUS_SUCCESS);
    }

    /* Get readLock and increase numver of readers */
    _ResetEvent(&(kl->kReadLock));
    ++kl->lReaders;
    /* Free the next readers. */
    _SetEvent(&(kl->kWriteLock));
    KeRaiseIrql(irql, &irql);
    
    print("KLog: end vReadKLogLock\n");
}

static VOID vReadKLogUnLock(KLog *kl)
{
    KIRQL irql;
    NTSTATUS status;
    _PASSERT(kl, "vReadKLogUnLock ERROR: Invalid point to KLog.");

    print("KLog: start vReadKLogUnLock\n");
    
    /* Wait while action with rb->dwReaders is end */
    irql = KeGetCurrentIrql();
    KeLowerIrql(PASSIVE_LEVEL);
    status = _WaitForSingleObject(&(kl->kWriteLock));
    if (!NT_SUCCESS(status)) {
        print("vReadKLogUnLock ERROR: WaitForSingleObject fail code %ld\n",
                                                                    status);
        _CASSERT(status == STATUS_SUCCESS);
    }

    /* decrease numver of readers */
    --kl->lReaders;
    /* if there are not readers, we will free readLock. */
    if (kl->lReaders == 0)
        _SetEvent(&(kl->kReadLock));

    /* Now every read and write could starts.   */
    _SetEvent(&(kl->kWriteLock));
    KeRaiseIrql(irql, &irql);
    
    print("KLog: end vReadKLogUnLock\n");
}

static VOID vWriteKLogLock(KLog *kl)
{
    KIRQL irql;
    NTSTATUS status;
    PVOID HANDLES[2];
    const ULONG dwNumLock = 2;
    _PASSERT(kl, "vWriteKLogLock ERROR: Invalid point to KLog.");

    print("KLog: start vWriteKLogLock\n");
    
    HANDLES[0] = &(kl->kReadLock);
    HANDLES[1] = &(kl->kWriteLock);

    /*  We could write only after read and write lock have been freed.  */
    irql = KeGetCurrentIrql();
    KeLowerIrql(PASSIVE_LEVEL);
    status = _WaitForMultipleObjects(dwNumLock, HANDLES, WaitAll);
    if (!NT_SUCCESS(status)) {
        print("vWriteKLogLock ERROR: WaitForMultipleObjects fail code %ld\n", 
                                                                        status);
        _CASSERT(status == STATUS_SUCCESS);
    }

    KeRaiseIrql(irql, &irql);
    
    print("KLog: end vWriteKLogLock\n");
}

static VOID vWriteKLogUnLock(KLog *kl)
{
    KIRQL irql;
    _PASSERT(kl, "vWriteKLogUnLock ERROR: Invalid point to KLog.");

    print("KLog: start vWriteKLogUnLock\n");
    
    irql = KeGetCurrentIrql();
    KeLowerIrql(PASSIVE_LEVEL);
    print("KLog: PASSIVE_LEVEL vWriteKLogUnLock\n");
    
    /* Nobody starts.   */
    _SetEvent(&(kl->kReadLock));
    print("KLog: set ReadLock vWriteKLogUnLock\n");
    /* Could Start Evebody. */
    _SetEvent(&(kl->kWriteLock));
    print("KLog: set WriteLock vWriteKLogUnLock\n");
    
    KeRaiseIrql(irql, &irql);
    print("KLog: reLEVEL vWriteKLogUnLock\n");
    
    print("KLog: end vWriteKLogUnLock\n");
}

static size_t iTakeFromKLog(void *dest, KLog *kl, size_t num)
{
    KIRQL irql;
    size_t	count	= 0;
    size_t	write_p = 0;
    size_t	read_p	= 0;
    BOOLEAN	bufEmpty= FALSE;
    _PASSERT(dest, "iTakeFromKLog ERROR: Invalid point to destination.");
    _PASSERT(kl, "iTakeFromKLog ERROR: Invalid point to KLog.");

    print("KLog(Flush): start iTakeFromKLog\n");
    
    /* Get synch for atomic read buffer state.	*/
    vReadKLogLock(kl);
    bufEmpty= isEmptyKLog(kl);
    write_p = kl->write_p;
    read_p  = kl->read_p;
    count = sizeBusySpaceInKLog(kl);
    /* Free synch.	*/ 
    vReadKLogUnLock(kl);

    if (bufEmpty)
        return 0;

    if (num > count) {
        /* We can't to copy more than busy bytes.   */
        num = count;
    }

    KeLowerIrql(PASSIVE_LEVEL);
    _WaitForSingleObject(&(kl->kFlushLock));
    KeRaiseIrql(kl->irql, &irql);
    if (write_p > read_p) {
        /* write point >= read point    */
        count = write_p - read_p;
        if (count >= num) {
            /*	There are characters >= than num in the RingBuffer.
            So we can to copy num character from buffer.    */
            count = num;
        }

        _CopyMemory(dest, &kl->buf[read_p], count);
        read_p += count;
        read_p %= kl->size;
    } else {
        /* write point <= read point && buffer  isn't empty	*/
        count = kl->size - read_p;
        if (count >= num) {
            /*	There are characters >= than num between end of buffer and read
            pointer. So we can to copy num character from read pointer. */
            _CopyMemory(dest, &kl->buf[read_p], num);
            read_p += num;
            read_p %= kl->size;
        } else {
            _CopyMemory(dest, &kl->buf[read_p], count);
            dest = (void*)((size_t)dest + count);
            count = num - count;
            read_p = count;
            _CopyMemory(dest, &kl->buf[0], count);
		}
    }

    /* Get synch for atomic write new buffer state.	*/
    vWriteKLogLock(kl);
    kl->read_p = read_p;
    if (write_p == read_p)
        kl->state = EMPTY;
    else
        kl->state = PART_FILL;
    /* Free synch.	*/
    vWriteKLogUnLock(kl);

    print("KLog(Flush): end iTakeFromKLog\n");
    return num;
}

static VOID FlushThreadFunction(PVOID pParam)
{
    KIRQL irql;
    NTSTATUS status, wrStatus;
	size_t i = 0;
	KLog* kl = (KLog*) pParam;
	/* Create or Open Files Variables */
	CHAR buf[PAGESIZE] = {'\0'};
    BOOLEAN bErrorFlag = FALSE;
    IO_STATUS_BLOCK iostatus;
	ULONG ulBytesWritten = 0;
	ULONG ulBytesToWrite = PAGESIZE;
	_PASSERT(pParam, "FlushThreadFunction ERROR: Invalid point to paramets."); 
	
    print("KLog(Flush): start FlushThreadFunction\n");
    
	/* Create Auto Reset WaitableTimer. Every 1 seconds.	*/
    KeLowerIrql(PASSIVE_LEVEL);
    pkInitializeAndSetTimer(1, &(kl->kFlushTimer));
    KeRaiseIrql(kl->irql, &irql);
    
	for(;;) {
		ulBytesWritten = 0;

        KeLowerIrql(PASSIVE_LEVEL);
		status = _WaitForMultipleObjects(   NUM_FLUSH_EVENTS,
								            kl->pvFlushEvents,
								            WaitAny);
		if (!NT_SUCCESS(status)) {
			print("FlushThreadFunction ERROR: WaitForMultipleObjects code %ld\n", 
                                                                        status);
			_CASSERT(status == STATUS_SUCCESS);
		}
        KeRaiseIrql(kl->irql, &irql);

		ulBytesToWrite = iTakeFromKLog(buf, kl, PAGESIZE);

        KeLowerIrql(PASSIVE_LEVEL);
		while (ulBytesWritten != ulBytesToWrite) {
			ulBytesToWrite -= ulBytesWritten;
			wrStatus = _WriteFile(  kl->hLogFile,
                                    &iostatus,
									buf,			
									ulBytesToWrite  );          
			if (!NT_SUCCESS(wrStatus)) {
				print("FlushThreadFunction ERROR: WriteFile code %ld\n", wrStatus);
				_CASSERT(wrStatus == STATUS_SUCCESS);
			}

            ulBytesWritten = (ULONG)iostatus.Information;
		}
        KeRaiseIrql(kl->irql, &irql);

		if (status == STATUS_WAIT_0 + FLUSH_EXIT) {
			/* We need to free destruct thread */
            print("KLog(Flush): end FlushThreadFunction\n");
            irql = KeGetCurrentIrql();
            KeLowerIrql(PASSIVE_LEVEL);
            KeCancelTimer(&(kl->kFlushTimer));
			_SetEvent(&(kl->kExit));
			return;
		}
	}
}

static VOID createFlushThread(KLog *kl)
{
    NTSTATUS status;
    PVOID pThreadParam;

    _PASSERT(kl, "createFlushThread ERROR: Invalid point to KLog."); 
	
    print("KLog: start createFlushThread\n");
    
    pThreadParam = (PVOID) kl;
    status = _CreateThread( &(kl->hFlushThread),
                            FlushThreadFunction,
                            pThreadParam);

    if(!NT_SUCCESS(status)) {
        _FREE(kl->buf);
        _FREE(kl);
        print("createFlushThread ERROR: CreateThread fail code %ld\n", status);
        _CASSERT(status == STATUS_SUCCESS);
    }
    
    print("KLog: end createFlushThread\n");
}

/* Function must be called only on PASSIVE_LEVEL.   */
static VOID vOpenFile(KLog *kl, const PWSTR log)
{
    NTSTATUS status;
    UNICODE_STRING uStrFile;
    IO_STATUS_BLOCK ioStatus;
    OBJECT_ATTRIBUTES objAttr;
    _PASSERT(kl, "vOpenFile ERROR: Invalid point to KLog."); 
    _PASSERT(log, "vOpenFile ERROR: Invalid point to Log file name.");

    print("KLog: start vOpenFile\n");
    
    RtlInitUnicodeString(&uStrFile, L"\\??\\C:\\kLog.log");

    InitializeObjectAttributes( &objAttr, &uStrFile,
		                        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, 
                                NULL, NULL  );

    status = _CreateFile(   &(kl->hLogFile),
                            &objAttr,
                            &ioStatus   );

    if (!NT_SUCCESS(status)) {
        print("ERROR CreateFile code: %ld\n", status);
        _CASSERT(status == STATUS_SUCCESS);;
    }
    
    print("KLog: end vOpenFile\n");
}

KLog *constructKLog(size_t order, const PWSTR log)
{
    KLog        *kl;
    KIRQL       irql;
    NTSTATUS    status;
    _PASSERT(log, "constructKLog ERROR: Invalid point to logFile name.");

    print("KLog: start constructKLog\n");
    
    /* Save and change the IRQL. */
    irql = KeGetCurrentIrql();
    KeLowerIrql(PASSIVE_LEVEL);

    /* Allocate memory for buffer structer. */
    kl = (KLog*) _MALLOC(sizeof(KLog));
    _PASSERT(kl, "constructKLog ERROR: Failed to allocate memory.");

    /* Initialization buffer and state. */
    kl->irql = irql;
    kl->read_p = 0;
    kl->write_p = 0;
    kl->state = EMPTY;
    kl->size = PAGESIZE << order;
    kl->buf = (CHAR*) _MALLOC(kl->size);
    if (kl->buf == NULL) {
        _FREE(kl);
        _PASSERT(NULL, "constructKLog ERROR: Failed to allocate memory for buf.");
    }
	
    vOpenFile(kl, log);
    
    /* Initialization other fields */
    kl->lReaders = 0;
    kl->lWriters = 0;
    _InitializeEvent(&(kl->kExit), SynchronizationEvent, FALSE);
    kl->pvFlushEvents[FLUSH_FUNC] = &(kl->kFlushFunc);
    _InitializeEvent(&(kl->kFlushFunc), SynchronizationEvent, FALSE);
    kl->pvFlushEvents[FLUSH_EXIT] = &(kl->kFlushExit);
    _InitializeEvent(&(kl->kFlushExit), SynchronizationEvent, FALSE);
    kl->pvFlushEvents[FLUSH_TIMER] = &(kl->kFlushTimer);
    _InitializeEvent(&(kl->kReadLock), SynchronizationEvent, TRUE);
    _InitializeEvent(&(kl->kWriteLock), SynchronizationEvent, TRUE);
    _InitializeEvent(&(kl->kFlushLock), NotificationEvent, TRUE);

    /* Create flush thread. */
    createFlushThread(kl);

    /* Restore the IRQL. */
    KeRaiseIrql(kl->irql, &irql);
    
    print("KLog: complete constructKLog\n");
    return kl;
}

void destructKLog(KLog *kl)
{
    KIRQL irql;
	int i = 0;
	_PASSERT(kl, "ERROR: Invalid point to buffer want to free."); 

    print("KLog: start destructKLog\n");
    
    irql = KeGetCurrentIrql();
    KeLowerIrql(PASSIVE_LEVEL);

	_SetEvent(&(kl->kFlushExit));
	_WaitForSingleObject(&(kl->kExit));
    _FREE(kl->buf);
    kl->irql = PASSIVE_LEVEL;
    kl->size = 0;
    kl->read_p = 0;
    kl->write_p = 0;
    kl->state = EMPTY;

    kl->lWriters = 0;
	kl->lReaders = 0;

	_CloseHandle(kl->hLogFile);

	_FREE(kl);

    KeRaiseIrql(kl->irql, &irql);
    
    print("KLog: complete destructKLog\n");
}

static void vGetPlaceInKLog(KLog *kl,  size_t n)
{
	_PASSERT(kl, "vGetPlaceInRingBuf ERROR: Invalid point to KLog.");
	kl->write_p += n;
	kl->write_p %= kl->size;
}

size_t writeInKLog(KLog *kl, const void *data, size_t n)
{
    KIRQL   irql;
    size_t size_ew = 0;	/* rb->size - rb->write_p   */
    size_t read_p  = 0;
    size_t write_p = 0;

    print("KLog: start writeInKLog\n");
    
    irql = KeGetCurrentIrql();
    if (kl && data == NULL || n > kl->size) {
        _CASSERT("ERROR: Invalid argument in writeInBuf function\n" == 0);
    }

    if (kl->size < n)
        n = kl->size;
    
    /* We are in the loop while we have not resources   */
    while (TRUE) {
    /* Get synch for take and change a ringBuf state.   */
        vWriteKLogLock(kl);
        if (sizeFreeSpaceInKLog(kl) >= n) {
            read_p  = kl->read_p;
            write_p = kl->write_p;
            /*  Indicates that somebody is writing in buffer and we can't 
            flush.  */
            ++kl->lWriters;
            KeLowerIrql(PASSIVE_LEVEL);
            _ResetEvent(&(kl->kFlushLock));
            KeRaiseIrql(irql, &irql);
            
            vGetPlaceInKLog(kl, n);
            if (kl->write_p == kl->read_p)
                kl->state = FULL;
            else
                kl->state = PART_FILL;
            
            break;
        }
		
        flushKLog(kl);
        vWriteKLogUnLock(kl);
        /* Sleep for 25 msec.   */
        irql = KeGetCurrentIrql();
        KeLowerIrql(PASSIVE_LEVEL);
        KeStallExecutionProcessor(25);
        KeRaiseIrql(irql, &irql);
    }
    vWriteKLogUnLock(kl);

    /*  Now we have enough space. And we have consistent state. */
    if (write_p >= read_p) {
        /*  write point > read point    */
        size_ew = kl->size - write_p;
        if (size_ew >= n) {
            /*  There is enough space in buffer between write point and end of
            buffer. */
            _CopyMemory(&kl->buf[write_p], data, n);
        } else {
            _CopyMemory(&kl->buf[write_p], data, size_ew);
            n -= size_ew;
            data = (const void*)((size_t)data + size_ew);
            _CopyMemory(kl->buf, data, n);
        }
    } else {
        /*  There is enough space in buffer between read point and write
        point.  */
        _CopyMemory(&kl->buf[write_p], data, n);	
    }

    vWriteKLogLock(kl);
    --kl->lWriters;
    if (kl->lWriters == 0){
        irql = KeGetCurrentIrql();
        KeLowerIrql(PASSIVE_LEVEL);
        _SetEvent(&(kl->kFlushLock));
        KeRaiseIrql(irql, &irql);
    }
    // free synch
    vWriteKLogUnLock(kl);
    
    print("KLog: complete writeInKLog\n");
    return n;
}

VOID flushKLog(KLog *kl)
{
    KIRQL   irql;
    _PASSERT(kl, "flush ERROR: Invalid point to buffer.");

    print("KLog: start flushKLog\n");
    
    KeLowerIrql(PASSIVE_LEVEL);
    _SetEvent(&(kl->kFlushFunc));
    KeRaiseIrql(kl->irql, &irql);
    
    print("KLog: complete flushKLog\n");
}