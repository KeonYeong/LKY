#include "Utility.h"

// 메모리 특정 값으로 채우는 것
void kMemSet(void* pvDestination, BYTE bData, int iSize){
	int i;
	for (i = 0; i < iSize; i++)
		((char*)pvDestination)[i] = bData;
}

// 메모리 복사
int kMemCpy(void* pvDestination, const void* pvSource, int iSize){
	int i;
	for (i = 0; i < iSize; i++)
		((char*)pvDestination)[i] = ((char*)pvSource)[i];
	return iSize;
}

// 메모리 비교
int kMemCmp(const void* pvDestination, const void* pvSource, int iSize){
	int i;
	char cTemp;

	for(i = 0; i < iSize; i ++){
		cTemp = ((char*)pvDestination)[i] - ((char*)pvSource)[i];
		if(cTemp != 0)
			return (int)cTemp;
	}
	return 0;
}
