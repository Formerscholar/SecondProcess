/************************************************************************/
/*
	win7 x32 �Խ�������ϵ

	0��3�����ÿ��ʹ��IDT��������35
*/
/************************************************************************/

#include <ntddk.h>

#include "kernel.h"
#include "IDT.h"
#include "SSDT.h"
#include "kernel_function.h"
#include "hook_dbgfunction.h"
#include "debugport.h"
#include "PAEAddr.h"
#include "hook_kfunction.h"

PKSYSTEM_SERVICE_TABLE pSystemServiceTable;
PDRIVER_OBJECT g_pDriverObj;
PMyDebugPortDLinkList g_pDebugPortDLinkList;

#define _DEBUG_HOOK
//#define _RECOVER_TP_HOOK

//����0��3�����ÿ��
#define _OPEN_API_FRAME

#define _CREATE_DEVICE

#ifdef _CREATE_DEVICE
#define DEVICE_NAME L"\\Device\\SecondProcess"
#define SYMBOLICLINK_NAME L"\\??\\SecondProcess"

// 0-2047�Ǳ�����  2048��4095
#define OPER1  CTL_CODE(FILE_DEVICE_UNKNOWN,0x800,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define OPER2  CTL_CODE(FILE_DEVICE_UNKNOWN,0x900,METHOD_BUFFERED,FILE_ANY_ACCESS)
#endif

VOID demo(IN ULONG a, IN ULONG b)
{
	DbgPrint("%d \n", a + b);

	return a + b;
}

VOID DriverUnload(PDRIVER_OBJECT pDriverObj)
{	
#ifdef _CREATE_DEVICE
	UNICODE_STRING SymbolicLinkName = { 0 };
	RtlInitUnicodeString(&SymbolicLinkName, SYMBOLICLINK_NAME);

	IoDeleteSymbolicLink(&SymbolicLinkName);
	IoDeleteDevice(pDriverObj->DeviceObject);
#endif

#ifdef _DEBUG_HOOK
	//�ҹ�������DebugPort��صĺ����������Ǳ����Խ��̣�
	UnHook_DbgkpQueueMessage();
	UnHook_KiDispatchException();
	UnHook_DbgkForwardException();
	UnHook_PspExitThread();
	UnHook_DbgkExitThread();
	UnHook_DbgkCreateThread();
	UnHook_DbgkExitProcess();
	UnHook_DbgkMapViewOfSection();
	UnHook_DbgkUnMapViewOfSection();
	UnHook_DbgkpMarkProcessPeb();

	//�ҹ�������DebugPort��صĺ����������ǵ�������
	//������Ϊһ��ʹ��
//	UnHook_NtDebugActiveProcess();
//	UnHook_DbgkpSetProcessDebugObject();

	UnHook_NtDebugActiveProcess_rewrite();
	UnHook_NtCreateDebugObject();

	UnHook_NtWaitForDebugEvent();
	UnHook_NtDebugContinue();

	UnHook_NtQueryInformationProcess();
	UnHook_NtQueryObject();
	UnHook_NtQueryInformationThread();

	MyDebugPortDLinkListFree(g_pDebugPortDLinkList);
#endif

#ifdef _OPEN_API_FRAME
	FreeSystemServiceTable(pSystemServiceTable);
#endif

	DbgPrint("������ж�� \n");
}

NTSTATUS IrpCreateProc(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	pIrp->IoStatus.Information = 0;

	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

NTSTATUS IrpCloseProc(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	pIrp->IoStatus.Information = 0;

	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

NTSTATUS IrpDeviceContrlProc(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	NTSTATUS status = STATUS_INVALID_DEVICE_REQUEST;
	PIO_STACK_LOCATION pIrpStack;
	ULONG uIoControlCode;
	PVOID pIoBuffer;
	ULONG uInLength;
	ULONG uOutLength;
	ULONG uRead;
	ULONG uWrite;

	//������ʱ������ֵ
	uRead = 0;
	uWrite = 0x12345678;

	//��ȡIRP����
	pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
	//��ȡ������
	uIoControlCode = pIrpStack->Parameters.DeviceIoControl.IoControlCode;
	//��ȡ��������ַ(���������Ļ���������һ����
	pIoBuffer = pIrp->AssociatedIrp.SystemBuffer;
	//Ring3 �������ݵĳ���
	uInLength = pIrpStack->Parameters.DeviceIoControl.InputBufferLength;
	//Ring0 �������ݵĳ���
	uOutLength = pIrpStack->Parameters.DeviceIoControl.OutputBufferLength;
	
	switch (uIoControlCode)
	{
	case OPER1:
		DbgPrint("IrpDeviceContrlProc -> OPER1 ...\n");

		pIrp->IoStatus.Information = 0;
		status = STATUS_SUCCESS;
		break;
	case OPER2:
		DbgPrint("IrpDeviceContrlProc -> OPER2 �����ֽ�����%d  \n", uInLength);
		DbgPrint("IrpDeviceContrlProc -> OPER2 ����ֽ�����%d  \n", uOutLength);
		//Read From Buffer
		memcpy(&uRead, pIoBuffer, 4);
		DbgPrint("IrpDeviceContrlProc -> OPER2 ...%x \n", uRead);
		//Write To Buffer
		memcpy(pIoBuffer, &uWrite, 4);
		//Set Status

		pIrp->IoStatus.Information = 2;
		status = STATUS_SUCCESS;
		break;
	}

	pIrp->IoStatus.Status = status;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return status;
}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObj, PUNICODE_STRING pRegistryString)
{
	NTSTATUS status = STATUS_SUCCESS;

	g_pDriverObj = pDriverObj;

#ifdef _CREATE_DEVICE
	PDEVICE_OBJECT pDeviceObj = NULL;
	UNICODE_STRING Devicename;
	UNICODE_STRING SymbolicLinkName;

	RtlInitUnicodeString(&Devicename, DEVICE_NAME);

	status = IoCreateDevice(pDriverObj, 0, &Devicename, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &pDeviceObj);
	if (status != STATUS_SUCCESS)
	{
		DbgPrint("�����豸ʧ��!  \r\n");
		return status;
	}

	pDeviceObj->Flags |= DO_BUFFERED_IO;

	RtlInitUnicodeString(&SymbolicLinkName, SYMBOLICLINK_NAME);

	status = IoCreateSymbolicLink(&SymbolicLinkName, &Devicename);
	if (status != STATUS_SUCCESS)
	{
		DbgPrint("������������ʧ��!  \r\n");
		IoDeleteDevice(pDeviceObj);
		return status;
	}
#endif

#ifdef _OPEN_API_FRAME
	status = CreateSystemServiceTable(&pSystemServiceTable);
	if (!NT_SUCCESS(status)) {
		DbgPrint("%s(%d): CreateSystemServiceTable failed.", __FILE__, __LINE__);

		return status;
	}
	else
	{
		DbgPrint("SystemServiceTable��ַ��%X \n", pSystemServiceTable);
	}

	SetIDTEntry(35, (ULONG)YKiSystemService);
#endif

#ifdef _OPEN_API_FRAME
	AddFun2SystemServiceTable(pSystemServiceTable, (ULONG)demo, 2);								//0
	AddFun2SystemServiceTable(pSystemServiceTable, (ULONG)NtReadVirtualMemory, 5);				//1
	AddFun2SystemServiceTable(pSystemServiceTable, (ULONG)NtReadVirtualMemory1, 5);				//2
	AddFun2SystemServiceTable(pSystemServiceTable, (ULONG)Hook_NtQueryObject, 0);				//3
	AddFun2SystemServiceTable(pSystemServiceTable, (ULONG)UnHook_NtQueryObject, 0);				//4
	AddFun2SystemServiceTable(pSystemServiceTable, (ULONG)Hook_NtQueryInformationProcess, 0);				//5
	AddFun2SystemServiceTable(pSystemServiceTable, (ULONG)UnHook_NtQueryInformationProcess, 0);				//6

	AddFun2SystemServiceTable(pSystemServiceTable, (ULONG)GetPDE, 1);				//7
	AddFun2SystemServiceTable(pSystemServiceTable, (ULONG)ChangePDEWrite, 1);		//8
	AddFun2SystemServiceTable(pSystemServiceTable, (ULONG)GetPTE, 1);				//9
	AddFun2SystemServiceTable(pSystemServiceTable, (ULONG)ChangePTEWrite, 1);		//10
	AddFun2SystemServiceTable(pSystemServiceTable, (ULONG)IsAddrValid, 1);			//11
	AddFun2SystemServiceTable(pSystemServiceTable, (ULONG)IsAddrWrite, 1);			//12
	AddFun2SystemServiceTable(pSystemServiceTable, (ULONG)SetPDE, 2);				//13
	AddFun2SystemServiceTable(pSystemServiceTable, (ULONG)SetPTE, 2);				//14
	AddFun2SystemServiceTable(pSystemServiceTable, (ULONG)IsPDEValid, 1);			//15
	AddFun2SystemServiceTable(pSystemServiceTable, (ULONG)IsPDEWritable, 1);		//16
	AddFun2SystemServiceTable(pSystemServiceTable, (ULONG)IsPTEValid, 1);			//17
	AddFun2SystemServiceTable(pSystemServiceTable, (ULONG)IsPTEWritable, 1);		//18
	AddFun2SystemServiceTable(pSystemServiceTable, (ULONG)ChangePDENoWrite, 1);		//19
	AddFun2SystemServiceTable(pSystemServiceTable, (ULONG)ChangePTENoWrite, 1);		//20

	AddFun2SystemServiceTable(pSystemServiceTable, (ULONG)MyNtProtectVirtualMemory, 5);		//21
#endif	
	
#ifdef _DEBUG_HOOK
	//�������DebugPort��˫������
	g_pDebugPortDLinkList = MyDebugPortDLinkListCreate();
	if (NULL == g_pDebugPortDLinkList)
	{
		return STATUS_UNSUCCESSFUL;
	}

	//�ҹ�������DebugPort��صĺ����������Ǳ����Խ��̣�
	//��������Ϊһ��ʹ��
//	Hook_NtDebugActiveProcess();
//	Hook_DbgkpSetProcessDebugObject();

	Hook_NtDebugActiveProcess_rewrite();	//1�����ݴ���˫������ 2��DebugPort��0
	Hook_NtCreateDebugObject();				//���Զ���ĵ�ַ����˫��������

	Hook_DbgkpQueueMessage();
	Hook_KiDispatchException();
	Hook_DbgkForwardException();
	Hook_PspExitThread();
	Hook_DbgkExitThread();
	Hook_DbgkCreateThread();
	Hook_DbgkExitProcess();
	Hook_DbgkMapViewOfSection();
	Hook_DbgkUnMapViewOfSection();
	Hook_DbgkpMarkProcessPeb();

	//�ҹ�������DebugPort��صĺ����������ǵ�������
	Hook_NtWaitForDebugEvent();
	Hook_NtDebugContinue();

	Hook_NtQueryInformationProcess();
	Hook_NtQueryObject();
	Hook_NtQueryInformationThread();
#endif

#ifdef _RECOVER_TP_HOOK
//	Recover_NtAllocateVirtualMemory();

	Recover_PsResumeProcess();
	Recover_ZwFsControlFile();
	Recover_KiSwapThread();
	Recover_KiAttachProcess();
	Recover_KeResumeThread();

	//�������ͨ������ָ���������ͨ��PChunter�ָ��򲻻ᣬԭ����
//	Recover_SeDefaultObjectMethod();
#endif
	
	

//	DbgPrint("%X \n", KF_GetMiProtectVirtualMemoryAddr());

	pDriverObj->MajorFunction[IRP_MJ_CREATE] = IrpCreateProc;
	pDriverObj->MajorFunction[IRP_MJ_CLOSE] = IrpCloseProc;
	pDriverObj->MajorFunction[IRP_MJ_DEVICE_CONTROL] = IrpDeviceContrlProc;
	pDriverObj->DriverUnload = DriverUnload;
	return status;
}