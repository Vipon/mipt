#ifndef _MOUSEDRIVER_H
#define  _MOUSEDRIVER_H

#include <ntddk.h>
#include <ntddmou.h>
#include <Kbdmou.h>

typedef struct {
	PDEVICE_OBJECT  pLowerDO;	/* Object that is bellow us in the driver stack. */
	CONNECT_DATA    upperConnectData;
    BOOLEAN         inverse;
} DEVICE_EXTENSION;

typedef DEVICE_EXTENSION* PDEVICE_EXTENSION;

/*	
 *	Entry point.
 *	pDriverObject	- contain points to all functions.
 *	ustrRegistryPath- 
 */
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING ustrRegistryPath);

/*
 * pFiDO	- point to Filter Device Object.
 * pIRP		- point to Physical Device Object.
 */
NTSTATUS DispatchPnp(PDEVICE_OBJECT pFiDO, PIRP pIrp);
/* Call when mause is clicked. */
NTSTATUS DispatchInternalDeviceControl(PDEVICE_OBJECT pFiDO, PIRP pIrp);
NTSTATUS DispatchRead(PDEVICE_OBJECT pFiDO, PIRP pIrp);
/* Do nothing. Only transmit packet to the next. */
NTSTATUS DispatchThru(PDEVICE_OBJECT pFiDO, PIRP pIrp);
NTSTATUS DispatchPower(PDEVICE_OBJECT pFiDO, PIRP pIrp);
/*
 * pFiDO	- point to Filter Device Object.
 * pPDO		- point to Physical Device Object.
 */
NTSTATUS AddDevice(PDRIVER_OBJECT pFiDO, PDEVICE_OBJECT pPDO);
NTSTATUS MouseFilterInvertorYCompletion(PDEVICE_OBJECT pFiDO, PIRP pIrp, PVOID pContext);

VOID FilterServiceCallback( PDEVICE_OBJECT pFiDO,
                            PMOUSE_INPUT_DATA pInputDataStart,
							PMOUSE_INPUT_DATA pInputDataEnd,
							PULONG pULInputDataConsumed );
VOID DriverUnload(PDRIVER_OBJECT DriverObject);

#endif /* _MOUSEDRIVER_H */