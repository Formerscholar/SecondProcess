#ifndef __SSDT_H_
#define __SSDT_H_

#include <ntddk.h>

typedef struct _KSYSTEM_SERVICE_TABLE
{
	PULONG  ServiceTableBase;          // ������ַ�����ַ
	PULONG  ServiceCounterTableBase;   // SSDT���������õĴ���
	ULONG   NumberOfService;           // �����ĸ���
	PUSHORT  ParamTableBase;           // �������������ַ
} KSYSTEM_SERVICE_TABLE, *PKSYSTEM_SERVICE_TABLE;

typedef struct _KSERVICE_TABLE_DESCRIPTOR
{
	KSYSTEM_SERVICE_TABLE   ntoskrnl;  // ntoskrnl.exe �ĺ���
	KSYSTEM_SERVICE_TABLE   win32k;    // win32k.sys �ĺ���
	KSYSTEM_SERVICE_TABLE   notUsed1;
	KSYSTEM_SERVICE_TABLE   notUsed2;
} KSERVICE_TABLE_DESCRIPTOR, *PKSERVICE_TABLE_DESCRIPTOR;

#ifdef __cplusplus
extern "C" {
#endif

	/*
		����SSDT��
	*/
	NTSTATUS CreateSystemServiceTable(PKSYSTEM_SERVICE_TABLE* returnVal);

	/*
		�ͷ�SSDT��
	*/
	VOID FreeSystemServiceTable(PKSYSTEM_SERVICE_TABLE pSystemServiceTable);

	/*
		��SSDT���м��뺯��
	*/
	NTSTATUS AddFun2SystemServiceTable(PKSYSTEM_SERVICE_TABLE pSystemServiceTable, ULONG funAddr, UCHAR paramNumber);

#ifdef __cplusplus
}
#endif

#endif
