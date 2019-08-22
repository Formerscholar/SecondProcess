#ifndef __KERNEL_OBJECT_H_
#define __KERNEL_OBJECT_H_

#include <ntddk.h>

#include "common.h"

typedef struct _LDR_DATA_TABLE_ENTRY
{
	LIST_ENTRY InLoadOrderLinks;
	LIST_ENTRY InMemoryOrderLinks;
	LIST_ENTRY InInitializationOrderLinks;
	ULONG DllBase;
	ULONG EntryPoint;
	ULONG SizeOfImage;
	UNICODE_STRING FullDllName;
	UNICODE_STRING BaseDllName;
}LDR_ENTRY, *PLDR_ENTRY;

#ifdef __cplusplus
extern "C" {
#endif

	//���ܷ���extern "C" {}��
	extern PDRIVER_OBJECT g_pDriverObj;

	/*
		ͨ���ں˽ṹ���е�˫����������ں˽ṹ��
	*/
	VOID EnumKernelObject1();

	/*
		ͨ��ZwQuerySystemInformation�ĵ�11�Ź���
	*/
	VOID EnumKernelObject2();

	/*
		��ӡ�ں˽ṹ�����Ϣ
	*/
	VOID PrintKernelObject(IN PDRIVER_OBJECT pDriverObj);

	/*
		��ȡ�ں˶���������Ϣ
	*/
	PLDR_ENTRY GetKernelObjectEntry(IN PWSTR kernelObjName);


#ifdef __cplusplus
}
#endif

#endif
