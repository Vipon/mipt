#include "viponMouse.h"

VOID DriverUnload(PDRIVER_OBJECT pFiDO)
{
	DbgPrint("Mouse Filter: Entry in DriverUnload\n");
	UNREFERENCED_PARAMETER(pFiDO);
}

NTSTATUS DispatchThru(PDEVICE_OBJECT pFiDO, PIRP pIrp)
{
	NTSTATUS ntStatus;
	PDEVICE_EXTENSION pDeviceExtension = (PDEVICE_EXTENSION)pFiDO->DeviceExtension;

	DbgPrint("Mouse Filter: Entry in DispatchThru\n");

	IoSkipCurrentIrpStackLocation(pIrp);
	return IoCallDriver(pDeviceExtension->pLowerDO, pIrp);
}

NTSTATUS DispatchPower(PDEVICE_OBJECT pFiDO, PIRP pIrp)
{
	PDEVICE_EXTENSION pDeviceExtension = (PDEVICE_EXTENSION)pFiDO->DeviceExtension;

	DbgPrint("Mouse Filter: Entry in DispatchPower\n");

	PoStartNextPowerIrp(pIrp);
    /*  Delay IRP stack pointer for the next driver in the stack.   */
	IoSkipCurrentIrpStackLocation(pIrp);
	return PoCallDriver(pDeviceExtension->pLowerDO, pIrp);
}

NTSTATUS DispatchPnp(PDEVICE_OBJECT pFiDO, PIRP pIrp)
{
	ULONG ulMinorFunction = 0;
    NTSTATUS ntStatus = 0;
	PDEVICE_EXTENSION pDeviceExtension = (PDEVICE_EXTENSION)pFiDO->DeviceExtension;

	DbgPrint("Mouse Filter: Entry in DispatchPnp\n");

    ulMinorFunction = IoGetCurrentIrpStackLocation(pIrp)->MinorFunction;
    
    IoSkipCurrentIrpStackLocation(pIrp);
    ntStatus = IoCallDriver(pDeviceExtension->pLowerDO, pIrp);

	if (ulMinorFunction == IRP_MN_REMOVE_DEVICE)
	{
		IoDetachDevice(pDeviceExtension->pLowerDO);
		IoDeleteDevice(pFiDO);
	}
	
    return ntStatus;
}

NTSTATUS MouseFilterInvertorYCompletion(PDEVICE_OBJECT pFiDO, PIRP pIrp, PVOID pContext)
{
    ULONG_PTR i = 0;
	PMOUSE_INPUT_DATA MouseData = NULL;
	ULONG_PTR EventCount = pIrp->IoStatus.Information / sizeof(MOUSE_INPUT_DATA);

	DbgPrint("Mouse Filter: Entry in MouseFilterInvertorYCompletion\n");

	if (NT_SUCCESS(pIrp->IoStatus.Status)) {
		MouseData = (PMOUSE_INPUT_DATA)pIrp->AssociatedIrp.SystemBuffer;
        for (i = 0; i < EventCount; ++i) {
			if (MouseData[i].Flags & MOUSE_MOVE_ABSOLUTE) {
				MouseData[i].LastY = 0xFFFF - MouseData[i].LastY;
			} else {
				MouseData[i].LastY *= -1;
			}
		}
	}

	if (pIrp->PendingReturned) {
		IoMarkIrpPending(pIrp);
	}

	return pIrp->IoStatus.Status;
}

NTSTATUS DispatchRead(PDEVICE_OBJECT pFiDO, PIRP pIrp)
{
    /* We will be here, if we attache to \Device\PointerClass0. */
	PDEVICE_EXTENSION pDeviceExtension = (PDEVICE_EXTENSION)pFiDO->DeviceExtension;

	DbgPrint("Mouse Filter: Entry in DispatchRead\n");
	
	/* Copy our stack to the next driver. */
	IoCopyCurrentIrpStackLocationToNext(pIrp);
	IoSetCompletionRoutine(	pIrp, 
							(PIO_COMPLETION_ROUTINE) MouseFilterInvertorYCompletion, 
							pFiDO, 
							TRUE,	/*InvokeOnSuccess*/
							FALSE,	/*InvokeOnError*/
							FALSE	/*InvokeOnCancel*/);

	return  IoCallDriver(pDeviceExtension->pLowerDO, pIrp);
}

VOID FilterServiceCallback( PDEVICE_OBJECT pFiDO,
                            PMOUSE_INPUT_DATA pInputDataStart,
							PMOUSE_INPUT_DATA pInputDataEnd,
							PULONG pULInputDataConsumed )
{
    ULONG i = 0;
    PDEVICE_EXTENSION pDeviceExtension = (PDEVICE_EXTENSION)pFiDO->DeviceExtension;
    PSERVICE_CALLBACK_ROUTINE pCallBackRoutine;
	DbgPrint("Mouse Filter: Entry in FilterServiceCallback\n");

	for (;&(pInputDataStart[i]) != pInputDataEnd; ++i) {
        if (pInputDataStart[i].Flags & MOUSE_MOVE_ABSOLUTE) {
			pInputDataStart[i].LastY = 0xFFFF - pInputDataStart[i].LastY;
        } else {
			pInputDataStart[i].LastY *= -1;
        }
    }

    pCallBackRoutine = (PSERVICE_CALLBACK_ROUTINE)pDeviceExtension->upperConnectData.ClassService;
	(*pCallBackRoutine)(
		pDeviceExtension->upperConnectData.ClassDeviceObject,
		pInputDataStart,
		pInputDataEnd,
		pULInputDataConsumed    );
}

NTSTATUS DispatchInternalDeviceControl(PDEVICE_OBJECT pFiDO, PIRP pIrp)
{
    ULONG ulIoControlCode;
    PCONNECT_DATA pConnectData;
    PIO_STACK_LOCATION  pCurrentSL;
    PDEVICE_EXTENSION pDeviceExtension = (PDEVICE_EXTENSION)pFiDO->DeviceExtension;
	DbgPrint("Mouse Filter: Entry in DispatchInternalDeviceControl\n");

    pCurrentSL = IoGetCurrentIrpStackLocation(pIrp);
    ulIoControlCode = pCurrentSL->Parameters.DeviceIoControl.IoControlCode;
	if (ulIoControlCode == IOCTL_INTERNAL_MOUSE_CONNECT) {
		DbgPrint("USB MOUSE: IOCTL_INTERNAL_MOUSE_CONNECT\n");
		
		pConnectData = (PCONNECT_DATA)pCurrentSL->Parameters.DeviceIoControl.Type3InputBuffer;
        pDeviceExtension->upperConnectData = *pConnectData;
        
		pConnectData->ClassDeviceObject = pFiDO;
		pConnectData->ClassService = (PVOID)FilterServiceCallback;
	}

	IoSkipCurrentIrpStackLocation(pIrp);
	return IoCallDriver(pDeviceExtension->pLowerDO, pIrp);
}

NTSTATUS AddDevice(PDRIVER_OBJECT pFiDO, PDEVICE_OBJECT pPDO)
{
	NTSTATUS ntStatus;
	PDEVICE_OBJECT pMause, pLowerDO;
	PDEVICE_EXTENSION pDeviceExtension;

	DbgPrint("Mouse Filter: Entry in AddDevice\n");

	ntStatus = IoCreateDevice(	pFiDO, 
								sizeof(DEVICE_EXTENSION),
								NULL,				/* Device Name like object name. */
								FILE_DEVICE_MOUSE,	/* Device type. */
								0,
								FALSE,				/* Exclusive.	*/
								&pMause	);

	if (NT_SUCCESS(ntStatus)) {
		pLowerDO = IoAttachDeviceToDeviceStack(pMause, pPDO);
		if (pLowerDO) {
			DbgPrint("Mouse Filter: AddDevice: Success IoAttachDeviceToDeviceStac\n");

            pDeviceExtension = (PDEVICE_EXTENSION)pMause->DeviceExtension;
			pDeviceExtension->pLowerDO = pLowerDO;
			pDeviceExtension->inverse = FALSE;

            pMause->Flags |=  pLowerDO->Flags & DO_POWER_PAGABLE;;
			pMause->Flags &= ~DO_DEVICE_INITIALIZING;

			ntStatus = STATUS_SUCCESS;
		} else {
			DbgPrint("Mouse Filter: AddDevice: Fail IoAttachDeviceToDeviceStack\n");

			ntStatus = STATUS_DEVICE_REMOVED;
			IoDeleteDevice(pMause);
		}	
	} else {
		DbgPrint("Mouse Filter: AddDevice - IoCreateDevice failed - %X\n", ntStatus);
	}

	return ntStatus;
}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, 
					 PUNICODE_STRING ustrRegistryPath)
{
	INT i = 0;

	DbgPrint("Mouse Filter: Entry in DriverEntry\n");

	for (i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; ++i)
		pDriverObject->MajorFunction[i] = DispatchThru;

	pDriverObject->MajorFunction[IRP_MJ_PNP] = DispatchPnp;
	pDriverObject->MajorFunction[IRP_MJ_READ] = DispatchRead;
	pDriverObject->MajorFunction[IRP_MJ_POWER] = DispatchPower;
    pDriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL] = DispatchInternalDeviceControl;

	pDriverObject->DriverUnload = DriverUnload;
	pDriverObject->DriverExtension->AddDevice = AddDevice;

	return STATUS_SUCCESS;
}