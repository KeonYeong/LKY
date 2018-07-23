[BITS 32]

; C 커널에서 볼 수 있도록 노출
global kReadCPUID, kSwitchAndExecute64bitKernel

SECTION .text

; CPUID 반환함수
; 인자는 dwEAX, *pdwEAX, *pdwEBX, *pdwECX, *pdwEDX
kReadCPUID:
	push ebp
	mov ebp, esp
	push eax
	push ebx
	push ecx
	push edx
	push esi

	; EAX 값으로 실제 cpuid 실행 부분
	mov eax, dword[ebp + 8] ; ebp는 스택이고 스택 첫째포인터는 빈 공간, 그 다음은 ret addr, 그 다음이 첫번째 인자, dwEAX를 eax에 저장하는 것
	cpuid

	; cpuid 실행 후 반환 값들을 파라미터에 저장
	; pdwEAX
	mov esi, dword[ebp + 12]
	mov dword[esi], eax

	; pdwEBX
	mov esi, dword[ebp + 16]
	mov dword[esi], ebx

	; pdwECX
	mov esi, dword[ebp + 20]
	mov dword[esi], ecx

	; pdwEDX
	mov esi, dword[ebp + 24]
	mov dword[esi], edx

	; 모든 과정 완료 후 레지스터 원래 값 복구
	pop esi
	pop edx
	pop ecx
	pop ebx
	pop eax
	pop ebp
	ret

; IA-32e 모드 전환 후 64비트 커널 수행 함수
; 인자 없음
kSwitchAndExecute64bitKernel:
	; CR4 레지스터 PAE 비트 1 설정
	mov eax, cr4
	or eax, 0x20 ; PAE 비트 (비트 5) = 1
	mov cr4, eax

	; CR3 레지스터에 PML4 테이블 주소 넣고 캐시 활성화
	mov eax, 0x100000
	mov cr3, eax

	; IA32_EFER.LME를 1로 설정함으로써 IA-32e 모드 활성화
	mov ecx, 0xC0000080 ; IA32_EFER 레지스터를 읽겠다는 뜻
	rdmsr
	or eax, 0x0100 ; LME 비트를 1로 설정 (eax 가 하위 32비트가 저장되있음)
	wrmsr

	; CR0 레지스터 NW 비트, CD 비트 0으로 PG 비트 1로 설정하여 캐시 와 페이징 활성화 (NW = non write-through, CD = Cache Disable)
	mov eax, cr0
	or eax, 0xE0000000 ; 전부 1로 바꾸고
	xor eax, 0x60000000 ; xor로 nw, cd만 0으로 바꿈
	mov cr0, eax

	; cs 세그먼트 셀렉터를 IA-32e 모드용 코드 세그먼트 디스크립터 IA_32eCODEDESCRIPTOR로 변경 하여 2MB 어드레스로 이동
	jmp 0x08:0x200000
	
	; 여긴 실행 안됨
	jmp $




