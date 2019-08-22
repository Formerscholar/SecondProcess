#ifndef __PAEADDR_H
#define __PAEADDR_H

#include <ntdef.h>

#define PDE_BASE 0xC0600000
#define PTE_BASE 0xC0000000

#define ADDR_IS_VALID	0x00	//���Ե�ַ��Ч
#define NO_PDE			0x01	//���Ե�ַ��PDE
#define PDE_NO_VALID	0x02	//PDE��Ч��PλΪ0��
#define NO_PTE			0x03	//���Ե�ַ��PTE
#define PTE_NO_VALID	0x04	//���Ե�ַ��Ч��PλΪ0��

#define ADDR_IS_WRITABLE	0x07	//���Ե�ַ��д
#define PDE_NO_WRITABLE		0x05	//pde����д
#define PTE_NO_WRITABLE		0x06	//pte����д

#define PDE_VALID	0x08
#define PTE_VALID	0x09

#define PDE_WRITABLE	0x10
#define PTE_WRITABLE	0x11

#ifdef __cplusplus
extern "C" {
#endif

	ULONG GetPDE(IN ULONG vAddr);
	VOID SetPDE(IN ULONG vAddr, IN ULONG pde);
	VOID ChangePDEWrite(IN ULONG vAddr);
	VOID ChangePDENoWrite(IN ULONG vAddr);
	ULONG IsPDEValid(IN ULONG addr);
	ULONG IsPDEWritable(IN ULONG addr);

	ULONG GetPTE(IN ULONG vAddr);
	VOID SetPTE(IN ULONG vAddr, IN ULONG pte);
	VOID ChangePTEWrite(IN ULONG vAddr);
	VOID ChangePTENoWrite(IN ULONG vAddr);
	ULONG IsPTEValid(IN ULONG addr);
	ULONG IsPTEWritable(IN ULONG addr);

	ULONG IsAddrValid(IN ULONG vAddr);
	ULONG IsAddrWrite(IN ULONG vAddr);

#ifdef __cplusplus
}
#endif

#endif