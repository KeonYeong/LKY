[BITS 64]

SECTION .text

; 외부 함수 사용하도록 선언
extern Main

START:
	mov ax, 0x10 ; IA_32eDATADESCRIPTOR을 ax에 넣고 나머지 레지스터에 넣는것
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	; 스택 생성하는 것, 1MB 크기
	mov ss, ax
	mov rsp, 0x6FFFF8
	mov rbp, 0x6FFFF8

	; Main 함수 호출
	call Main

	jmp $
