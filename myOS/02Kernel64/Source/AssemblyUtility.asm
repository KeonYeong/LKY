[BITS 64]

SECTION .text

global kInPortByte, kOutPortByte, kLoadGDTR, kLoadTR, kLoadIDTR
; 포트에서 1바이트 읽기
; PARAM: 포트 번호가 들어옴
kInPortByte:
	push rdx
	mov rdx, rdi ; rdi가 여기선 첫번째 파라미터이다 (포트 번호), rdx는 포트 어드레스를 가지게 된다
	mov rax, 0 ; 반환 하는 레지스터로 ax 레지스터를 쓰는데 여기엔 크기순으로 RAX, EAX, AX, AL이 있다 출력 값은 1바이트라 그냥 AL을 쓸것이고 따라서 RAX를 초기화시킨다
	in al, dx ; dx가 rdx의 하위 2바이트이고 이는  포트 어드레스를 지정하고 있다 따라서 이 값을 읽어 AL로 넣어 준다, in은 포트에서 읽는 명령어
	pop rdx
	ret

; 포트에 1바이트 쓰기
; PARAM: 포트 번호, 데이터
kOutPortByte:
	push rdx
	push rax ; rax를 따로 저장해놓는 데 InPortByte의 경우 rax가 반환값을 가지고 있기에 0으로 초기화해주고 썼지만 여기선 반환값을 가지지 않기에 원래 값을 다시 들고 있게 하려고 스택에 넣는다
	mov rdx, rdi ; InPortByte랑 마찬가지
	mov rax, rsi ; 두번째 파라미터(데이터)를 rax에 저장해준다
	out dx, al ; 이번엔 거꾸로 포트에다가 데이터를 쓴다, out은 포트에 쓰는 명령어
	pop rax
	pop rdx
	ret

; GDTR 레지스터에 GDT 테이블 설정
; PARAM: GDT 테이블의 어드레스
kLoadGDTR:
	lgdt [rdi] ; 파라미터1의 값에 있는 걸 gdt 레지스터에 넣는 것
	ret

; TR 레지스터에 TSS 설정
; PARAM: TSS 세그먼트 디스크립터의 offset
kLoadTR:
	ltr di ; 파라미터 1에 있는 걸 tr에 로드하는 것
	ret

; IDTR 레지스터에 IDT 테이블 로드
; PARAM: IDT 테이블의 어드레스
kLoadIDTR:
	lidt [rdi] ; 파라미터 1에 있는 걸 idt 레지스터에 넣음
	ret

