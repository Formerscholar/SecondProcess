#include "kernel_object.h"

/*
	��ӡ�ں˽ṹ�����Ϣ

	��ӡ�ں˶�����Ҳ��������
	DbgPrint("DriverName=%wZ \n", pDriverObj->DriverName);
*/
VOID PrintKernelObject(IN PDRIVER_OBJECT pDriverObj)
{
	if (NULL == pDriverObj)
	{
		alert("IkPrintKernelObject: pDriverObj=NULL");

		return;
	}

	DbgPrint("ObjectAddr=%p \n", pDriverObj);
	DbgPrint("DriverStart=%p \n", pDriverObj->DriverStart);
	DbgPrint("DriverSize=%p \n", pDriverObj->DriverSize);
	DbgPrint("DriverName=%S \n", pDriverObj->DriverName.Buffer);
}

/*
	ͨ���ں˽ṹ���е�˫����������ں˽ṹ��
*/
VOID EnumKernelObject1()
{
	PLDR_ENTRY pHeader = NULL;
	PLDR_ENTRY pCurrent = NULL;

	pHeader = (PLDR_ENTRY)g_pDriverObj->DriverSection;
	pCurrent = (PLDR_ENTRY)pHeader->InLoadOrderLinks.Flink;

	while (pHeader != pCurrent) {
		if ((0 != pCurrent->DllBase) && (0 != pCurrent->EntryPoint) && (0 != pCurrent->SizeOfImage)) {
			if (0 == pCurrent->BaseDllName.Length)
			{
				DbgPrint("�ں˶������Ʊ�Ĩȥ \n");
			} 
			else
			{
				//DbgPrint("%wZ \n", pCurrent->BaseDllName) ����������У�ԭ��δ֪
				DbgPrint("%S \n", pCurrent->BaseDllName.Buffer);
			}
		}

		pCurrent = (PLDR_ENTRY)pCurrent->InLoadOrderLinks.Flink;
	}
}

/*
	��ȡ�ں˶���������Ϣ
*/
PLDR_ENTRY GetKernelObjectEntry(IN PWSTR kernelObjName)
{
	PLDR_ENTRY pHeader = NULL;
	PLDR_ENTRY pCurrent = NULL;

	UNICODE_STRING searchObjName = { 0 };
	RtlInitUnicodeString(&searchObjName, kernelObjName);

	pHeader = (PLDR_ENTRY)g_pDriverObj->DriverSection;
	pCurrent = (PLDR_ENTRY)pHeader->InLoadOrderLinks.Flink;

	while (pHeader != pCurrent) {
		if ((0 != pCurrent->DllBase) && (0 != pCurrent->EntryPoint) && (0 != pCurrent->SizeOfImage)) {
			if (0 == pCurrent->BaseDllName.Length)
			{
				alert("�ں˶������Ʊ�Ĩȥ");

				return NULL;
			}
			else
			{
				if (0 == RtlCompareUnicodeString(&pCurrent->BaseDllName, &searchObjName, FALSE)) {
					return pCurrent;
				}
			}
		}

		pCurrent = (PLDR_ENTRY)pCurrent->InLoadOrderLinks.Flink;
	}

	return NULL;
}