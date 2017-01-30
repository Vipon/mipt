#include "kLogger.h"

#define BUF_ORDER 1
#define THREAD_NUM 3
#define MSG_NUM 200

KLog *klog;

VOID KLogUnload(PDRIVER_OBJECT DriverObject)
{
    DbgPrint("KLog: start DriverUnload\n");

    /*Do nothing */

	DbgPrint("KLogger: DriverUnload completed\n");
}

VOID highThreadFunction(VOID *param)
{
    KLog *klog = (KLog*)param;
    KIRQL irql;
    int i;
    const CHAR high[] = "Hi from high\n";
    
    DbgPrint("KLogger: highThreadFunction start\n");
    
    KeRaiseIrql(HIGH_LEVEL, &irql);
    for (i = 0; i < MSG_NUM; ++i) 
        writeInKLog(klog, high, sizeof(high));
    KeLowerIrql(irql);
    
    DbgPrint("KLogger: highThreadFunction end\n");
}

VOID lowThreadFunction(VOID *param)
{
    KLog *klog = (KLog*)param;
    KIRQL irql;
    int i;
    const CHAR low[] = "Hi from low\n";
    
    DbgPrint("KLogger: lowThreadFunction start\n");
    KeLowerIrql(PASSIVE_LEVEL);
    for (i = 0; i < MSG_NUM; ++i) 
        writeInKLog(klog, low, sizeof(low));
    
    DbgPrint("KLogger: lowThreadFunction end\n");
}

NTSTATUS DriverEntry(
	IN PDRIVER_OBJECT   DriverObject,
	IN PUNICODE_STRING  RegistryPath)
{
    int i = 0;
    HANDLE hThread;
    PKTHREAD kThread[THREAD_NUM]; 
    PWSTR fileName = L"\\??\\C:\\kLog.log";
    print("KLog: start DriverEntry\n");
	
    DriverObject->DriverUnload = KLogUnload;

    klog = constructKLog(BUF_ORDER, fileName);
    if (klog == NULL) {
        print("DriverEntry ERROR: constructKLog\n");
        return STATUS_FAILED_DRIVER_ENTRY;
    }

    KeLowerIrql(PASSIVE_LEVEL);
    for (i = 0; i < THREAD_NUM; ++i) {
        DbgPrint("KLogger: create thread %d\n", i);
        if (i % 2) {
            _CreateThread( &(hThread),
                                    highThreadFunction,
                                    klog);
        }
        else {
            _CreateThread( &(hThread),
                                    lowThreadFunction,
                                    klog);
        }
        
        ObReferenceObjectByHandle(hThread, THREAD_ALL_ACCESS, 
                                    NULL, KernelMode, 
                                    &(kThread[i]), NULL );
    }
    
    _WaitForMultipleObjects(    THREAD_NUM,
                                kThread,
                                WaitAll); 
    destructKLog(klog);
    
    DbgPrint("KLog: DriverEntry completed\n");
    return STATUS_SUCCESS;
}