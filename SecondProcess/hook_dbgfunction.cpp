#include "hook_dbgfunction.h"

/*
	ͨ�����������̵�eprocess��ȡ���Զ����ַ
*/
static ULONG s_GetDbgObjAddrByDebuggerProcess()
{
	ULONG addr = MyDebugPortDLinkListFindByDebuggerProcess(g_pDebugPortDLinkList, (ULONG)PsGetCurrentProcess());

//	DbgPrint("�����Խ��̡���ȡ���Զ����ַ��%X \n", addr);

	return addr;
}

/*
	ͨ�������Խ��̵�eprocess��ȡ���Զ����ַ
*/
static ULONG s_GetDbgObjAddr()
{
	ULONG addr = MyDebugPortDLinkListFind(g_pDebugPortDLinkList, (ULONG)PsGetCurrentProcess());

//	DbgPrint("�������Խ��̡���ȡ���Զ����ַ��%X \n", addr);

	return addr;
}

VOID Print_HookLog(IN ULONG funNameCode, IN ULONG dbgObjAddr)
{
	if (0 != dbgObjAddr)
	{
		return;
	}

	switch (funNameCode)
	{
	case 1:
		DbgPrint("��DbgkpQueueMessage�����Զ����ַ��%X \n", dbgObjAddr);
		break;
	case 2:
		DbgPrint("��DbgkCreateThread�����Զ����ַ��%X \n", dbgObjAddr);
		break;
	case 3:
		DbgPrint("��NtWaitForDebugEvent�����Զ����ַ��%X \n", dbgObjAddr);
		break;
	case 4:
		DbgPrint("��DbgkExitProcess�����Զ����ַ��%X \n", dbgObjAddr);
		break;
	case 5:
		DbgPrint("��PspExitThread�����Զ����ַ��%X \n", dbgObjAddr);
		break;
	case 6:
		DbgPrint("��DbgkExitThread�����Զ����ַ��%X \n", dbgObjAddr);
		break;
	default:
		DbgPrint("�������Խ��̡����Զ����ַ��%X \n", dbgObjAddr);
		break;
	}
}

/************************************************************************/
/*
	hook��unhook�ں˺���DbgkpSetProcessDebugObject�����������ã���jmp��ʽ��
*/
/************************************************************************/
static ULONG DbgkpSetProcessDebugObjectRetAddr;

VOID Run_DbgkpSetProcessDebugObject(IN ULONG Process, IN ULONG DebugObject)
{
	MyDebugPortDLinkListAdd(g_pDebugPortDLinkList, Process, DebugObject, (ULONG)PsGetCurrentProcess());

	DbgPrint("����������Process: %X, DebugObject:%X, DebuggerProcess: %X \n", Process, DebugObject, PsGetCurrentProcess());
}

VOID __declspec(naked) Dispatch_DbgkpSetProcessDebugObject()
{
	if (IsCE() || IsOD() || IsSelfDebug())
	{
		__asm {
			pushad
			pushfd

			mov		ecx, [ebp + 0x0c]	//DebugObject
			push	ecx

			mov		ecx, [ebp + 0x08]	//Process
			push	ecx

			call	Run_DbgkpSetProcessDebugObject

			popfd
			popad
		}
	}

	__asm {
		mov     ecx, dword ptr fs:[124h]

		jmp		DbgkpSetProcessDebugObjectRetAddr;
	}
}

NTSTATUS Hook_DbgkpSetProcessDebugObject()
{
	NTSTATUS status = STATUS_SUCCESS;

	UCHAR hookBytes[8] = { 0xe9, 0x00, 0x00, 0x00, 0x00, 0x90, 0x90, 0x89 };

	ULONG funAddr = KF_GetDbgkpSetProcessDebugObjectAddr();
	if (0 == funAddr)
	{
		alert("Hook_DbgkpSetProcessDebugObject: KF_GetDbgkpSetProcessDebugObjectAddr=0");

		return STATUS_UNSUCCESSFUL;
	}

	ULONG hookAddr = funAddr + 0x0b;

	DbgkpSetProcessDebugObjectRetAddr = hookAddr + 7;

	*(PULONG)(hookBytes + 1) = (ULONG)Dispatch_DbgkpSetProcessDebugObject - hookAddr - 5;

	Hook8b(hookAddr, hookBytes, NULL);

	operate_prompt("Hook_DbgkpSetProcessDebugObject");

	return status;
}

NTSTATUS UnHook_DbgkpSetProcessDebugObject()
{
	NTSTATUS status = STATUS_SUCCESS;

	UCHAR hookBytes[8] = { 0x64, 0x8b, 0x0d, 0x24, 0x01, 0x00, 0x00, 0x89};

	ULONG funAddr = KF_GetDbgkpSetProcessDebugObjectAddr();
	if (0 == funAddr)
	{
		alert("UnHook_DbgkpSetProcessDebugObject: KF_GetDbgkpSetProcessDebugObjectAddr=0");

		return STATUS_UNSUCCESSFUL;
	}

	ULONG hookAddr = funAddr + 0x0b;

	DbgkpSetProcessDebugObjectRetAddr = 0;

	Hook8b(hookAddr, hookBytes, NULL);

	operate_prompt("UnHook_DbgkpSetProcessDebugObject");

	return status;
}

/************************************************************************/
/*
	hook��unhook�ں˺���NtDebugActiveProcess�����������ã���jmp��ʽ��
*/
/************************************************************************/
static ULONG NtDebugActiveProcessRetAddr;

VOID Run_NtDebugActiveProcess(IN ULONG process)
{
	*(PULONG)(process + 0xec) = 0;

	if (0 == (*(PULONG)(process + 0xec)))
	{
		DbgPrint("���ɹ���DebugPort��0 \n");
	}
	else
	{
		DbgPrint("��ʧ�ܡ�DebugPort��0 \n");
	}
}

VOID __declspec(naked) Dispatch_NtDebugActiveProcess()
{
	if (IsCE() || IsOD() || IsSelfDebug())
	{
		__asm {
			pushad
			pushfd

			mov		ecx, [ebp - 4]
			push	ecx
			call	Run_NtDebugActiveProcess

			popfd
			popad
		}
	}

	__asm {
		mov     ecx, [esi]
		and     ecx, 0FFFFFFFEh

		jmp		NtDebugActiveProcessRetAddr
	}
}

NTSTATUS Hook_NtDebugActiveProcess()
{
	NTSTATUS status = STATUS_SUCCESS;

	UCHAR hookBytes[8] = { 0xe9, 0x00, 0x00, 0x00, 0x00, 0x8b, 0xf8, 0x8d };

	ULONG funAddr = KF_GetNtDebugActiveProcessAddr();
	if (0 == funAddr)
	{
		alert("Hook_NtDebugActiveProcess: KF_GetNtDebugActiveProcessAddr=0");

		return STATUS_UNSUCCESSFUL;
	}

	ULONG hookAddr = funAddr + 0xf2;

	NtDebugActiveProcessRetAddr = hookAddr + 5;

	*(PULONG)(hookBytes + 1) = (ULONG)Dispatch_NtDebugActiveProcess - hookAddr - 5;

	Hook8b(hookAddr, hookBytes, NULL);

	operate_prompt("Hook_NtDebugActiveProcess");

	return status;
}

NTSTATUS UnHook_NtDebugActiveProcess()
{
	NTSTATUS status = STATUS_SUCCESS;

	UCHAR hookBytes[8] = { 0x8b, 0x0e, 0x83, 0xe1, 0xfe, 0x8b, 0xf8, 0x8d };

	ULONG funAddr = KF_GetNtDebugActiveProcessAddr();
	if (0 == funAddr)
	{
		alert("UnHook_NtDebugActiveProcess: KF_GetNtDebugActiveProcessAddr=0");

		return STATUS_UNSUCCESSFUL;
	}

	ULONG hookAddr = funAddr + 0xf2;

	NtDebugActiveProcessRetAddr = 0;

	Hook8b(hookAddr, hookBytes, NULL);

	operate_prompt("UnHook_NtDebugActiveProcess");

	return status;
}

/************************************************************************/
/*
	��ת����д��NtDebugActiveProcess��ͷ5�ֽڣ���jmp��ʽ��
*/
/************************************************************************/
static ULONG NtDebugActiveProcessRetAddr_rewrite;

/*
	���ݲ�����ʾִ�е�λ�ã������Ŵ�
*/
VOID Print_Hook_NtDebugActiveProcess_rewrite(IN ULONG printCode, IN ULONG objAddr)
{
	switch (printCode)
	{
	case 1:
		DbgPrint("�����Խ���eprocess��0x%08X \n", objAddr);
		break;
	case 2:
		DbgPrint("�����Խ��̷����ѡ��ǳ�ʼ��ϵͳ���� \n");
		break;
	case 3:
		DbgPrint("���û�ģʽ���Ǳ������� \n");
		break;
	case 4:
		DbgPrint("������eprocess��0x%08X \n", objAddr);
		break;
	case 5:
		DbgPrint("DbgkpPostFakeProcessCreateMessagesִ�н����Status = 0x%08X \n", objAddr);
		break;
	case 6:
		DbgPrint("DbgkpSetProcessDebugObjectִ�н����Status = 0x%08X \n", objAddr);
		break;
	case 7:
		DbgPrint("��ȡ���Զ����ַǰ����� = %X \n", objAddr);
		break;
	case 8:
		DbgPrint("��ȡ���Զ���ִ�н����Status = %08X \n", objAddr);
		break;
	default:
		break;
	}
}

VOID Run_Hook_NtDebugActiveProcess_rewrite(IN ULONG process, IN ULONG debugObjAddr)
{
	if (0 == process)
	{
		DbgPrint("Run_Hook_NtDebugActiveProcess_rewrite: process = 0 \n");
		return;
	}

	if (0 == debugObjAddr)
	{
		DbgPrint("Run_Hook_NtDebugActiveProcess_rewrite: debugObjAddr = 0 \n");
		return;
	}

	if (MyDebugPortDLinkListAdd(g_pDebugPortDLinkList, process, debugObjAddr, (ULONG)PsGetCurrentProcess()));
	{
		DbgPrint("NtDebugActiveProcess: process: %08X, debugObjAddr: %08X, debuggerProcess: %08X \n", process, debugObjAddr, PsGetCurrentProcess());
	}

	SetDebugPort(process, 0);
}

VOID Print_DbgkpPostFakeProcessCreateMessages(IN ULONG printCode, IN ULONG objAddr)
{
	switch (printCode)
	{
	case 1:
		DbgPrint("DbgkpPostFakeThreadMessages result: 0x%08X \n", objAddr);
		break;
	case 2:
		DbgPrint("KeStackAttachProcess result: 0x%08X \n", objAddr);
		break;
	case 3:
		DbgPrint("DbgkpPostModuleMessages result: 0x%08X \n", objAddr);
		break;
	case 4:
		DbgPrint("KeUnstackDetachProcess result: 0x%08X \n", objAddr);
		break;
	case 5:
		DbgPrint("DbgkpPostFakeProcessCreateMessages running \n", objAddr);
		break;
	default:
		break;
	}
}

NTSTATUS __declspec(naked) DbgkpPostFakeProcessCreateMessages(IN PEPROCESS Process, IN PDEBUG_OBJECT DebugObject, OUT PETHREAD *LastThread)
{
	__asm {
//		int 3
		mov     edi, edi
		push    ebp
		mov     ebp, esp
		and     esp, 0FFFFFFF8h
		sub     esp, 20h
	}

	__asm {
//		push	0
//		push	5
//		call	Print_DbgkpPostFakeProcessCreateMessages

//		jmp		End1
	}

	__asm {
		lea     eax, [esp + 4]
		push    eax
		lea     eax, [esp + 4]
		push    eax
		push    dword ptr[ebp + 0Ch]
		xor     ecx, ecx
		push    dword ptr[ebp + 8]

		//��Ҫ�����ֳ�����������
		push	ecx
		push	edx
		push	ebx
		push	esi
		push	edi

		call	KF_GetDbgkpPostFakeThreadMessagesAddr

		pop		edi
		pop		esi 
		pop		ebx
		pop		edx 
		pop		ecx

		call	eax

		test    eax, eax
		jl		End1
	}

	__asm {
//		push	eax
//		push	1
//		call	Print_DbgkpPostFakeProcessCreateMessages

//		jmp		End1
 	}

	__asm {
		lea     eax, [esp + 8]
		push    eax
		push    dword ptr[ebp + 8]

		push	ecx
		push	edx
		push	ebx
		push	esi
		push	edi
		
		call	KF_GetKeStackAttachProcessAddr

		pop		edi
		pop		esi
		pop		ebx
		pop		edx
		pop		ecx

		call	eax
	}

	__asm {
//		push	eax
//		push	2
//		call	Print_DbgkpPostFakeProcessCreateMessages

//		jmp		End1
	}

	__asm {
		push    dword ptr[ebp + 0Ch]
		mov     ecx, dword ptr[ebp + 8]
		push    dword ptr[esp + 4]

		push	ecx
		push	edx
		push	ebx
		push	esi
		push	edi
		
		call	KF_GetDbgkpPostModuleMessagesAddr

		pop		edi
		pop		esi
		pop		ebx
		pop		edx
		pop		ecx

		call	eax
	}

	__asm {
//		push	eax
//		push	3
//		call	Print_DbgkpPostFakeProcessCreateMessages

//		jmp		End1
	}

	__asm {
		lea     eax, [esp + 8]
		push    eax

		push	ecx
		push	edx
		push	ebx
		push	esi
		push	edi

		call	KF_GetKeUnstackDetachProcessAddr

		pop		edi
		pop		esi
		pop		ebx
		pop		edx
		pop		ecx

		call	eax
	}

	__asm {
//		push	eax
//		push	4
//		call	Print_DbgkpPostFakeProcessCreateMessages

//		jmp		End1
	}

	__asm {
		mov     ecx, dword ptr[esp]
		call	ObfDereferenceObject
		mov     ecx, dword ptr[esp + 4]
		xor     eax, eax
		jmp		End2
	}

	__asm {
	End1:
		xor     ecx, ecx
	End2:
		mov     edx, dword ptr[ebp + 10h]
		mov     dword ptr[edx], ecx
		mov     esp, ebp
		pop     ebp
		ret     0Ch
	}
}

VOID __declspec(naked) Dispatch_NtDebugActiveProcess_rewrite()
{
	if (IsCE() || IsOD() || IsSelfDebug())
	{
		__asm {
			//int 3

			//����
			//jmp		RetFun

			//ִ����c��д��NtDebugActiveProcess��δ�ɹ���ԭ���꣩
			//push	NtDebugActiveProcess
			//ret
		}

		__asm {
			mov     edi, edi
			push    ebp
			mov     ebp, esp
			sub     esp, 10h
			mov     eax, dword ptr fs : [00000124h]
			push    ebx
			mov     bl, byte ptr[eax + 13Ah]
			push    edi
		}

		//���ݾ����ȡ�����Խ��̵�eprocess
		__asm {
			push    0
			lea     eax, [ebp - 4]
			push    eax
			mov     byte ptr[ebp - 0Ch], bl
			push    dword ptr[ebp - 0Ch]
			mov     edi, 800h

			//����PsProcessType��ֵ��ϵͳ�ṩ��PsProcessTypeֻ�ǵ�ַ��
			mov		eax, PsProcessType
			mov		eax, [eax]
			push	eax

			push    edi
			push    dword ptr[ebp + 8]
			call	ObReferenceObjectByHandle
			test    eax, eax
			jl		End2
		}

		__asm {
			//�жϵ�ǰ���Ե��ǲ����Լ����Ǿ���ת������
			mov     eax, dword ptr fs : [00000124h]
			push    esi
			mov     esi, dword ptr[ebp - 4]
			cmp     esi, dword ptr[eax + 50h]
			je		End3

			//�жϵ�ǰ���Ե��ǲ��ǳ�ʼ��ϵͳ���̣��Ǿ���ת������
			cmp     esi, PsInitialSystemProcess
			je		End3
		}

		__asm {
			//�жϵ�ǰ�̵߳���ǰģʽ�Ƿ����1��UserMode�����Ǿ���ת
			cmp     bl, 1
			jne		NoUserMode

			//�ж�CurrentProcess.ProtectedProcess�Ƿ����0����������ת
			mov     eax, dword ptr fs : [00000124h]
			mov     eax, dword ptr[eax + 50h]
			mov     esi, dword ptr[ebp - 4]
			test    dword ptr[eax + 26Ch], edi
			jne		CurrentProcessProtectedProcessNoZero

			//�ж�Process.ProtectedProcess�Ƿ����0��������ת
			test    dword ptr[esi + 26Ch], edi
			je		ProcessProtectedProcessIsZero

			mov     ecx, esi
			call	ObfDereferenceObject
			mov     eax, 0C0000712h
			jmp		End1
		}

		//ͨ�����������̵�eprocess��ȡ���Զ���ĵ�ַ
		__asm {
		NoUserMode:
		CurrentProcessProtectedProcessNoZero :
		ProcessProtectedProcessIsZero :
			call	s_GetDbgObjAddrByDebuggerProcess
			cmp		eax, 0
			je		End6

			mov		[ebp - 8], eax

			//���Ӷ�������ü���������ɾ��ʱ������
			mov		ecx, eax
			call	ObfReferenceObject

			xor		eax, eax
			mov     edi, eax
		}

		/*
		__asm {
			push	[ebp - 8]
			push	4
			call	Print_Hook_NtDebugActiveProcess_rewrite

			mov     eax, 0C0000022h
			jl		End2
		}
		*/
		
/*
		//ͨ�������ȡ���Զ���ĵ�ַ
		__asm {
		NoUserMode:
		CurrentProcessProtectedProcessNoZero:
		ProcessProtectedProcessIsZero:
			push    0
			lea     eax, [ebp - 8]
			push    eax
			push    dword ptr[ebp - 0Ch]

			call	KF_GetDbgkDebugObjectTypeAddr
			mov		eax, [eax]
			push	eax

			push    2
			push    dword ptr[ebp + 0Ch]
		}

		__asm {
			push	[ebp + 0Ch]
			push	7
			call	Print_Hook_NtDebugActiveProcess_rewrite
		}

		__asm {
			call	ObReferenceObjectByHandle
		}

		__asm {
			push	eax
			push	8
			call	Print_Hook_NtDebugActiveProcess_rewrite
		}
		
		__asm {
			mov     edi, eax
			test    edi, edi
			jl		End6
		}
*/
//		__asm {
//			push	[ebp - 8]
//			push	4
//			call	Print_Hook_NtDebugActiveProcess_rewrite
//		}

		__asm {
			//Process.RundownProtect����һЩ���㡢����
			add     esi, 0B0h
			mov     ecx, dword ptr[esi]
			and     ecx, 0FFFFFFFEh
			lea     eax, [ecx + 2]
			mov     edx, eax
			mov     edi, esi
			mov     eax, ecx
			lock cmpxchg dword ptr[edi], edx
			cmp     eax, ecx
			je		Code1

			mov     ecx, esi
			call	ExAcquireRundownProtection
			test    al, al
			je		End4
		}

		__asm {
		Code1 :
			lea     eax, [ebp - 10h]
			push    eax
			push    dword ptr[ebp - 8]
			push    dword ptr[ebp - 4]

			call	KF_GetDbgkpPostFakeProcessCreateMessagesAddr
			call	eax

//			call	DbgkpPostFakeProcessCreateMessages
		}

		__asm {
//			push	eax
//			push	5
//			call	Print_Hook_NtDebugActiveProcess_rewrite

//			mov     eax, 0C0000022h
//			jmp		End2
		}

		__asm {
			push    dword ptr[ebp - 10h]
			push    dword ptr[ebp - 8]
			push    dword ptr[ebp - 4]

			call	KF_GetDbgkpSetProcessDebugObjectAddr
			mov		ecx, eax
			xor		eax, eax
			call	ecx
		}

		//1������˫������
		//2��DebugPort��0
		__asm {
			push	[ebp - 8]
			push	[ebp - 4]
			call	Run_Hook_NtDebugActiveProcess_rewrite
		}
		
		__asm {
		Code2:
			mov     ecx, dword ptr[esi]
			and     ecx, 0FFFFFFFEh
			mov     edi, eax
			lea     eax, [ecx - 2]
			mov     edx, eax
			mov     ebx, esi
			mov     eax, ecx
			lock cmpxchg dword ptr[ebx], edx
			cmp     eax, ecx
			je		End5

			mov     ecx, esi
			call	ExReleaseRundownProtection
			jmp		End5

		End4:
			mov     edi, 0C000010Ah
		End5:
			mov     ecx,dword ptr [ebp-8]
			call	ObfDereferenceObject
			mov     esi, dword ptr[ebp - 4]
		End6:	
			mov     ecx, esi
			call	ObfDereferenceObject
			mov     eax, edi
			jmp		End1
		End3:
			mov     ecx, esi
			call	ObfDereferenceObject
			mov     eax, 0C0000022h
		End1:
			pop     esi
		End2:
			pop     edi
			pop     ebx
			leave
			ret     8
		}
	}

	__asm {
		RetFun:
		mov     edi, edi
		push    ebp
		mov     ebp,esp

		jmp		NtDebugActiveProcessRetAddr_rewrite
	}
}

NTSTATUS Hook_NtDebugActiveProcess_rewrite()
{
	NTSTATUS status = STATUS_SUCCESS;

	UCHAR hookBytes[8] = { 0xe9, 0x00, 0x00, 0x00, 0x00, 0x83, 0xec, 0x10 };

	ULONG funAddr = KF_GetNtDebugActiveProcessAddr();
	if (0 == funAddr)
	{
		alert("Hook_NtDebugActiveProcess_rewrite: KF_GetNtDebugActiveProcessAddr=0");

		return STATUS_UNSUCCESSFUL;
	}
	
	ULONG hookAddr = funAddr;

	NtDebugActiveProcessRetAddr_rewrite = hookAddr + 5;

	*(PULONG)(hookBytes + 1) = (ULONG)Dispatch_NtDebugActiveProcess_rewrite - hookAddr - 5;

	Hook8b(hookAddr, hookBytes, NULL);

	operate_prompt("Hook_NtDebugActiveProcess_rewrite");

	return status;
}

NTSTATUS UnHook_NtDebugActiveProcess_rewrite()
{
	NTSTATUS status = STATUS_SUCCESS;

	UCHAR hookBytes[8] = { 0x8b, 0xff, 0x55, 0x8b, 0xec, 0x83, 0xec, 0x10 };

	ULONG funAddr = KF_GetNtDebugActiveProcessAddr();
	if (0 == funAddr)
	{
		alert("UnHook_NtDebugActiveProcess_rewrite: KF_GetNtDebugActiveProcessAddr=0");

		return STATUS_UNSUCCESSFUL;
	}

	ULONG hookAddr = funAddr;

	NtDebugActiveProcessRetAddr_rewrite = 0;

	Hook8b(hookAddr, hookBytes, NULL);

	operate_prompt("UnHook_NtDebugActiveProcess_rewrite");

	return status;
}

/************************************************************************/
/*
	hook��unhook�ں˺���NtWaitForDebugEvent�����������ã���jmp��ʽ��
*/
/************************************************************************/
static ULONG NtWaitForDebugEventRetAddr;

VOID __declspec(naked) Dispatch_NtWaitForDebugEvent()
{
	if (IsCE() || IsOD() || IsSelfDebug())
	{
		__asm {
			call	s_GetDbgObjAddrByDebuggerProcess

			cmp		eax, 0
			je		RetFun

			mov		[ebp - 28h], eax

			//���Ӷ�������ü���������ɾ��ʱ������
			mov		ecx, eax
			call	ObfReferenceObject

			//���óɹ������ؽ��eax��Ϊ0��ObReferenceObjectByHandleִ�к�Ľ����eax�У�
			xor		eax, eax
			
			//�������ݵ��Զ�������ȡ���Զ���Ĵ���
			mov		esi, NtWaitForDebugEventRetAddr
			add		esi, 0x13
			jmp		esi
		}
	}

	__asm {
	RetFun:
		push    ebx
		lea     eax, [ebp - 28h]
		push    eax

		jmp		NtWaitForDebugEventRetAddr
	}
}

NTSTATUS Hook_NtWaitForDebugEvent()
{
	NTSTATUS status = STATUS_SUCCESS;

	UCHAR hookBytes[8] = { 0xe9, 0x00, 0x00, 0x00, 0x00, 0xff, 0x75, 0xd4 };
	
	ULONG funAddr = KF_GetNtWaitForDebugEventAddr();
	if (0 == funAddr)
	{
		alert("Hook_NtWaitForDebugEvent: KF_GetNtWaitForDebugEventAddr=0");

		return STATUS_UNSUCCESSFUL;
	}

	ULONG hookAddr = funAddr + 0xab;

	NtWaitForDebugEventRetAddr = hookAddr + 5;

	*(PULONG)(hookBytes + 1) = (ULONG)Dispatch_NtWaitForDebugEvent - hookAddr - 5;

	Hook8b(hookAddr, hookBytes, NULL);

	operate_prompt("Hook_NtWaitForDebugEvent");

	return status;
}

NTSTATUS UnHook_NtWaitForDebugEvent()
{
	NTSTATUS status = STATUS_SUCCESS;

	UCHAR hookBytes[8] = { 0x53, 0x8d, 0x45, 0xd8, 0x50, 0xff, 0x75, 0xd4 };

	ULONG funAddr = KF_GetNtWaitForDebugEventAddr();
	if (0 == funAddr)
	{
		alert("UnHook_NtWaitForDebugEvent: KF_GetNtWaitForDebugEventAddr=0");

		return STATUS_UNSUCCESSFUL;
	}

	ULONG hookAddr = funAddr + 0xab;

	NtWaitForDebugEventRetAddr = 0;

	Hook8b(hookAddr, hookBytes, NULL);

	operate_prompt("UnHook_NtWaitForDebugEvent");

	return status;
}

/************************************************************************/
/*
	hook��unhook�ں˺���DbgkpQueueMessage�������Խ��̵��ã���jmp��ʽ��
*/
/************************************************************************/
VOID __declspec(naked) Dispatch_DbgkpQueueMessage()
{
	__asm {
		call	s_GetDbgObjAddr
		cmp		eax, 0

		je		retFun
		
		ret
		
retFun :
		//Ϊ���ȶ��Լӵ�
		mov     eax, dword ptr[ebp + 8]

		//ִ��hook������
		mov     eax, dword ptr[eax + 0ECh]

		ret
	}
}

NTSTATUS Hook_DbgkpQueueMessage()
{
	NTSTATUS status = STATUS_SUCCESS;

	UCHAR hookBytes[8] = { 0xe8, 0x00, 0x00, 0x00, 0x00, 0x90, 0x89, 0x44 };

	ULONG funAddr = KF_GetDbgkpQueueMessageAddr();
	if (0 == funAddr)
	{
		alert("Hook_DbgkpQueueMessage: KF_GetDbgkpQueueMessageAddr=0");

		return STATUS_UNSUCCESSFUL;
	}
	
	ULONG hookAddr = funAddr + 0xad;

	*(PULONG)(hookBytes + 1) = (ULONG)Dispatch_DbgkpQueueMessage - hookAddr - 5;

	Hook8b(hookAddr, hookBytes, NULL);

	operate_prompt("Hook_DbgkpQueueMessage");

	return status;
}

NTSTATUS UnHook_DbgkpQueueMessage()
{
	NTSTATUS status = STATUS_SUCCESS;

	UCHAR hookBytes[8] = { 0x8b, 0x80, 0xec, 0x00, 0x00, 0x00, 0x89, 0x44 };

	ULONG funAddr = KF_GetDbgkpQueueMessageAddr();
	if (0 == funAddr)
	{
		alert("Hook_DbgkpQueueMessage: KF_GetDbgkpQueueMessageAddr=0");

		return STATUS_UNSUCCESSFUL;
	}

	ULONG hookAddr = funAddr + 0xad;

	Hook8b(hookAddr, hookBytes, NULL);

	operate_prompt("UnHook_DbgkpQueueMessage");

	return status;
}

/************************************************************************/
/*
	hook��unhook�ں˺���KiDispatchException�������Խ��̵��ã���call��ʽ��
*/
/************************************************************************/
VOID __declspec(naked) Dispatch_KiDispatchException()
{
	__asm {
		call	s_GetDbgObjAddr
		cmp		eax, 0

		je		retFun
		
		ret

retFun:
		//Ϊ���ȶ��Լӵ�
		mov     eax, dword ptr fs : [00000124h]
		mov     eax, dword ptr[eax + 50h]

		//ִ��hook������
		cmp     dword ptr[eax + 0ECh], esi

		ret
	}
}

NTSTATUS Hook_KiDispatchException()
{
	NTSTATUS status = STATUS_SUCCESS;

	UCHAR hookBytes[8] = { 0xe8, 0x00, 0x00, 0x00, 0x00, 0x90, 0x75, 0x09 };

	ULONG funAddr = KF_GetKiDispatchExceptionAddr();
	if (0 == funAddr)
	{
		alert("Hook_KiDispatchException: KF_GetKiDispatchExceptionAddr=0");

		return STATUS_UNSUCCESSFUL;
	}

	ULONG hookAddr = funAddr + 0x1d9;

	*(PULONG)(hookBytes + 1) = (ULONG)Dispatch_KiDispatchException - hookAddr - 5;

	Hook8b(hookAddr, hookBytes, NULL);

	operate_prompt("Hook_KiDispatchException");

	return status;
}

NTSTATUS UnHook_KiDispatchException()
{
	NTSTATUS status = STATUS_SUCCESS;

	UCHAR hookBytes[8] = { 0x39, 0xb0, 0xec, 0x00, 0x00, 0x00, 0x75, 0x09 };

	ULONG funAddr = KF_GetKiDispatchExceptionAddr();
	if (0 == funAddr)
	{
		alert("UnHook_KiDispatchException: KF_GetKiDispatchExceptionAddr=0");

		return STATUS_UNSUCCESSFUL;
	}

	ULONG hookAddr = funAddr + 0x1d9;

	Hook8b(hookAddr, hookBytes, NULL);

	operate_prompt("UnHook_KiDispatchException");

	return status;
}

/************************************************************************/
/*
	hook��unhook�ں˺���DbgkForwardException�������Խ��̵��ã���call��ʽ��
*/
/************************************************************************/
VOID __declspec(naked) Dispatch_DbgkForwardException()
{
	__asm {
		call	s_GetDbgObjAddr
		cmp		eax, 0

		je		retFun
		
		mov		ebx, eax
		ret

retFun:
		//Ϊ���ȶ��Լӵ�
		mov     eax, dword ptr fs : [00000124h]
		mov     eax, dword ptr[eax + 50h]

		//ִ��hook������
		mov     ebx, dword ptr[eax + 0ECh]

		ret
	}
}

NTSTATUS Hook_DbgkForwardException()
{
	NTSTATUS status = STATUS_SUCCESS;

	UCHAR hookBytes[8] = { 0xe8, 0x00, 0x00, 0x00, 0x00, 0x90, 0x32, 0xc0 };

	ULONG funAddr = KF_GetDbgkForwardExceptionAddr();
	if (0 == funAddr)
	{
		alert("Hook_DbgkForwardException: KF_GetDbgkForwardExceptionAddr=0");

		return STATUS_UNSUCCESSFUL;
	}

	ULONG hookAddr = funAddr + 0x49;

	*(PULONG)(hookBytes + 1) = (ULONG)Dispatch_DbgkForwardException - hookAddr - 5;

	Hook8b(hookAddr, hookBytes, NULL);

	operate_prompt("Hook_DbgkForwardException");

	return status;
}

NTSTATUS UnHook_DbgkForwardException()
{
	NTSTATUS status = STATUS_SUCCESS;

	UCHAR hookBytes[8] = { 0x8b, 0x98, 0xec, 0x00, 0x00, 0x00, 0x32, 0xc0 };

	ULONG funAddr = KF_GetDbgkForwardExceptionAddr();
	if (0 == funAddr)
	{
		alert("UnHook_DbgkForwardException: KF_GetDbgkForwardExceptionAddr=0");

		return STATUS_UNSUCCESSFUL;
	}

	ULONG hookAddr = funAddr + 0x49;

	Hook8b(hookAddr, hookBytes, NULL);

	operate_prompt("UnHook_DbgkForwardException");

	return status;
}

/************************************************************************/
/*
	hook��unhook�ں˺���PspExitThread�������Խ��̵��ã���call��ʽ��
*/
/************************************************************************/
VOID __declspec(naked) Dispatch_PspExitThread()
{
	__asm {
		call	s_GetDbgObjAddr
		cmp		eax, 0

		je		retFun

#ifdef _OPERATE_PROMPT
		push	eax
		push	5
		call	Print_HookLog
#endif
		
		ret

	retFun:
		//hook������
		cmp     dword ptr[edi + 0ECh], 0

		ret
	}
}

NTSTATUS Hook_PspExitThread()
{
	NTSTATUS status = STATUS_SUCCESS;

	UCHAR hookBytes[8] = { 0xe8, 0x00, 0x00, 0x00, 0x00, 0x90, 0x90, 0x74 };

	ULONG funAddr = KF_GetPspExitThreadAddr();
	if (0 == funAddr)
	{
		alert("Hook_PspExitThread: KF_GetPspExitThreadAddr=0");

		return STATUS_UNSUCCESSFUL;
	}

	ULONG hookAddr = funAddr + 0x2b6;

	*(PULONG)(hookBytes + 1) = (ULONG)Dispatch_PspExitThread - hookAddr - 5;

	Hook8b(hookAddr, hookBytes, NULL);

	operate_prompt("Hook_PspExitThread");

	return status;
}

NTSTATUS UnHook_PspExitThread()
{
	NTSTATUS status = STATUS_SUCCESS;

	UCHAR hookBytes[8] = { 0x83, 0xbf, 0xec, 0x00, 0x00, 0x00, 0x00, 0x74 };

	ULONG funAddr = KF_GetPspExitThreadAddr();
	if (0 == funAddr)
	{
		alert("UnHook_PspExitThread: KF_GetPspExitThreadAddr=0");

		return STATUS_UNSUCCESSFUL;
	}

	ULONG hookAddr = funAddr + 0x2b6;

	Hook8b(hookAddr, hookBytes, NULL);

	operate_prompt("UnHook_PspExitThread");

	return status;
}

/************************************************************************/
/*
	hook��unhook�ں˺���DbgkExitThread�������Խ��̵��ã���call��ʽ��
*/
/************************************************************************/
VOID __declspec(naked) Dispatch_DbgkExitThread()
{
	__asm {
		push	eax

		call	s_GetDbgObjAddr
		cmp		eax, 0

		je		retFun

#ifdef _OPERATE_PROMPT
		push	eax
		push	6
		call	Print_HookLog
#endif
		
		//������eax���жϣ�ǰ��ĺ��������޸���eax
		mov     eax, fs:[124h]
		mov     eax, [eax + 280h]

		pop		eax
		ret

	retFun:
		pop		eax

		//Ϊ���ȶ����ϵ�
		mov     ecx, dword ptr fs : [124h]
		mov     ecx, dword ptr[ecx + 50h]

		//hook������
		cmp     dword ptr[ecx + 0ECh], 0

		ret
	}
}

NTSTATUS Hook_DbgkExitThread()
{
	NTSTATUS status = STATUS_SUCCESS;

	UCHAR hookBytes[8] = { 0xe8, 0x00, 0x00, 0x00, 0x00, 0x90, 0x90, 0x74 };

	ULONG funAddr = KF_GetDbgkExitThreadAddr();
	if (0 == funAddr)
	{
		alert("Hook_DbgkExitThread: KF_GetDbgkExitThreadAddr=0");

		return STATUS_UNSUCCESSFUL;
	}

	ULONG hookAddr = funAddr + 0x28;

	*(PULONG)(hookBytes + 1) = (ULONG)Dispatch_DbgkExitThread - hookAddr - 5;

	Hook8b(hookAddr, hookBytes, NULL);

	operate_prompt("Hook_DbgkExitThread");

	return status;
}

NTSTATUS UnHook_DbgkExitThread()
{
	NTSTATUS status = STATUS_SUCCESS;

	UCHAR hookBytes[8] = { 0x83, 0xbf, 0xec, 0x00, 0x00, 0x00, 0x00, 0x74 };

	ULONG funAddr = KF_GetDbgkExitThreadAddr();
	if (0 == funAddr)
	{
		alert("UnHook_DbgkExitThread: KF_GetDbgkExitThreadAddr=0");

		return STATUS_UNSUCCESSFUL;
	}

	ULONG hookAddr = funAddr + 0x28;

	Hook8b(hookAddr, hookBytes, NULL);

	operate_prompt("UnHook_DbgkExitThread");

	return status;
}

/************************************************************************/
/*
	hook��unhook�ں˺���DbgkCreateThread�������Խ��̵��ã���call��ʽ��
*/
/************************************************************************/
VOID __declspec(naked) Dispatch_DbgkCreateThread()
{
	__asm {
		call	s_GetDbgObjAddr
		cmp		eax, 0

		je		retFun

#ifdef _OPERATE_PROMPT
		push	eax
		push	2
		call	Print_HookLog
#endif
		
		ret

retFun:
		//Ϊ���ȶ����ϵ�
		mov     esi, dword ptr[ebp - 2Ch]

		//hook������
		cmp     dword ptr[esi + 0ECh], ebx

		ret
	}
}

NTSTATUS Hook_DbgkCreateThread()
{
	NTSTATUS status = STATUS_SUCCESS;

	UCHAR hookBytes[8] = { 0xe8, 0x00, 0x00, 0x00, 0x00, 0x90, 0x0f, 0x84 };

	ULONG funAddr = KF_GetDbgkCreateThreadAddr();
	if (0 == funAddr)
	{
		alert("Hook_DbgkCreateThread: KF_GetDbgkCreateThreadAddr=0");

		return STATUS_UNSUCCESSFUL;
	}

	ULONG hookAddr = funAddr + 0x22a;

	*(PULONG)(hookBytes + 1) = (ULONG)Dispatch_DbgkCreateThread - hookAddr - 5;

	Hook8b(hookAddr, hookBytes, NULL);

	operate_prompt("Hook_DbgkCreateThread");

	return status;
}

NTSTATUS UnHook_DbgkCreateThread()
{
	NTSTATUS status = STATUS_SUCCESS;

	UCHAR hookBytes[8] = { 0x39, 0x9e, 0xec, 0x00, 0x00, 0x00, 0x0f, 0x84 };

	ULONG funAddr = KF_GetDbgkCreateThreadAddr();
	if (0 == funAddr)
	{
		alert("UnHook_DbgkCreateThread: KF_GetDbgkCreateThreadAddr=0");

		return STATUS_UNSUCCESSFUL;
	}

	ULONG hookAddr = funAddr + 0x22a;

	Hook8b(hookAddr, hookBytes, NULL);

	operate_prompt("UnHook_DbgkCreateThread");

	return status;
}

/************************************************************************/
/*
	hook��unhook�ں˺���DbgkExitProcess�������Խ��̵��ã���call��ʽ��

	hook����;���ı����debugport���ж�
*/
/************************************************************************/
VOID __declspec(naked) Dispatch_DbgkExitProcess()
{
	__asm {
		push	eax
		push	ecx

		call	s_GetDbgObjAddr
		cmp		eax, 0

		je		retFun

#ifdef _OPERATE_PROMPT
		push	eax
		push	4
		call	Print_HookLog
#endif

		//�����ж�eax��ecx�Ĳ�����֮ǰ�ĺ��������޸���eax��ecx��ֵ��������Ҫ���»��
		pop		ecx
		pop		eax
		
		ret

	retFun:
		pop		ecx
		pop		eax

		//Ϊ���ȶ����ϵ�
		mov     ecx, dword ptr fs : [124h]
		mov     ecx, dword ptr[ecx + 50h]

		//hook������
		cmp     dword ptr[ecx + 0ECh], 0

		ret
	}
}

NTSTATUS Hook_DbgkExitProcess()
{
	NTSTATUS status = STATUS_SUCCESS;

	UCHAR hookBytes[8] = { 0xe8, 0x00, 0x00, 0x00, 0x00, 0x90, 0x90, 0x74 };

	ULONG funAddr = KF_GetDbgkExitProcessAddr();
	if (0 == funAddr)
	{
		alert("Hook_DbgkExitProcess: KF_GetDbgkExitProcessAddr=0");

		return STATUS_UNSUCCESSFUL;
	}

	ULONG hookAddr = funAddr + 0x28;

	*(PULONG)(hookBytes + 1) = (ULONG)Dispatch_DbgkExitProcess - hookAddr - 5;

	Hook8b(hookAddr, hookBytes, NULL);

	operate_prompt("Hook_DbgkExitProcess");

	return status;
}

NTSTATUS UnHook_DbgkExitProcess()
{
	NTSTATUS status = STATUS_SUCCESS;

	UCHAR hookBytes[8] = { 0x83, 0xb9, 0xec, 0x00, 0x00, 0x00, 0x00, 0x74 };

	ULONG funAddr = KF_GetDbgkExitProcessAddr();
	if (0 == funAddr)
	{
		alert("UnHook_DbgkExitProcess: KF_GetDbgkExitProcessAddr=0");

		return STATUS_UNSUCCESSFUL;
	}

	ULONG hookAddr = funAddr + 0x28;

	Hook8b(hookAddr, hookBytes, NULL);

	operate_prompt("UnHook_DbgkExitProcess");

	return status;
}

/************************************************************************/
/*
	hook��unhook �ں˺���NtQueryObject

	�����DNF����Ľ��̵���
		��ѯ��Ϊ3������STATUS_ACCESS_DENIED
		���в�ѯ����STATUS_ACCESS_DENIED
*/
/************************************************************************/
static ULONG NtQueryObjectRetAddr;

VOID Run_NtQueryObject(IN ULONG ObjectInformationClass)
{
	if (3 == ObjectInformationClass)
	{
		DbgPrint("NtQueryObject: %s#%d \n", GetCurrentImageFileName(), ObjectInformationClass);
	}
	else
	{
		DbgPrint("NtQueryObject: %s->%d \n", GetCurrentImageFileName(), ObjectInformationClass);
	}
}

VOID __declspec(naked) Dispatch_NtQueryObject()
{
	if (IsDNFClient() || IsTASLogin() || IsDNF())
	{
		__asm {
			//���˵��ú�
			mov		eax, [esp + 0x08]
			cmp		eax, 3
			jne		RetFun

			push	[esp + 0x08]
			call	Run_NtQueryObject
			
			mov		eax, 0xC0000022		//STATUS_ACCESS_DENIED
			retn    14h
		}
	}

	__asm {
	RetFun:
		//hook������
		push    84h

		jmp		NtQueryObjectRetAddr
	}
}

NTSTATUS Hook_NtQueryObject()
{
	NTSTATUS status = STATUS_SUCCESS;

	UCHAR hookBytes[8] = { 0xe9, 0x00, 0x00, 0x00, 0x00, 0x68, 0xc0, 0x19 };

	ULONG funAddr = KF_GetNtQueryObjectAddr();
	if (0 == funAddr)
	{
		alert("Hook_NtQueryObject: KF_GetNtQueryObjectAddr=0");

		return STATUS_UNSUCCESSFUL;
	}

	ULONG hookAddr = funAddr;

	NtQueryObjectRetAddr = hookAddr + 5;

	*(PULONG)(hookBytes + 1) = (ULONG)Dispatch_NtQueryObject - hookAddr - 5;

	Hook8b(hookAddr, hookBytes, NULL);

	operate_prompt("Hook_NtQueryObject");

	return status;
}

NTSTATUS UnHook_NtQueryObject()
{
	NTSTATUS status = STATUS_SUCCESS;

	UCHAR hookBytes[8] = { 0x68, 0x84, 0x00, 0x00, 0x00, 0x68, 0xc0, 0x19 };

	ULONG funAddr = KF_GetNtQueryObjectAddr();
	if (0 == funAddr)
	{
		alert("UnHook_NtQueryObject: KF_GetNtQueryObjectAddr=0");

		return STATUS_UNSUCCESSFUL;
	}

	ULONG hookAddr = funAddr;

	NtQueryObjectRetAddr = 0;

	Hook8b(hookAddr, hookBytes, NULL);

	operate_prompt("UnHook_NtQueryObject");

	return status;
}

/************************************************************************/
/*
	hook��unhook �ں˺���NtCreateDebugObject
*/
/************************************************************************/
VOID Run_NtCreateDebugObject(IN ULONG debugObjectAddr)
{
	MyDebugPortDLinkListAdd(g_pDebugPortDLinkList, 1, debugObjectAddr, (ULONG)PsGetCurrentProcess());

	DbgPrint("NtCreateDebugObject: DebugObject:%X, DebuggerProcess: %X \n", debugObjectAddr, PsGetCurrentProcess());
}

VOID __declspec(naked) Dispatch_NtCreateDebugObject()
{
	__asm {
		push	edx
	}

	if (IsCE() || IsOD() || IsSelfDebug())
	{
		__asm {
			push	[ebp - 20h]
			call	Run_NtCreateDebugObject
		}
	}

	__asm {
		pop		edx

		//hook������
		xor     esi, esi
		inc     esi
		mov     dword ptr[edx + 10h], esi

		ret
	}
}

NTSTATUS Hook_NtCreateDebugObject()
{
	NTSTATUS status = STATUS_SUCCESS;

	UCHAR hookBytes[8] = { 0xe8, 0x00, 0x00, 0x00, 0x00, 0x90, 0x89, 0x5a };

	ULONG funAddr = KF_GetNtCreateDebugObjectAddr();
	if (0 == funAddr)
	{
		alert("Hook_NtCreateDebugObject: KF_GetNtCreateDebugObjectAddr=0");

		return STATUS_UNSUCCESSFUL;
	}

	ULONG hookAddr = funAddr + 0x7a;

	*(PULONG)(hookBytes + 1) = (ULONG)Dispatch_NtCreateDebugObject - hookAddr - 5;

	Hook8b(hookAddr, hookBytes, NULL);

	operate_prompt("Hook_NtCreateDebugObject");

	return status;
}

NTSTATUS UnHook_NtCreateDebugObject()
{
	NTSTATUS status = STATUS_SUCCESS;

	UCHAR hookBytes[8] = { 0x33, 0xf6, 0x46, 0x89, 0x72, 0x10, 0x89, 0x5a };

	ULONG funAddr = KF_GetNtCreateDebugObjectAddr();
	if (0 == funAddr)
	{
		alert("UnHook_NtCreateDebugObject: KF_GetNtCreateDebugObjectAddr=0");

		return STATUS_UNSUCCESSFUL;
	}

	ULONG hookAddr = funAddr + 0x7a;

	Hook8b(hookAddr, hookBytes, NULL);

	operate_prompt("UnHook_NtCreateDebugObject");

	return status;
}

/************************************************************************/
/*
	hook��unhook �ں˺���NtDebugContinue
*/
/************************************************************************/
static ULONG NtDebugContinueRetAddr;

VOID __declspec(naked) Dispatch_NtDebugContinue()
{
	if (IsCE() || IsOD() || IsSelfDebug())
	{
		__asm {
			call	s_GetDbgObjAddrByDebuggerProcess

			cmp		eax, 0
			je		RetFun

			mov		[ebp - 20h], eax

			//���Ӷ�������ü���������ɾ��ʱ������
			mov		ecx, eax
			call	ObfReferenceObject

			//���óɹ������ؽ��eax��Ϊ0��ObReferenceObjectByHandleִ�к�Ľ����eax�У�
			xor		eax, eax
			
			//�������ݵ��Զ�������ȡ���Զ���Ĵ���
			mov		ecx, NtDebugContinueRetAddr
			add		ecx, 0x13
			jmp		ecx
		}
	}

	__asm {
	RetFun:
		push    edi
		lea     eax, [ebp - 20h]
		push    eax

		jmp		NtDebugContinueRetAddr
	}
}

NTSTATUS Hook_NtDebugContinue()
{
	NTSTATUS status = STATUS_SUCCESS;

	UCHAR hookBytes[8] = { 0xe9, 0x00, 0x00, 0x00, 0x00, 0xff, 0x75, 0xdc };

	ULONG funAddr = KF_GetNtDebugContinueAddr();
	if (0 == funAddr)
	{
		alert("Hook_NtDebugContinue: KF_GetNtDebugContinueAddr=0");

		return STATUS_UNSUCCESSFUL;
	}

	ULONG hookAddr = funAddr + 0x78;

	NtDebugContinueRetAddr = hookAddr + 5;

	*(PULONG)(hookBytes + 1) = (ULONG)Dispatch_NtDebugContinue - hookAddr - 5;

	Hook8b(hookAddr, hookBytes, NULL);

	operate_prompt("Hook_NtDebugContinue");

	return status;
}

NTSTATUS UnHook_NtDebugContinue()
{
	NTSTATUS status = STATUS_SUCCESS;

	UCHAR hookBytes[8] = { 0x57, 0x8d, 0x45, 0xe0, 0x50, 0xff, 0x75, 0xdc };

	ULONG funAddr = KF_GetNtDebugContinueAddr();
	if (0 == funAddr)
	{
		alert("UnHook_NtDebugContinue: KF_GetNtDebugContinueAddr=0");

		return STATUS_UNSUCCESSFUL;
	}

	ULONG hookAddr = funAddr + 0x78;

	NtDebugContinueRetAddr = 0;

	Hook8b(hookAddr, hookBytes, NULL);

	operate_prompt("UnHook_NtDebugContinue");

	return status;
}

/************************************************************************/
/* 
	hook\unhook�ں˺���DbgkMapViewOfSection
*/
/************************************************************************/
VOID __declspec(naked) Dispatch_DbgkMapViewOfSection()
{
	__asm {
		push	ecx
		push	ebx
	}

	if (IsCE() || IsOD() || IsSelfDebug())
	{
		__asm {
			call	s_GetDbgObjAddr

			pop		ebx
			pop		ecx

			cmp		eax, 0
			ret
		}
	}

	__asm {
		pop		ebx
		pop		ecx

		cmp		[ecx + 0ECh], ebx

		ret
	}
}

NTSTATUS Hook_DbgkMapViewOfSection()
{
	NTSTATUS status = STATUS_SUCCESS;

	UCHAR hookBytes[8] = { 0xe8, 0x00, 0x00, 0x00, 0x00, 0x90, 0x0f, 0x84 };

	ULONG funAddr = KF_GetDbgkMapViewOfSectionAddr();
	if (0 == funAddr)
	{
		alert("Hook_DbgkMapViewOfSection: KF_GetDbgkMapViewOfSectionAddr=0");

		return STATUS_UNSUCCESSFUL;
	}

	ULONG hookAddr = funAddr + 0x3b;

	*(PULONG)(hookBytes + 1) = (ULONG)Dispatch_DbgkMapViewOfSection - hookAddr - 5;

	Hook8b(hookAddr, hookBytes, NULL);

	operate_prompt("Hook_DbgkMapViewOfSection");

	return status;
}

NTSTATUS UnHook_DbgkMapViewOfSection()
{
	NTSTATUS status = STATUS_SUCCESS;

	UCHAR hookBytes[8] = { 0x39, 0x99, 0xec, 0x00, 0x00, 0x00, 0x0f, 0x84 };

	ULONG funAddr = KF_GetDbgkMapViewOfSectionAddr();
	if (0 == funAddr)
	{
		alert("UnHook_DbgkMapViewOfSection: KF_GetDbgkMapViewOfSectionAddr=0");

		return STATUS_UNSUCCESSFUL;
	}

	ULONG hookAddr = funAddr + 0x3b;

	Hook8b(hookAddr, hookBytes, NULL);

	operate_prompt("UnHook_DbgkMapViewOfSection");

	return status;
}

/************************************************************************/
/*
	hook��unhook�ں˺���DbgkUnMapViewOfSection
*/
/************************************************************************/
VOID __declspec(naked) Dispatch_DbgkUnMapViewOfSection()
{
	__asm {
		push	eax
	}

	if (IsCE() || IsOD() || IsSelfDebug())
	{
		DbgPrint("Dispatch_DbgkUnMapViewOfSection");

		__asm {
			call	s_GetDbgObjAddr

			cmp		eax, 0

			pop		eax

			ret
		}
	}

	__asm {
		pop		eax

	retFun:
		cmp     dword ptr[eax + 0ECh], 0

		ret
	}
}

NTSTATUS Hook_DbgkUnMapViewOfSection()
{
	NTSTATUS status = STATUS_SUCCESS;

	UCHAR hookBytes[8] = { 0xe8, 0x00, 0x00, 0x00, 0x00, 0x90, 0x90, 0x74 };

	ULONG funAddr = KF_GetDbgkUnMapViewOfSectionAddr();
	if (0 == funAddr)
	{
		alert("Hook_DbgkUnMapViewOfSection: KF_GetDbgkMapViewOfSectionAddr=0");

		return STATUS_UNSUCCESSFUL;
	}

	ULONG hookAddr = funAddr + 0x33;

	*(PULONG)(hookBytes + 1) = (ULONG)Dispatch_DbgkUnMapViewOfSection - hookAddr - 5;

	Hook8b(hookAddr, hookBytes, NULL);

	operate_prompt("Hook_DbgkUnMapViewOfSection");

	return status;
}

NTSTATUS UnHook_DbgkUnMapViewOfSection()
{
	NTSTATUS status = STATUS_SUCCESS;

	UCHAR hookBytes[8] = { 0x83, 0xB8, 0xEC, 0x00, 0x00, 0x00, 0x00, 0x74 };

	ULONG funAddr = KF_GetDbgkUnMapViewOfSectionAddr();
	if (0 == funAddr)
	{
		alert("UnHook_DbgkUnMapViewOfSection: KF_GetDbgkUnMapViewOfSectionAddr=0");

		return STATUS_UNSUCCESSFUL;
	}

	ULONG hookAddr = funAddr + 0x33;

	Hook8b(hookAddr, hookBytes, NULL);

	operate_prompt("UnHook_DbgkUnMapViewOfSection");

	return status;
}

/************************************************************************/
/* 
	hook��unhook �ں˺���DbgkpMarkProcessPeb������ʱ�����޸�PEB.BeingDebugged��
*/
/************************************************************************/
NTSTATUS Hook_DbgkpMarkProcessPeb()
{
	NTSTATUS status = STATUS_SUCCESS;

	UCHAR hookBytes[8] = { 0xc2, 0x04, 0x00, 0x90, 0x90, 0x90, 0x90, 0x90 };

	ULONG funAddr = KF_GetDbgkpMarkProcessPebAddr();
	if (0 == funAddr)
	{
		alert("Hook_DbgkpMarkProcessPeb: KF_GetNtDebugContinueAddr=0");

		return STATUS_UNSUCCESSFUL;
	}

	ULONG hookAddr = funAddr;

	Hook8b(hookAddr, hookBytes, NULL);

	operate_prompt("Hook_DbgkpMarkProcessPeb");

	return status;
}

NTSTATUS UnHook_DbgkpMarkProcessPeb()
{
	NTSTATUS status = STATUS_SUCCESS;

	UCHAR hookBytes[8] = { 0x6a, 0x24, 0x68, 0x40, 0xd3, 0xe5, 0x83, 0xe8 };

	ULONG funAddr = KF_GetDbgkpMarkProcessPebAddr();
	if (0 == funAddr)
	{
		alert("UnHook_DbgkpMarkProcessPeb: KF_GetNtDebugContinueAddr=0");

		return STATUS_UNSUCCESSFUL;
	}

	ULONG hookAddr = funAddr;

	Hook8b(hookAddr, hookBytes, NULL);

	operate_prompt("UnHook_DbgkpMarkProcessPeb");

	return status;
}

/************************************************************************/
/*
	hook��unhook�ں˺���NtQueryInformationProcess���޸�ĳЩ����µķ���ֵ��

	ProcessDebugPort			0x07	//07 DebugPort��0������������0
	ProcessDebugObjectHandle	0x1e	//30 DebugPort��0�������������ʧ��
	ProcessDebugFlags			0x1f	//31 1�ǵ���״̬ 0����״̬
*/
/************************************************************************/
static ULONG NtQueryInformationProcessRetAddr;

VOID Run_NtQueryInformationProcess(IN ULONG ProcessInformationClass)
{
	if ((0x07 == ProcessInformationClass) || (0x1e == ProcessInformationClass) || (0x1f == ProcessInformationClass)) {
		DbgPrint("%s: ProcessInformationClass=0x%X( %d ) \n", GetCurrentImageFileName(), ProcessInformationClass, ProcessInformationClass);
	}
}

/************************************************************************/
/*
	TP��DNF��س�����ô˺�����ȡ������ص���Ϣȫ������STATUS_INFO_LENGTH_MISMATCH
	���Ҫ����ֵ�����̲߳���ȫ�ԣ���������
*/
/************************************************************************/
VOID __declspec(naked) Dispatch_NtQueryInformationProcess()
{
	/*
		ע������ֻ������һ�����̽��룬��Ϊ����ĳ�����޸�0�������ݣ�����ʱ�л��˽��̾��л���cr3���޸�����ʱ������
	*/
	if (IsDNFClient() || IsTASLogin() || IsDNF())
	{
		__asm {
			push	[esp + 8]
			call	Run_NtQueryInformationProcess

			//ProcessInformationClass = 0x07
			mov		ecx, [esp + 8]
			cmp		ecx, 0x07
			jne		Check1
			jmp		ReturnError
			//mov		ecx, [esp + 0x0c]
			//mov		[ecx], 0
			//xor		eax, eax
			//jmp		EndCheck

			//ProcessInformationClass = 0x1e
		Check1:
			mov		ecx, [esp + 8]
			cmp		ecx, 0x1e
			jne		Check2
			jmp		ReturnError

			//ProcessInformationClass = 0x1f
		Check2:
			mov		ecx, [esp + 8]
			cmp		ecx, 0x1f
			jne		RetFun
			//mov		ecx, [esp + 0x0c]
			//mov		[ecx], 1
			//xor		eax, eax
			//jmp		EndCheck

		ReturnError:
			mov		eax, 0xC0000004L	//STATUS_INFO_LENGTH_MISMATCH

		EndCheck:
			retn    14h
		}
	}

	__asm {
	RetFun:
		push    2CCh

		jmp		NtQueryInformationProcessRetAddr
	}
}

NTSTATUS Hook_NtQueryInformationProcess()
{
	NTSTATUS status = STATUS_SUCCESS;

	UCHAR hookBytes[8] = { 0xe9, 0x00, 0x00, 0x00, 0x00, 0x68, 0xb8, 0x95 };

	ULONG funAddr = KF_GetNtQueryInformationProcessAddr();
	if (0 == funAddr)
	{
		alert("Hook_NtQueryInformationProcess: KF_GetNtQueryInformationProcessAddr=0");

		return STATUS_UNSUCCESSFUL;
	}

	ULONG hookAddr = funAddr;

	NtQueryInformationProcessRetAddr = hookAddr + 5;

	*(PULONG)(hookBytes + 1) = (ULONG)Dispatch_NtQueryInformationProcess - hookAddr - 5;

	Hook8b(hookAddr, hookBytes, NULL);

	operate_prompt("Hook_NtQueryInformationProcess");

	return status;
}

NTSTATUS UnHook_NtQueryInformationProcess()
{
	NTSTATUS status = STATUS_SUCCESS;

	UCHAR hookBytes[8] = { 0x68, 0xcc, 0x02, 0x00, 0x00, 0x68, 0xb8, 0x95 };

	ULONG funAddr = KF_GetNtQueryInformationProcessAddr();
	if (0 == funAddr)
	{
		alert("UnHook_NtQueryInformationProcess: KF_GetNtQueryInformationProcessAddr=0");

		return STATUS_UNSUCCESSFUL;
	}

	ULONG hookAddr = funAddr;

	NtQueryInformationProcessRetAddr = 0;

	Hook8b(hookAddr, hookBytes, NULL);

	operate_prompt("UnHook_NtQueryInformationProcess");

	return status;
}

/************************************************************************/
/*
	�ָ�tp�ҹ����ں˺���NtQueryInformationThread

	ThreadHideFromDebugger
*/
/************************************************************************/
static ULONG NtQueryInformationThreadRetAddr = 0;
static UCHAR s_NtQueryInformationThreadOldBytes[8] = { 0 };

VOID Run_NtQueryInformationThread(IN ULONG ThreadInformationClass)
{
	if (0x11 == ThreadInformationClass) {
		DbgPrint("%s: ThreadInformationClass=0x%X( %d ) \n", GetCurrentImageFileName(), ThreadInformationClass, ThreadInformationClass);
	}
}

VOID __declspec(naked) Dispatch_NtQueryInformationThread()
{
	if (IsDNFClient() || IsTASLogin() || IsDNF())
	{
		__asm {
			push	[esp + 8]
			call	Run_NtQueryInformationThread

			mov		ecx, [esp + 8]
			cmp		ecx, 0x11
			jne		RetFun
		
			mov		eax, 0xC0000004L	//STATUS_INFO_LENGTH_MISMATCH
			retn    14h
		}
	}

	__asm {
	RetFun:
		push    110h

		jmp		NtQueryInformationThreadRetAddr
	}
}

NTSTATUS Hook_NtQueryInformationThread()
{
	NTSTATUS status = STATUS_SUCCESS;

	UCHAR hookBytes[8] = { 0xe9, 0x00, 0x00, 0x00, 0x00, 0xcc, 0xcc, 0xcc };

	ULONG funAddr = KF_GetNtQueryInformationThreadAddr();
	if (0 == funAddr)
	{
		alert("Hook_NtQueryInformationThread: KF_GetNtQueryInformationThreadAddr=0");

		return STATUS_UNSUCCESSFUL;
	}

	ULONG hookAddr = funAddr;

	NtQueryInformationThreadRetAddr = hookAddr + 5;

	*(PULONG)(hookBytes + 1) = (ULONG)Dispatch_NtQueryInformationThread - hookAddr - 5;
	
	//Ϊ���ȶ���hookBytes���3�ֽڴ�hookAddr+5����ȡ
	RtlCopyMemory((PVOID)(hookBytes + 5), (PVOID)(hookAddr + 5), 3);

	Hook8b(hookAddr, hookBytes, s_NtQueryInformationThreadOldBytes);

	operate_prompt("Hook_NtQueryInformationThread");

	return status;
}

NTSTATUS UnHook_NtQueryInformationThread()
{
	NTSTATUS status = STATUS_SUCCESS;

	ULONG funAddr = KF_GetNtQueryInformationThreadAddr();
	if (0 == funAddr)
	{
		alert("UnHook_NtQueryInformationThread: KF_GetNtQueryInformationThreadAddr=0");

		return STATUS_UNSUCCESSFUL;
	}

	ULONG hookAddr = funAddr;

	NtQueryInformationThreadRetAddr = 0;

	Hook8b(hookAddr, s_NtQueryInformationThreadOldBytes, NULL);

	operate_prompt("UnHook_NtQueryInformationThread");

	return status;
}

/************************************************************************/
/*
	�ָ�tp�ҹ����ں˺���
*/
/************************************************************************/
NTSTATUS Recover_NtAllocateVirtualMemory()
{
	NTSTATUS status = STATUS_SUCCESS;

	UCHAR hookBytes[8] = { 0x68, 0xc0, 0x00, 0x00, 0x00, 0x68, 0x00, 0x25 };

	ULONG funAddr = KF_GetNtAllocateVirtualMemoryAddr();
	if (0 == funAddr)
	{
		alert("Recover_NtAllocateVirtualMemory: KF_GetNtAllocateVirtualMemoryAddr=0");

		return STATUS_UNSUCCESSFUL;
	}

	ULONG hookAddr = funAddr;

	Hook8b(hookAddr, hookBytes, NULL);

	operate_prompt("Recover_NtAllocateVirtualMemory");

	return status;
}

NTSTATUS Recover_KiAttachProcess()
{
	NTSTATUS status = STATUS_SUCCESS;

	UCHAR hookBytes[8] = { 0x8b, 0xff, 0x55, 0x8b, 0xec, 0x53, 0x8b, 0x5d };

	ULONG funAddr = KF_GetKiAttachProcessAddr();
	if (0 == funAddr)
	{
		alert("Recover_KiAttachProcess: KF_GetKiAttachProcessAddr=0");

		return STATUS_UNSUCCESSFUL;
	}

	ULONG hookAddr = funAddr;

	Hook8b(hookAddr, hookBytes, NULL);

	operate_prompt("Recover_KiAttachProcess");

	return status;
}

NTSTATUS Recover_KeResumeThread()
{
	NTSTATUS status = STATUS_SUCCESS;

	UCHAR hookBytes[8] = { 0x8b, 0xff, 0x55, 0x8b, 0xec, 0x83, 0xec, 0x0c };

	ULONG funAddr = KF_GetKeResumeThreadAddr();
	if (0 == funAddr)
	{
		alert("Recover_KeResumeThread: KF_GetKiAttachProcessAddr=0");

		return STATUS_UNSUCCESSFUL;
	}

	ULONG hookAddr = funAddr;

	Hook8b(hookAddr, hookBytes, NULL);

	operate_prompt("Recover_KeResumeThread");

	return status;
}

/*
	�˺���ͨ������ָ�����������ʱʹ��PChunter�ָ����������о����ͨ������ָ�
*/
NTSTATUS Recover_SeDefaultObjectMethod()
{
	NTSTATUS status = STATUS_SUCCESS;

	UCHAR hookBytes[8] = { 0x8b, 0xff, 0x55, 0x8b, 0xec, 0x8b, 0x45, 0x0c };

	ULONG funAddr = KF_GetKeResumeThreadAddr();
	if (0 == funAddr)
	{
		alert("Recover_SeDefaultObjectMethod: KF_GetKiAttachProcessAddr=0");

		return STATUS_UNSUCCESSFUL;
	}

	ULONG hookAddr = funAddr;

//	Hook8b(hookAddr, hookBytes, NULL);

	operate_prompt("Recover_SeDefaultObjectMethod");

	return status;
}

NTSTATUS Recover_KiSwapThread()
{
	NTSTATUS status = STATUS_SUCCESS;

	UCHAR hookBytes[8] = { 0x8b, 0xff, 0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf8 };

	ULONG funAddr = KF_GetKiSwapThreadAddr();
	if (0 == funAddr)
	{
		alert("Recover_KiSwapThread: KF_GetKiAttachProcessAddr=0");

		return STATUS_UNSUCCESSFUL;
	}

	ULONG hookAddr = funAddr;

	Hook8b(hookAddr, hookBytes, NULL);

	operate_prompt("Recover_KiSwapThread");

	return status;
}

NTSTATUS Recover_ZwFsControlFile()
{
	NTSTATUS status = STATUS_SUCCESS;

	UCHAR hookBytes[8] = { 0xb8, 0x86, 0x00, 0x00, 0x00, 0x8d, 0x54, 0x24 };

	ULONG funAddr = KF_GetZwFsControlFileAddr();
	if (0 == funAddr)
	{
		alert("Recover_ZwFsControlFile: KF_GetKiAttachProcessAddr=0");

		return STATUS_UNSUCCESSFUL;
	}

	ULONG hookAddr = funAddr;

	Hook8b(hookAddr, hookBytes, NULL);

	operate_prompt("Recover_ZwFsControlFile");

	return status;
}

NTSTATUS Recover_PsResumeProcess()
{
	NTSTATUS status = STATUS_SUCCESS;

	UCHAR hookBytes[8] = { 0x8b, 0xff, 0x55, 0x8b, 0xec, 0x53, 0x56, 0x64 };

	ULONG funAddr = KF_GetPsResumeProcessAddr();
	if (0 == funAddr)
	{
		alert("Recover_PsResumeProcess: KF_GetKiAttachProcessAddr=0");

		return STATUS_UNSUCCESSFUL;
	}

	ULONG hookAddr = funAddr;

	Hook8b(hookAddr, hookBytes, NULL);

	operate_prompt("Recover_PsResumeProcess");

	return status;
}