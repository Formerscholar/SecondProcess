#include "PAEAddr.h"

static ULONG GetPTPDEIndex(ULONG vAddr)
{
	ULONG index = 0;

	index = vAddr & 0xC0000000;
	index = vAddr >> 0x1E;	//����30λ

	return index;
}

static ULONG GetPDI(ULONG vAddr)
{
	ULONG pdi = 0;

	pdi = vAddr & 0x3FE00000;
	pdi = pdi >> 0x15;	//����21λ

	return pdi;
}

/*
	���PDE��ֵ

	PDE = *(PDE_BASE + ptpdeIndex * 1000 + pdi * 8)
*/
ULONG GetPDE(IN ULONG vAddr)
{
	ULONG pde = 0;
	ULONG ptpdeIndex = 0;
	ULONG pdi = 0;

	ptpdeIndex = GetPTPDEIndex(vAddr);
	pdi = GetPDI(vAddr);

	pde = *(PULONG)(PDE_BASE + ptpdeIndex * 0x1000 + pdi * 8);

	return pde;
}

VOID SetPDE(IN ULONG vAddr, IN ULONG pde)
{
	ULONG ptpdeIndex = 0;
	ULONG pdi = 0;

	ptpdeIndex = GetPTPDEIndex(vAddr);
	pdi = GetPDI(vAddr);

	*(PULONG)(PDE_BASE + ptpdeIndex * 0x1000 + pdi * 8) = pde;
}

VOID ChangePDEWrite(IN ULONG vAddr)
{
	ULONG ptpdeIndex = 0;
	ULONG pdi = 0;
	ULONG pde = 0;

	pde = GetPDE(vAddr);

	if (0 != (pde & 0x00000002)) {
		return;
	}

	pde |= 0x00000002;

	ptpdeIndex = GetPTPDEIndex(vAddr);
	pdi = GetPDI(vAddr);

	*(PULONG)(PDE_BASE + ptpdeIndex * 0x1000 + pdi * 8) = pde;
}

VOID ChangePDENoWrite(IN ULONG vAddr)
{
	ULONG ptpdeIndex = 0;
	ULONG pdi = 0;
	ULONG pde = 0;

	pde = GetPDE(vAddr);

	if (0 == (pde & 0x00000002)) {
		return;
	}

	pde &= 0xfffffffd;

	ptpdeIndex = GetPTPDEIndex(vAddr);
	pdi = GetPDI(vAddr);

	*(PULONG)(PDE_BASE + ptpdeIndex * 0x1000 + pdi * 8) = pde;
}

ULONG IsPDEValid(IN ULONG addr)
{
	ULONG pde = 0;

	pde = GetPDE(addr);
	if (0 == pde) {
		return NO_PDE;
	}

	if (0 == (pde & 0x00000001)) {
		return PDE_NO_VALID;
	}

	return PDE_VALID;
}

ULONG IsPDEWritable(IN ULONG addr)
{
	ULONG pde = 0;
	
	pde = GetPDE(addr);
	if (0 == pde) {
		return NO_PDE;
	}

	if (0 == (pde & 0x00000002)) {
		return PDE_NO_WRITABLE;
	}

	return PDE_WRITABLE;
}

/************************************************************************/
/*
	PTE��ز���
*/
/************************************************************************/
ULONG GetPTE(IN ULONG vAddr)
{
	ULONG PTE = 0;

	vAddr = vAddr >> 9;

	PTE = *(PULONG)(PTE_BASE + (vAddr & 0x007FFFF8));

	return PTE;
}

VOID SetPTE(IN ULONG vAddr, IN ULONG pte)
{
	vAddr = vAddr >> 9;

	*(PULONG)(PTE_BASE + (vAddr & 0x007FFFF8)) = pte;
}

VOID ChangePTEWrite(IN ULONG vAddr)
{
	ULONG pte = GetPTE(vAddr);

	if (0 != (pte & 0x00000002)) {
		return;
	}

	pte |= 0x00000002;

	vAddr = vAddr >> 9;

	*(PULONG)(PTE_BASE + (vAddr & 0x007FFFF8)) = pte;
}

VOID ChangePTENoWrite(IN ULONG vAddr)
{
	ULONG pte = GetPTE(vAddr);

	if (0 == (pte & 0x00000002)) {
		return;
	}

	pte &= 0xfffffffd;

	vAddr = vAddr >> 9;

	*(PULONG)(PTE_BASE + (vAddr & 0x007FFFF8)) = pte;
}

ULONG IsPTEValid(IN ULONG addr)
{
	return IsAddrValid(addr);
}

ULONG IsPTEWritable(IN ULONG addr)
{
	return IsAddrWrite(addr);
}

/*
	�ж����Ե�ַ�Ƿ���Ч

	1���ж�PDE�Ƿ�Ϊ0
	2������PDE��Pλ
	3���ж�PTE�Ƿ�Ϊ0
	4������PTE��Pλ
	5����Ч
*/
ULONG IsAddrValid(IN ULONG vAddr)
{
	ULONG pde = 0;
	ULONG pte = 0;

	pde = GetPDE(vAddr);
	if(0 == pde) {
		return NO_PDE;
	}

	if(0 == (pde & 0x00000001)) {
		return PDE_NO_VALID;
	}
	
	pte = GetPTE(vAddr);
	if(0 == pte) {
		return NO_PTE;
	}

	if(0 == (pte & 0x00000001)) {
		return PTE_NO_VALID;
	}

	return ADDR_IS_VALID;
}

/*
	�ж����Ե�ַ��Ӧ������ҳ�Ƿ��д

	1���ж�PDE�Ƿ����0
	2���ж�PDE�Ƿ��п�дȨ��
	3���ж�PTE�Ƿ����0
	4���ж�PTE�Ƿ��п�дȨ��
	5����д
*/
ULONG IsAddrWrite(IN ULONG vAddr)
{
	ULONG pde = 0;
	ULONG pte = 0;

	pde = GetPDE(vAddr);
	if(0 == pde) {
		return NO_PDE;
	}

	if(0 == (pde & 0x00000002)) {
		return PDE_NO_WRITABLE;
	}

	pte = GetPTE(vAddr);
	if(0 == pte) {
		return NO_PTE;
	}

	if(0 == (pte & 0x00000002)) {
		return PTE_NO_WRITABLE;
	}

	return ADDR_IS_WRITABLE;
}

