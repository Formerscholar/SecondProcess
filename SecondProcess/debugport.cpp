#include "debugport.h"

PMyDebugPort createMyDebugPort(IN ULONG process, IN ULONG debugObject, IN ULONG debuggerProcess)
{
	PMyDebugPort pDebugPort = (PMyDebugPort)ExAllocatePool(NonPagedPool, sizeof(MyDebugPort));
	if (NULL == pDebugPort)
	{
		alert("ExAllocatePool failed");

		return NULL;
	}

	pDebugPort->process = process;
	pDebugPort->debuggerProcess = debuggerProcess;
	pDebugPort->debugObject = debugObject;

	return pDebugPort;
}

PMyDebugPortDLinkList MyDebugPortDLinkListCreate()
{
	PMyDebugPortDLinkList pDLinkList = (PMyDebugPortDLinkList)ExAllocatePool(NonPagedPool, sizeof(MyDebugPortDLinkList));
	if (NULL == pDLinkList)
	{
		alert("ExAllocatePool failed");

		return NULL;
	}

	pDLinkList->top = createMyDebugPort(0, 0, 0);
	pDLinkList->top->prev = pDLinkList->top->next = NULL;

	pDLinkList->bottom = createMyDebugPort(0, 0, 0);
	pDLinkList->bottom->prev = pDLinkList->bottom->next = NULL;

	pDLinkList->top->next = pDLinkList->bottom;
	pDLinkList->bottom->prev = pDLinkList->top;

	pDLinkList->eleNum = 0;

	return pDLinkList;
}

BOOLEAN MyDebugPortDLinkListIsEmpty(IN PMyDebugPortDLinkList pDLinkList)
{
	if (NULL == pDLinkList)
	{
		alert("ʵ�����п�ָ��");

		return FALSE;
	}

	return (pDLinkList->top->next == pDLinkList->bottom) ? TRUE : FALSE;
}

VOID MyDebugPortDLinkListFree(IN PMyDebugPortDLinkList pDLinkList)
{
	if (NULL == pDLinkList)
	{
		alert("ʵ�����п�ָ��");

		return;
	}

	operate_prompt("�ͷŴ洢DebugPort��˫������");

	if (MyDebugPortDLinkListIsEmpty(pDLinkList))
	{
		ExFreePool(pDLinkList->top);
		ExFreePool(pDLinkList->bottom);
		ExFreePool(pDLinkList);

		return;
	}

	PMyDebugPort pCurrent = pDLinkList->top->next;
	while (pCurrent != pDLinkList->bottom)
	{
		ExFreePool(pCurrent);

		pCurrent = pCurrent->next;
	}

	ExFreePool(pDLinkList->top);
	ExFreePool(pDLinkList->bottom);
	ExFreePool(pDLinkList);

	return;
}

VOID MyDebugPortDLinkListTraverseTop(IN PMyDebugPortDLinkList pDlinkList)
{
	if (NULL == pDlinkList)
	{
		alert("ʵ�����п�ָ��");

		return;
	}

	if (MyDebugPortDLinkListIsEmpty(pDlinkList))
	{
		alert("������");

		return;
	}

	PMyDebugPort pCurrent = pDlinkList->top->next;
	while (pCurrent != pDlinkList->bottom)
	{
		DbgPrint("process=%08X, debugObject=%08X \n", pCurrent->process, pCurrent->debugObject);

		pCurrent = pCurrent->next;
	}
}

ULONG MyDebugPortDLinkListFind(IN PMyDebugPortDLinkList pDLinkList, IN ULONG process)
{
	if (NULL == pDLinkList)
	{
		alert("ʵ�����п�ָ��");

		return 0;
	}

	if (NULL == process)
	{
		alert("eprocess�ĵ�ַ����Ϊ��");

		return 0;
	}

	if (MyDebugPortDLinkListIsEmpty(pDLinkList))
	{
		return 0;
	}

	PMyDebugPort pCurrent = pDLinkList->top->next;
	while (pCurrent != pDLinkList->bottom)
	{
		if (process == pCurrent->process)
		{
			return (ULONG)pCurrent->debugObject;
		}

		pCurrent = pCurrent->next;
	}

	return 0;
}

ULONG MyDebugPortDLinkListFindByDebuggerProcess(IN PMyDebugPortDLinkList pDLinkList, IN ULONG debuggerProcess)
{
	if (NULL == pDLinkList)
	{
		alert("ʵ�����п�ָ��");

		return 0;
	}

	if (NULL == debuggerProcess)
	{
		alert("debuggerProcess�ĵ�ַ����Ϊ��");

		return 0;
	}

	if (MyDebugPortDLinkListIsEmpty(pDLinkList))
	{
		alert("������");

		return 0;
	}

	PMyDebugPort pCurrent = pDLinkList->top->next;
	while (pCurrent != pDLinkList->bottom)
	{
		if (debuggerProcess == pCurrent->debuggerProcess)
		{
			return (ULONG)pCurrent->debugObject;
		}

		pCurrent = pCurrent->next;
	}

	return 0;
}

BOOLEAN MyDebugPortDLinkListDelete(IN PMyDebugPortDLinkList pDLinkList, IN ULONG process)
{
	if (NULL == pDLinkList)
	{
		alert("MyDebugPortDLinkListDelete: pDLinkList=NULL");

		return FALSE;
	}

	if (NULL == process)
	{
		alert("MyDebugPortDLinkListDelete: eprocess=NULL");

		return FALSE;
	}

	if (MyDebugPortDLinkListIsEmpty(pDLinkList))
	{
		alert("MyDebugPortDLinkListDelete: ������");

		return FALSE;
	}

	if (0 == MyDebugPortDLinkListFind(pDLinkList, process))
	{
		alert("MyDebugPortDLinkListDelete: eprocess no exists");

		return FALSE;
	}

	PMyDebugPort pCurrent = pDLinkList->top->next;
	while (pCurrent != pDLinkList->bottom)
	{
		if (process == pCurrent->process)
		{
			pCurrent->prev->next = pCurrent->next;
			pCurrent->next->prev = pCurrent->prev;

			pDLinkList->eleNum--;

			ExFreePool(pCurrent);

			return TRUE;
		}

		pCurrent = pCurrent->next;
	}

	return FALSE;
}

/*
	����2�������Խ���
	����4������������
*/
BOOLEAN MyDebugPortDLinkListAdd(IN PMyDebugPortDLinkList pDLinkList, IN ULONG process, IN ULONG debugObject, IN ULONG debuggerProcess)
{
	if (NULL == pDLinkList)
	{
		alert("MyDebugPortDLinkListAdd: pDLinkList=NULL");

		return FALSE;
	}

	if (NULL == process)
	{
		alert("MyDebugPortDLinkListAdd: eprocess=NULL");

		return FALSE;
	}

	if (NULL == debugObject)
	{
		alert("MyDebugPortDLinkListAdd: DebugObject=NULL");

		return FALSE;
	}

	if (0 != MyDebugPortDLinkListFind(pDLinkList, process))
	{
		MyDebugPortDLinkListDelete(pDLinkList, process);
	}

	PMyDebugPort pDebugPort = createMyDebugPort(process, debugObject, debuggerProcess);
	if (NULL == pDebugPort)
	{
		alert("MyDebugPortDLinkListAdd: createMyDebugPort=NULL");

		return FALSE;
	}

	pDLinkList->top->next->prev = pDebugPort;

	pDebugPort->prev = pDLinkList->top;
	pDebugPort->next = pDLinkList->top->next;

	pDLinkList->top->next = pDebugPort;

	pDLinkList->eleNum++;

	return TRUE;
}

