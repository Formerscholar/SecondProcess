#ifndef __KERNEL_H_
#define __KERNEL_H_

#include <ntddk.h>

#include "SSDT.h"

extern PKSYSTEM_SERVICE_TABLE pSystemServiceTable;

/*
	�������ֻ��д�㺯�������������ɿأ����㺯�����Զ����ɵĻ����ܻ��eax��0��

	ִ�к�Ķ�ջͼ��
	esp->	|__���ص�ַ__|
			|___����N____|
			|�����ڶ�����|
			|���һ������|
	ִ��ǰ�Ķ�ջͼ��
	esp->	|__���ص�ַ__|
			|trap_frameͷ��|
*/
VOID __declspec(naked) YKiSystemCall()
{
	__asm {
		mov		ecx, eax					//ecx = eax = 3������ĺ������

		shl		eax, 2						//eax << 2 = eax * 4

		mov		esi, [pSystemServiceTable]	//esi = pSystemServiceTable
		mov		esi, [esi]					//esi = *pSystemServiceTable

		add		eax, esi					//eax = 0�������ĵ�ַ�ĵ�ַ
		mov		eax, [eax]					//eax = 0�������ĵ�ַ

		mov		esi, [pSystemServiceTable]	//esi = pSystemServiceTable
		mov		esi, [esi + 0x0C]			//esi = ��ź��������ı�ĵ�ַ

		add		esi, ecx					//ecx = 0�������Ĳ����ֽ����ĵ�ַ

		xor		ecx, ecx
		mov		cl, [esi]					//ecx = 0�����������ֽ���

		mov		esi, [esp + 0x04 + 0x60]	//ebp = trap_frame��esp + ���ص�ַռ��4�ֽڣ� + 0x60, esi = ebp3
		add		esi, 0x08					//| ebp | ���ص�ַ | ��һ������ | ����
		
		sub		esp, ecx
		mov		edi, esp
		
		shr		ecx, 2						//ecx = ��������
							
		rep		movsd						//������������0����ջ

		call	eax
		
		ret
	}
}

VOID _declspec(naked) YKiSystemService()
{
	__asm {
		push    0
		push    ebp
		push    ebx
		push    esi
		push    edi
		push    fs

		mov     ebx, 30h
		mov     fs, bx
		mov     ebx, 23h
		mov     ds, bx
		mov     es, bx
		
		mov     esi, fs:124h
		
		push    dword ptr fs:[0]
		mov     dword ptr fs:[0], 0FFFFFFFFh
		
		push    dword ptr[esi + 13Ah]
		
		sub     esp, 48h
		
		mov     ebx, [esp + 6ch]
		and     ebx, 1
		mov		[esi + 13Ah], bl
		
		mov     ebp, esp
		
		mov     ebx, [esi + 128h]
		mov		[ebp + 3Ch], ebx
		and     dword ptr[ebp + 2Ch], 0
		mov		[esi + 128h], ebp

		cld
		mov     ebx, [ebp+60h]
		mov     edi, [ebp+68h]
		mov     [ebp+0Ch], edx
		mov     dword ptr [ebp+8], 0BADB0D00h
		mov     [ebp+0], ebx
		mov     [ebp+4], edi
		sti

		call	YKiSystemCall

//�ָ��Ĵ�����Ϊ��3����׼��
recoverRegister:
		add		esp, 0x50

		pop		fs
		pop		edi
		pop		esi
		pop		ebx
		pop		ebp

		add		esp, 0x04	//����ErrCode

//����3��
returnR3:
		iretd
	}
}

#endif
