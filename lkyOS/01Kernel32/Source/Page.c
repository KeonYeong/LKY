#include "Page.h"

// IA-32e 모드 커널을 위한 페이지 테이블 생성
void kInitializePageTables(void){
	PML4TENTRY* pstPML4TEntry;
	PDPTENTRY* pstPDPTEntry;
	PDENTRY* pstPDEntry;
	DWORD dwMappingAddress;
	int i;

	// PML4 테이블 생성
	// 첫번째 엔트리만 쓰기에 나머지는 0
	pstPML4TEntry = (PML4TENTRY*)  0x100000;	// 실제 물리 주소 0x100000을 변수에 넣어주고 바로 변경하는 방식
	// 첫번째 설정, 0x101000은 0x100000 뒤 4KB 뒤의 위치를 가리키므로, PDPT를 가리킨다는 걸 알 수 있다.
	kSetPageEntryData(&(pstPML4TEntry[0]), 0x00, 0x101000, PAGE_FLAGS_DEFAULT, 0);
	// 나머지 0 설정
	for (i = 1; i < PAGE_MAXENTRYCOUNT; i ++){
		kSetPageEntryData(&(pstPML4TEntry[i]), 0, 0, 0, 0);
	}

	// 페이지 디렉터리 포인터 테이블 생성
	// PML4T설정에 따라 하나만 쓴다(하나의 테이블이 512GB까지 매핑이 가능하기에)
	// 테이블 자체는 하나만 쓰고 해당 테이블에서 64개의 엔트리를 설정하게 된다
	pstPDPTEntry = (PDPTENTRY*) 0x101000;
	// 64개 설정, 마찬가지로 한 개의 페이지 테이블 사이즈는 0x1000 (4KB)이기에 적절하게 주소를 넣어준다
	for (i = 0; i < 64; i ++){
		kSetPageEntryData(&(pstPDPTEntry[i]), 0, 0x102000 + (i * PAGE_TABLESIZE), PAGE_FLAGS_DEFAULT, 0);
	}
	// 나머지 엔트리는 0으로 둔다
	for (i = 64; i < PAGE_MAXENTRYCOUNT; i ++){
		kSetPageEntryData(&(pstPDPTEntry[i]), 0, 0, 0, 0);
	}

	// 페이지 디렉터리 테이블 생성
	// 하나의테이블은 1GB 매핑 가능 그래서 위에서 64개의 디렉터리 테이블을 만들었고 이제 해당 테이블들을 전부 매핑
	pstPDEntry = (PDENTRY*) 0x102000; // 모든 페이지 디렉터리의 시작 지점
	dwMappingAddress = 0;
	for (i = 0 ; i < PAGE_MAXENTRYCOUNT * 64; i ++){ // 512개의 엔트리를 가진 디렉터리를 64번 매핑하는 것
		kSetPageEntryData(&(pstPDEntry[i]), (i * (PAGE_DEFAULTSIZE >> 20)) >> 12, dwMappingAddress, PAGE_FLAGS_DEFAULT | PAGE_FLAGS_PS, 0);
		dwMappingAddress += PAGE_DEFAULTSIZE;
	} // 상위 32 비트도 표현해야 하기 때문에, 두번째 인자에서 상위 32비트를 추출해내는 것
}

// 페이지 엔트리에 기준 주소, 속성 플래그 설정
void kSetPageEntryData(PTENTRY* pstEntry, DWORD dwUpperBaseAddress, DWORD dwLowerBaseAddress, DWORD dwLowerFlags, DWORD dwUpperFlags){
	pstEntry->dwAttributeAndLowerBaseAddress = dwLowerBaseAddress | dwLowerFlags;
	pstEntry->dwUpperBaseAddressAndEXB = (dwUpperBaseAddress & 0xFF) | dwUpperFlags; // 상위 8비트만 사용한다
}

