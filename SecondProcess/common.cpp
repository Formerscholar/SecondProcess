#include "common.h"

/*
	�ر�ҳ����
*/
void PageProtectOff()
{
	__asm {
		cli
		mov		eax, cr0
		and		eax, not 0x10000
		mov		cr0, eax
	}
}

/*
	�ָ��ڴ汣��
*/
void PageProtectOn()
{
	__asm {
		mov		eax, cr0
		or		eax, 0x10000
		mov		cr0, eax
		sti
	}
}

/*
	һ�����޸�8�ֽ��ڴ�
*/
VOID Hook8b(IN ULONG hookAddr, IN PUCHAR hookBytesArr, OUT PUCHAR oldCode)
{
	ULONG lowData = *(PULONG)&hookBytesArr[0];
	ULONG highData = *(PULONG)&hookBytesArr[4];

	if(NULL != oldCode) {
		RtlMoveMemory(oldCode, (PVOID)hookAddr, 8);
	}

	PageProtectOff();

	__asm {
		mov		ebx, [lowData]
		mov		ecx, [highData]

		mov		edi, [hookAddr]

		cmpxchg8b qword ptr ds:[edi]
		lock cmpxchg8b qword ptr ds:[edi]
	}

	PageProtectOn();
}
