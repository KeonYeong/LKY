[BITS 64]

SECTION .text

global kInPortByte, kOutPortByte, kLoadGDTR, kLoadTR, kLoadIDTR, kEnableInterrupt, kDisableInterrupt, kReadRFLAGS, kReadTSC, kSwitchContext, kHlt
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

; 인터럽트 활성화
; PARAM: 없음
kEnableInterrupt:
	sti ; 활성화 명령어
	ret

; 인터럽트 비활성화
; PARAM: 없음
kDisableInterrupt:
	cli ; 비활성화 명령어
	ret

; RFLAGS 레지스터를 읽어서 반환
; PARAM: 없음
kReadRFLAGS:
	pushfq	; RFLAGS를 스택에 저장하는 명령어
	pop rax ; 반환 값인 ax에다가 넣고 종료
	ret

; 타임 스탬프 카운터 (프로세서 내장)를 읽고 반환
; PARAM: 없음
kReadTSC:
	push rdx ; 상위 하위 바이트 따로 읽기 때문에, rdx, rax 두개를 써서 반환한다
	rdtsc
	shl rdx, 32 ; 상위가 rdx에 있기 때문에 rdx를 왼쪽으로 4바이트씩 옮긴다
	or rax, rdx ; 그 후에 rax에 하나로 or 연산 시키고 반환
	pop rdx
	ret

;;;;;;;;;;;;;;;;;;;;
; 태스크 관련 매크로
;;;;;;;;;;;;;;;;;;;;
; 저장 매크로, ISR에 있는거랑 조금 비슷함, 다만 rsp의 위치가 현재 rdi이고 그것은 CONTEXT 자료구조의 19번째에 지정되어 있기 때문에, push하는 동작이지만 실제 스택이 아닌 CONTEXT 자료구조에 각 레지스터 값을 저장하게 된다
%macro KSAVECONTEXT 0
	push rbp
	push rax
	push rbx
	push rcx
	push rdx
	push rdi
	push rsi
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15

	mov ax, ds
	push rax
	mov ax, es
	push rax
	push fs
	push gs
%endmacro

; 복원 매크로, 마찬가지로 rsp가 rsi이고 그건 두번째 태스크의 CONTEXT 자료구조의 0번지를 가리키고 있기 떄문에, pop을 수행하면 CONTEXT의 자료구조에서 19개를 하나씩 뽑아 내는 것이다
%macro KLOADCONTEXT 0
	pop gs
	pop fs
	pop rax
	mov es, ax
	pop rax
	mov ds, ax

	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop rsi
	pop rdi
	pop rdx
	pop rcx
	pop rbx
	pop rax
	pop rbp
%endmacro

; Current Context의 CONTEXT 자료구조에 현 레지스터 상태들을 저장하고 Next Context의 CONTEXT 자료구조에 있는 값을 현 레지스터에 넣어서 TASK를 바꾸는 함수
; PARAM: Current Context, Next Context
kSwitchContext:
	;;;;;;;;;;;;;;;
	; 콘텍스트 저장
	;;;;;;;;;;;;;;;
	push rbp
	mov rbp, rsp ; 현재 rbp를 일반 스택에 집어넣고, rbp를 스택 베이스 포인터로 사용한다

	; Current Context가 NULL일 경우 콘텍스트 저장이 필요 없기에 검사
	pushfq ; cmp로 RFLAGS가 바뀌니까 저장했다가 다시 불러오는것
	cmp rdi, 0
	je .LoadContext
	popfq

	; 현 태스크 콘텍스트 저장하는데, 우선 위의 5개 ss, rsp, rflags, cs, rip를 저장한다
	push rax ; 오프셋으로 사용 할거라 일단 안의 값은 저장

	mov ax, ss
	mov qword[rdi + (23 * 8)], rax ; ss를 저장하는데, rdi는 CONTEXT 자료구조의 주소이고 거기서 23번째에 저장한다는 것

	mov rax, rbp
	add rax, 16
	mov qword[rdi + (22 * 8)], rax ; rsp를 저장하는 것, rsp는 rbp에 저장되어 있고 현 rbp가 가리키는 스택위치엔 구 rbp 값, return address 값이 차례로 들어 있기 때문에 그것들을 빼고 난 주소값을 rsp로써 저장한다

	pushfq ; RFLAGS를 스택에 넣는 것
	pop rax
	mov qword[rdi + (21 * 8)], rax ; RFLAGS 저장

	mov ax, cs
	mov qword[rdi + (20 * 8)], rax ; cs 저장

	mov rax, qword[rbp + 8]
	mov qword[rdi + (19 * 8)], rax ; rip를 저장하는 것, rbp가 가리키는 스택의 두번째 (구 rbp 다음)가 return address니까 거기로 다시 돌아오기 위해 해당 값을 rip로써 저장한다

	; 저장하는 과정에서 넣었던 rbp, rax를 다시 뽑아내서 복구
	pop rax
	pop rbp

	; CONTEXT 자료구조의 19번째 부터를 위에서 저장했고, 이제 1번째부터 19번째를 저장해야 하기에 해당 자료구조의 19번째를 rsp로 지정한다(Descending Stack이기 때문에)(스택 변경)
	add rdi, (19 * 8)
	mov rsp, rdi
	sub rdi, (19 * 8)

	; 나머지 레지스터를 SAVECONTEXT로 저장
	KSAVECONTEXT

	;;;;;;;;;;;;;;;
	; 콘텍스트 복원
	;;;;;;;;;;;;;;;
.LoadContext: ; 이건 그냥 라벨링
	mov rsp, rsi ; rsi에 있는 값(두번째 태스크의 CONTEXT 자료구조 주소)를 rsp에 넣음으로써 모든 레지스터를 새로운 값으로 복원한다(스택 변경)
	
	KLOADCONTEXT
	iretq ; iretq를 호출하면, 프로세서가 알아서 상위 5개 레지스터의 값을 스택에서 꺼내어 채워넣기 때문에 이대로 종료하면 된다	

; 프로세서를 쉬게함
; PARAM: 없음
kHlt:
	hlt ; 프로세서 대기 상태 진입 명령어
	hlt
	ret
