#ifndef __COMMON_H_
#define __COMMON_H_

#include <ntddk.h>

//�Ƿ���������ʾ
#ifndef _OPERATE_PROMPT
#define _OPERATE_PROMPT
#endif

//�رղ�����ʾ
//#undef _OPERATE_PROMPT

/*
�����ˣ�����ʱ��ʾ��ʾ��Ϣ
δ����������ʱ����ʾ��ʾ��Ϣ
*/
#ifdef _OPERATE_PROMPT
#define operate_prompt(msg) do {	\
	DbgPrint("%s \n", msg);			\
} while (0);

#define operate_prompt_hex(msg, val) do {	\
	DbgPrint("%s: %X \n", msg, val);		\
} while (0);
#else
#define operate_prompt(msg) do {} while (0);

#define operate_prompt_hex(msg, val) do {} while (0);
#endif

#ifndef _DEBUG
#define _DEBUG
#endif

#ifdef _DEBUG
#define alert(msg) do {									\
	DbgPrint("%s(%d): %s \n", __FILE__, __LINE__, msg);	\
} while (0);
#else
#define alert(msg) do {		\
	DbgPrint("%s \n", msg);	\
} while (0);
#endif

/************************************************************************/
/*
	ժ��ReactOS-0.4.6
*/
/************************************************************************/
//
// Process Access Types
//
#define PROCESS_TERMINATE                       0x0001
#define PROCESS_CREATE_THREAD                   0x0002
#define PROCESS_SET_SESSIONID                   0x0004
#define PROCESS_VM_OPERATION                    0x0008
#define PROCESS_VM_READ                         0x0010
#define PROCESS_VM_WRITE                        0x0020
#define PROCESS_CREATE_PROCESS                  0x0080
#define PROCESS_SET_QUOTA                       0x0100
#define PROCESS_SET_INFORMATION                 0x0200
#define PROCESS_QUERY_INFORMATION               0x0400
#define PROCESS_SUSPEND_RESUME                  0x0800
#define PROCESS_QUERY_LIMITED_INFORMATION       0x1000
#if (NTDDI_VERSION >= NTDDI_LONGHORN)
#define PROCESS_ALL_ACCESS                      (STANDARD_RIGHTS_REQUIRED | \
                                                 SYNCHRONIZE | \
                                                 0xFFFF)
#else
#define PROCESS_ALL_ACCESS                      (STANDARD_RIGHTS_REQUIRED | \
                                                 SYNCHRONIZE | \
                                                 0xFFF)
#endif

//
// Debug Object Access Masks
//
#define DEBUG_OBJECT_WAIT_STATE_CHANGE      0x0001
#define DEBUG_OBJECT_ADD_REMOVE_PROCESS     0x0002
#define DEBUG_OBJECT_SET_INFORMATION        0x0004
#define DEBUG_OBJECT_ALL_ACCESS             (STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | 0x0F)

//
// Debug Object
//
typedef struct _DEBUG_OBJECT
{
	KEVENT EventsPresent;				//+0x00
	FAST_MUTEX Mutex;					//+0x10
	LIST_ENTRY EventList;				//+0x30
										//+0x38
	union
	{
		ULONG Flags;
		struct
		{
			UCHAR DebuggerInactive : 1;
			UCHAR KillProcessOnExit : 1;
		};
	};
} DEBUG_OBJECT, *PDEBUG_OBJECT;

typedef struct _KAPC_STATE
{
	LIST_ENTRY ApcListHead[2];
	PKPROCESS Process;
	BOOLEAN KernelApcInProgress;
	BOOLEAN KernelApcPending;
	BOOLEAN UserApcPending;
} KAPC_STATE, *PKAPC_STATE, *RESTRICTED_POINTER PRKAPC_STATE;

#ifdef __cplusplus
extern "C" {
#endif

	/*
		�ر�ҳ����
	*/
	VOID PageProtectOff();

	/*
		����ҳ����
	*/
	void PageProtectOn();

	/*
		һ�����޸�8�ֽ��ڴ�
	*/
	VOID Hook8b(IN ULONG hookAddr, IN PUCHAR hookBytesArr, OUT PUCHAR oldCode);

	

#ifdef __cplusplus
}
#endif

#endif
