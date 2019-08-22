#include "process.h"

/*
	��ȡĳ�����̵�pid

	win7 x32: eprocess + 0x0b4
*/
ULONG Proc_GetPid(IN PEPROCESS process)
{
	if (NULL == process)
	{
		alert("Proc_GetPid: process=NULL");

		return 0;
	}

	return *(PULONG)((ULONG)process + 0x0b4);
}

ULONG Proc_GetPidByName(IN PSTR procName)
{
	if (NULL == procName)
	{
		alert("Proc_GetPidByName: procName=NULL");

		return 0;
	}

	PEPROCESS process = (PEPROCESS)GetProcessByName(procName);
	if (!MmIsAddressValid(process))
	{
		alert("Proc_GetPidByName: process=NULL");

		return 0;
	}

	return Proc_GetPid(process);
}

ULONG Proc_GetCurrentPid()
{
	return Proc_GetPid(PsGetCurrentProcess());
}

/*
	��ȡĳ�����̵ľ����ļ�������������
*/
PSTR GetImageFileName(IN PEPROCESS process)
{
	if (NULL == process)
	{
		return NULL;
	}

	return (PSTR)((ULONG_PTR)process + 0x16c);
}

/*
	��ȡ��ǰ���̵ľ����ļ�������������
*/
PSTR GetCurrentImageFileName()
{
	return GetImageFileName(PsGetCurrentProcess());
}

/*
	��������
*/
VOID TraverseProcess()
{
	ANSI_STRING tmpProcessName = { 0 };

	ULONG currentProcess = 0;
	ULONG tmpProcess = 0;

	PCSTR processNameAddr = 0;

	PLIST_ENTRY pProcessListHead = NULL;
	PLIST_ENTRY pProcessNext = NULL;

	currentProcess = (ULONG)PsGetCurrentProcess();
	tmpProcess = currentProcess;

	pProcessListHead = (PLIST_ENTRY)(currentProcess + 0x0b8);
	pProcessNext = pProcessListHead;

	do {
		processNameAddr = (PCSTR)(tmpProcess + 0x16c);
		RtlInitAnsiString(&tmpProcessName, processNameAddr);

		DbgPrint("%s \n", tmpProcessName.Buffer);

		pProcessNext = pProcessNext->Blink;
		tmpProcess = (ULONG)pProcessNext - 0x0b8;
	} while (pProcessListHead != pProcessNext);
}

/*
	���̶���
		procName��������
		   IsAll���Ƿ�������и����ƵĽ��̣����ͬ���Ľ��̣�
*/
VOID CutProcessLink(IN PCSTR procName, IN BOOLEAN IsAll)
{
	if (NULL == procName)
	{
		alert("CutProcessLink: procName=NULL");

		return;
	}

	ANSI_STRING cutProcessName = { 0 };
	ANSI_STRING tmpProcessName = { 0 };

	ULONG currentProcess = 0;
	ULONG tmpProcess = 0;

	PCSTR processNameAddr = 0;

	PLIST_ENTRY pProcessListHead = NULL;
	PLIST_ENTRY pProcessNext = NULL;

	PLIST_ENTRY pCutPrev = NULL;
	PLIST_ENTRY pCutNext = NULL;

	RtlInitAnsiString(&cutProcessName, procName);

	currentProcess = (ULONG)PsGetCurrentProcess();
	tmpProcess = currentProcess;

	pProcessListHead = (PLIST_ENTRY)(currentProcess + 0x0b8);
	pProcessNext = pProcessListHead;

	do {
		processNameAddr = (PCSTR)(tmpProcess + 0x16c);
		RtlInitAnsiString(&tmpProcessName, processNameAddr);

		if (0 == RtlCompareString(&cutProcessName, &tmpProcessName, FALSE)) {
			pCutPrev = pProcessNext->Flink;
			pCutNext = pProcessNext->Blink;

			pCutPrev->Blink = pCutNext;
			pCutNext->Flink = pCutPrev;

			if (0 == IsAll) break;
		}

		pProcessNext = pProcessNext->Blink;
		tmpProcess = (ULONG)pProcessNext - 0x0b8;
	} while (pProcessListHead != pProcessNext);
}

/*
	�жϵ�ǰ�����ǲ���ָ���Ľ��̣�δ���ԣ�����ʹ��ʱ���ԣ�
*/
BOOLEAN IsSpecifiedProcess(IN PSTR procName)
{
	PSTR currentProcessName = GetCurrentImageFileName();

	if (0 == strcmp(procName, currentProcessName))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOLEAN IsCE()
{
	return IsSpecifiedProcess("MoCPlan.exe");
}

BOOLEAN IsOD()
{
	return IsSpecifiedProcess("MoOPlan.exe");
}

BOOLEAN IsSelfDebug()
{
	return IsSpecifiedProcess("MoTPlan.exe");
}

/*
	����ǲ�����dnf�����Ľ���

	DNF.exe TPHelper.exe GameLoader.exe CrossProxy.exe Client.exe TenSafe_1.exe TenioDL.exe TclsQmFix.exe
*/
BOOLEAN IsDNFFamily()
{
	return IsSpecifiedProcess("DNF.exe") || IsSpecifiedProcess("TPHelper.exe") || IsSpecifiedProcess("GameLoader.exe") || IsSpecifiedProcess("CrossProxy.exe") || IsSpecifiedProcess("Client.exe") \
		|| IsSpecifiedProcess("TenSafe_1.exe") || IsSpecifiedProcess("TenioDL.exe") || IsSpecifiedProcess("TclsQmFix.exe");
}

BOOLEAN IsDNFFamilyByPid()
{
	return TRUE;
}

BOOLEAN IsDNFClient()
{
	return IsSpecifiedProcess("Client.exe");
}

BOOLEAN IsDNF()
{
	return IsSpecifiedProcess("DNF.exe");
}

BOOLEAN IsTASLogin()
{
	return IsSpecifiedProcess("TASLogin.exe");
}

BOOLEAN IsNtQueryInformationProcess()
{
	return IsSpecifiedProcess("MoQPlan.exe");
}

/*
	���ݽ�������ȡeprocess
*/
ULONG GetProcessByName(IN PSTR procName)
{
	ULONG_PTR result = NULL;

	if (NULL == procName)
	{
		alert("GetProcessByName: procName=NULL");

		return result;
	}

	ANSI_STRING cutProcessName = { 0 };
	ANSI_STRING tmpProcessName = { 0 };

	ULONG currentProcess = 0;
	ULONG tmpProcess = 0;

	PCSTR processNameAddr = 0;

	PLIST_ENTRY pProcessListHead = NULL;
	PLIST_ENTRY pProcessNext = NULL;

	RtlInitAnsiString(&cutProcessName, procName);

	currentProcess = (ULONG)PsGetCurrentProcess();
	tmpProcess = currentProcess;

	pProcessListHead = (PLIST_ENTRY)(currentProcess + 0x0b8);
	pProcessNext = pProcessListHead;

	do {
		processNameAddr = (PCSTR)(tmpProcess + 0x16c);
		RtlInitAnsiString(&tmpProcessName, processNameAddr);

		if (0 == RtlCompareString(&cutProcessName, &tmpProcessName, FALSE)) {
			return (ULONG)pProcessNext - 0x0b8;
		}

		pProcessNext = pProcessNext->Blink;
		tmpProcess = (ULONG)pProcessNext - 0x0b8;
	} while (pProcessListHead != pProcessNext);

	return result;
}

/*
	����pid��ȡeprocess
*/
ULONG GetProcessByPid(IN ULONG pid)
{
	if (0 == pid)
	{
		alert("GetProcessByPid: pid=0");

		return 0;
	}

	ULONG currentProcess = 0;
	ULONG tmpProcess = 0;
	ULONG tmpPid = 0;

	PLIST_ENTRY pProcessListHead = NULL;
	PLIST_ENTRY pProcessNext = NULL;

	currentProcess = (ULONG)PsGetCurrentProcess();
	tmpProcess = currentProcess;

	pProcessListHead = (PLIST_ENTRY)(currentProcess + 0x0b8);
	pProcessNext = pProcessListHead;

	do {
		tmpPid = *(PULONG)((ULONG)pProcessNext - 0x0b8 + 0x0b4);
		if (tmpPid == pid)
		{
			return (ULONG)pProcessNext - 0x0b8;
		}

		pProcessNext = pProcessNext->Blink;
		tmpProcess = (ULONG)pProcessNext - 0x0b8;
	} while (pProcessListHead != pProcessNext);

	return 0;
}

/*
	��ȡĳ�����̵�_HANDLE_TABLE

	win7 x32��eprocess + 0x0f4
*/
ULONG GetProcessObjectTableByName(IN PSTR procName)
{
	if (NULL == procName)
	{
		alert("GetProcessObjectTableByName: procName=NULL");

		return 0;
	}

	ULONG process = GetProcessByName(procName);
	if (NULL == process)
	{
		alert("GetProcessObjectTableByName: process=MULL");

		return 0;
	}

	return *(PULONG)((ULONG)process + 0x0f4);
}

ULONG GetProcessObjectTableByPid(IN ULONG pid)
{
	if (0 == pid)
	{
		alert("GetProcessObjectTableByPid: pid=0");

		return 0;
	}

	ULONG process = GetProcessByPid(pid);
	if (NULL == process)
	{
		alert("GetProcessObjectTableByPid: process=MULL");

		return 0;
	}

	return *(PULONG)((ULONG)process + 0x0f4);
}

/*
	��ȡ��ǰ���̵�_HANDLE_TABLE

	win7 x32��eprocess + 0x0f4
*/
ULONG GetCurrentProcessObjectTable()
{
	return *(PULONG)((ULONG)PsGetCurrentProcess() + 0x0f4);
}

NTSTATUS SetDebugPort(IN ULONG process, IN ULONG val)
{
	if (0 == process)
	{
		DbgPrint("SetDebugPort: process=0 \n");
		return STATUS_UNSUCCESSFUL;
	}

	*(PULONG)(process + 0xec) = val;
	
	return STATUS_SUCCESS;
}

/************************************************************************/
/*
	��ȡ���̵�ҳĿ¼���ַ
*/
/************************************************************************/
static ULONG s_GetDirectoryTableBase(IN ULONG process)
{
	if (0 == process)
	{
		return 0;
	}

	return *(PULONG)(process + 0x18);
}

ULONG GetCurrentProcessDirectoryTableBase()
{
	return s_GetDirectoryTableBase((ULONG)PsGetCurrentProcess());
}

VOID SetCurrentProcessDirectoryTableBase(IN ULONG val)
{
	PEPROCESS process = PsGetCurrentProcess();

	*(PULONG)((ULONG)process + 0x18) = val;
}

ULONG GetProcessDirectoryTableBase(IN PEPROCESS process)
{
	return s_GetDirectoryTableBase((ULONG)process);
}

ULONG GetDirectoryTableBaseByPid(IN ULONG pid)
{
	if (0 == pid)
	{
		DbgPrint("GetDirectoryTableBaseByPid: pid=0 \n");
		return 0;
	}

	ULONG process = GetProcessByPid(pid);
	if (0 == process)
	{
		DbgPrint("GetDirectoryTableBaseByPid: process=0 \n");
		return 0;
	}

	return s_GetDirectoryTableBase(process);
}

ULONG GetDirectoryTableBaseByProcNameA(IN PSTR procName)
{
	if (NULL == procName)
	{
		DbgPrint("GetDirectoryTableBaseByProcNameA: procName=NULL \n");
		return 0;
	}

	ULONG process = GetProcessByName(procName);
	if (0 == process)
	{
		DbgPrint("GetDirectoryTableBaseByProcNameA: process=0 \n");
		return 0;
	}

	return s_GetDirectoryTableBase(process);
}

/************************************************************************/
/*
	��ȡ���̵�cr3
*/
/************************************************************************/
ULONG GetCurrentProcessCR3()
{
	ULONG cr3Val = 0;

	__asm {
		mov		eax, cr3
		mov		[cr3Val], eax
	}

	return cr3Val;
}

VOID SetCurrentProcessCR3(IN ULONG cr3Val)
{
	__asm {
		cli

		push	eax

		mov		eax, [cr3Val]
		mov		cr3, eax

		pop		eax

		sti
	}
}

/************************************************************************/
/*
	������������


*/
/************************************************************************/
NTSTATUS AttachProcess(IN PEPROCESS process)
{

	return STATUS_SUCCESS;
}

NTSTATUS UnAttachProcess(IN PEPROCESS process)
{

	return STATUS_SUCCESS;
}