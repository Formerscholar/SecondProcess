#ifndef __FEATURE_CODE_H_
#define __FEATURE_CODE_H_

#include <ntddk.h>

#ifdef __cplusplus
extern "C" {
#endif

	/*
		ĳ���ڴ��г���������Ĵ���

		baseAddr		�������ڴ���ʼλ��
		baseAddrSize	�����ķ�Χ
		featureCode		���������飨�����������ģ�
		featureCodeSize	�������С
	*/
	ULONG FC_AppareFeatureCodeTimes(ULONG baseAddr, ULONG baseAddrSize, PUCHAR featureCode, ULONG featureCodeSize);

	/*
		ʹ����������ĳ���ڴ��������ݣ���������������ַ

		baseAddr		�������ڴ���ʼλ��
		baseAddrSize	�����ķ�Χ
		featureCode		���������飨�����������ģ�
		featureCodeSize	�������С
		distance		�������һ���ֽ���Ҫ�ҵĵ�ַ�ľ���
		isBefore		�������һ���ֽ��Ƿ���Ҫ�ҵĵ�ַ��ǰ�� 1�� 0��
	*/
	ULONG FC_FindDataAddr(ULONG baseAddr, ULONG baseAddrSize, UCHAR* featureCode, ULONG featureCodeSize, ULONG distance, ULONG isBefore);


#ifdef __cplusplus
}
#endif

#endif
