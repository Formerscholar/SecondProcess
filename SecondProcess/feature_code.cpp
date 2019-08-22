#include "feature_code.h"

/*
	ĳ���ڴ��г���������Ĵ���

	baseAddr		�������ڴ���ʼλ��
	baseAddrSize	�����ķ�Χ
	featureCode		���������飨�����������ģ�
	featureCodeSize	�������С
*/
ULONG FC_AppareFeatureCodeTimes(ULONG baseAddr, ULONG baseAddrSize, PUCHAR featureCode, ULONG featureCodeSize)
{
	PUCHAR pBaseAddr = NULL;
	ULONG times = 0;

	ULONG i = 0;
	ULONG j = 0;

	pBaseAddr = (PUCHAR)baseAddr;

	for (; i < baseAddrSize; i++) {
		if (pBaseAddr[i] == featureCode[0]) {
			for (j = 1; j < featureCodeSize; j++) {
				if (pBaseAddr[i + j] != featureCode[j]) {
					break;
				}
			}

			if (j == featureCodeSize) times++;
		}
	}

	return times;
}

/*
	ʹ����������ĳ���ڴ��������ݣ���������������ַ

	baseAddr		�������ڴ���ʼλ��
	baseAddrSize	�����ķ�Χ
	featureCode		���������飨�����������ģ�
	featureCodeSize	�������С
	distance		�������һ���ֽ���Ҫ�ҵĵ�ַ�ľ���
	isBefore		�������һ���ֽ��Ƿ���Ҫ�ҵĵ�ַ��ǰ�� 1�� 0��
*/
ULONG FC_FindDataAddr(ULONG baseAddr, ULONG baseAddrSize, UCHAR* featureCode, ULONG featureCodeSize, ULONG distance, ULONG isBefore)
{
	ULONG appareTimes = 0;

	PUCHAR pBaseAddr = NULL;

	ULONG i = 0;
	ULONG j = 0;

	//���������������ڴ��г��ֵĴ�����������1ִ�н���
	if (1 != (appareTimes = FC_AppareFeatureCodeTimes(baseAddr, baseAddrSize, featureCode, featureCodeSize))) {
		DbgPrint("%s(%d): FeatureCode appare times: %d. \n", __FILE__, __LINE__, appareTimes);

		return 0;
	}

	pBaseAddr = (PUCHAR)baseAddr;

	for (; i < baseAddrSize; i++) {
		if (pBaseAddr[i] == featureCode[0]) {
			for (j = 1; j < featureCodeSize; j++) {
				if (pBaseAddr[i + j] != featureCode[j]) {
					break;
				}
			}

			if (j == featureCodeSize) {
				if (0 == isBefore) {
					return (ULONG)pBaseAddr + i - distance;
				}
				else {
					return (ULONG)pBaseAddr + i + distance;
				}
			}
		}
	}

	return 0;
}