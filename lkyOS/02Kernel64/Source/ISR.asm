[BITS 64]

SECTION .text

; 외부 함수를 쓸 수 있게 가져옴
extern kCommonExceptionHandler, kCommonInterruptHandler, kKeyboardHandler, kTimerHandler

; 정의한 함수들을 외부에서 쓸 수 있게 선언
; 이하는 예외(Exception)을 위한 것
global kISRDivideError, kISRDebug, kISRNMI, kISRBreakPoint, kISROverflow, kISRBoundRangeExceeded, kISRInvalidOpcode, kISRDeviceNotAvailable, kISRDoubleFault, kISRCoprocessorSegmentOverrun, kISRInvalidTSS, kISRSegmentNotPresent, kISRStackSegmentFault, kISRGeneralProtection, kISRPageFault, kISR15, kISRFPUError, kISRAlignmentCheck, kISRMachineCheck, kISRSIMDError, kISRETCException
; 이하는 인터럽트(Interrupt)를 위한것
global kISRTimer, kISRKeyboard, kISRSlavePIC, kISRSerial2, kISRSerial1, kISRParallel2, kISRFloppy, kISRParallel1, kISRRTC, kISRReserved, kISRNotUsed1, kISRNotUsed2, kISRMouse, kISRCoprocessor, kISRHDD1, kISRHDD2, kISRETCInterrupt

; 콘텍스트 저장을 위한 매크로
%macro KSAVECONTEXT 0 ; 이름은 KSAVECONTEXT, 인자는 없음
	; RBP 부터 GS까지 전부 스택 삽입 (프로세서가 알아서 처리하는 건 제외되있음 : 스택 포인터, eip 등)
	push rbp
	mov rbp, rsp ; 현재 스택포인터 위치에서 + 8을 하면 에러코드의 위치가 나오는데 에러코드 삽입이 필요한 인터럽트가 존재함으로 그것을 수행하기 위해 현재 스택 위치를 rbp에다가 저장하여 나중에 rbp + 8로 처리하기 위함
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
	mov ax, ds ; ds, es는 스택에 바로 못 넣는다 해서 ax에 넣고 스택에 넣음
	push rax
	mov ax, es
	push rax
	push fs
	push gs

	; 세그먼트 셀렉터 교체
	mov ax, 0x10 ; 커널 데이터 세그먼트!
	mov ds, ax
	mov es, ax
	mov gs, ax
	mov fs, ax
%endmacro

; 콘텍스트 복원을 위한 매크로
%macro KLOADCONTEXT 0
	; 저장과 달리 gs부터 rbp까지 거꾸로 복원
	pop gs
	pop fs
	pop rax ; 마찬가지, ds, es는 스택에서 바로 못 빼서 ax에 넣고 옮김
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

;;;;;;;;;;;;;;;;;;;;;;
;
; 이제부터 예외 핸들러
;
;;;;;;;;;;;;;;;;;;;;;;

; #0, Divide Error ISR
kISRDivideError:
	; 우선 콘텍스트를 저장하고 세그먼트도 교체해준다
	KSAVECONTEXT

	; rdi는 파라미터이고 여기에 예외 번호(여기는 0)을 삽입하고 핸들러를 호출한다
	mov rdi, 0
	; 만약에 에러 코드 또한 삽입해야 한다면 아까 rbp에 당시 rsp를 저장했기에 거기서 스택에 내용을 불러온다
	; mov rsi, qword [rbp + 8], 여기선 안씀
	call kCommonExceptionHandler

	; 이제 콘텍스트 복원
	KLOADCONTEXT

	; 종료
	iretq

;;;;;;;;;;;;;;;; 이하 동일 ;;;;;;;;;;;;;;;;;

; #1, Debug ISR
kISRDebug:
	KSAVECONTEXT

	mov rdi, 1
	call kCommonExceptionHandler

	KLOADCONTEXT
	iretq

; #2, NMI ISR
kISRNMI:
	KSAVECONTEXT

	mov rdi, 2
	call kCommonExceptionHandler

	KLOADCONTEXT 
	iretq

; #3, BreakPoint ISR
kISRBreakPoint:
	KSAVECONTEXT

	mov rdi, 3
	call kCommonExceptionHandler

	KLOADCONTEXT
	iretq

; #4, Overflow ISR
kISROverflow:
	KSAVECONTEXT

	mov rdi, 4
	call kCommonExceptionHandler

	KLOADCONTEXT
	iretq      

; #5, Bound Range Exceeded ISR
kISRBoundRangeExceeded:
	KSAVECONTEXT

	mov rdi, 5
	call kCommonExceptionHandler

	KLOADCONTEXT
	iretq

; #6, Invalid Opcode ISR
kISRInvalidOpcode:
	KSAVECONTEXT

	mov rdi, 6
	call kCommonExceptionHandler

	KLOADCONTEXT
	iretq

; #7, Device Not Available ISR
kISRDeviceNotAvailable:
	KSAVECONTEXT

	mov rdi, 7
	call kCommonExceptionHandler

	KLOADCONTEXT
	iretq

; #8, Double Fault ISR
kISRDoubleFault:
	KSAVECONTEXT

	mov rdi, 8
	mov rsi, qword [ rbp + 8 ]
	call kCommonExceptionHandler

	KLOADCONTEXT
	add rsp, 8
	iretq

; #9, Coprocessor Segment Overrun ISR
kISRCoprocessorSegmentOverrun:
	KSAVECONTEXT

	mov rdi, 9
	call kCommonExceptionHandler

	KLOADCONTEXT
	iretq

; #10, Invalid TSS ISR
kISRInvalidTSS:
	KSAVECONTEXT

	mov rdi, 10
	mov rsi, qword [ rbp + 8 ]
	call kCommonExceptionHandler

	KLOADCONTEXT
	add rsp, 8
	iretq

; #11, Segment Not Present ISR
kISRSegmentNotPresent:
	KSAVECONTEXT

	mov rdi, 11
	mov rsi, qword [ rbp + 8 ]
	call kCommonExceptionHandler

	KLOADCONTEXT
	add rsp, 8
	iretq

; #12, Stack Segment Fault ISR
kISRStackSegmentFault:
	KSAVECONTEXT

	mov rdi, 12
	mov rsi, qword [ rbp + 8 ]
	call kCommonExceptionHandler

	KLOADCONTEXT
	add rsp, 8
	iretq

; #13, General Protection ISR
kISRGeneralProtection:
	KSAVECONTEXT

	mov rdi, 13
	mov rsi, qword [rbp + 8]
	call kCommonExceptionHandler

	KLOADCONTEXT
	add rsp, 8
	iretq

; #14, Page Fault ISR
kISRPageFault:
	KSAVECONTEXT

	mov rdi, 14
	mov rsi, qword [rbp + 8]
	call kCommonExceptionHandler

	add rsp, 8
	KLOADCONTEXT
	iretq

; #15, Reserved ISR
kISR15:
	KSAVECONTEXT

	mov rdi, 15
	call kCommonExceptionHandler

	KLOADCONTEXT
	iretq

; #16, FPU Error ISR
kISRFPUError:
	KSAVECONTEXT

	mov rdi, 16
	call kCommonExceptionHandler

	KLOADCONTEXT
	iretq


; #17, Alignment Check ISR
kISRAlignmentCheck:
	KSAVECONTEXT

	mov rdi, 17
	mov rsi, qword [rbp + 8]
	call kCommonExceptionHandler

	add rsp, 8
	KLOADCONTEXT
	iretq


; #18, Machine Check ISR
kISRMachineCheck:
	KSAVECONTEXT

	mov rdi, 18
	call kCommonExceptionHandler

	KLOADCONTEXT
	iretq


; #19, SIMD Floating Point Exception ISR
kISRSIMDError:
	KSAVECONTEXT

	mov rdi, 19
	call kCommonExceptionHandler

	KLOADCONTEXT
	iretq


; #20~#31, Reserved ISR
kISRETCException:
	KSAVECONTEXT

	mov rdi, 20
	call kCommonExceptionHandler

	KLOADCONTEXT
	iretq

;;;;;;;;;;;;;;;;;;;;;;;;;;
; 
; 이제부터 인터럽트 핸들러
;
;;;;;;;;;;;;;;;;;;;;;;;;;;

; #32, 타이머
kISRTimer:
	KSAVECONTEXT

	mov rdi, 32
	call kTimerHandler

	KLOADCONTEXT
	iretq

; #33, 키보드
kISRKeyboard:
	KSAVECONTEXT

	mov rdi, 33
	call kKeyboardHandler

	KLOADCONTEXT
	iretq

; #34, 슬레이브 PIC
kISRSlavePIC:
	KSAVECONTEXT

	mov rdi, 34
	call kCommonInterruptHandler

	KLOADCONTEXT
	iretq

; #35, 시리얼 포트 2
kISRSerial2:
	KSAVECONTEXT

	mov rdi, 35
	call kCommonInterruptHandler

	KLOADCONTEXT
	iretq

; #36, 시리얼 포트 1
kISRSerial1:
	KSAVECONTEXT

	mov rdi, 36
	call kCommonInterruptHandler

	KLOADCONTEXT
	iretq

; #37, 패러렐 포트 2
kISRParallel2:
	KSAVECONTEXT

	mov rdi, 37
	call kCommonInterruptHandler

	KLOADCONTEXT
	iretq

; #38, 플로피 디스크 컨트롤러
kISRFloppy:
	KSAVECONTEXT

	mov rdi, 38
	call kCommonInterruptHandler

	KLOADCONTEXT
	iretq

; #39, 패러렐 포트 1
kISRParallel1:
	KSAVECONTEXT

	mov rdi, 39
	call kCommonInterruptHandler

	KLOADCONTEXT
	iretq

; #40, RTC
kISRRTC:
	KSAVECONTEXT

	mov rdi, 40
	call kCommonInterruptHandler

	KLOADCONTEXT
	iretq

; #41, 예약된 인터럽트
kISRReserved:
	KSAVECONTEXT

	mov rdi, 41
	call kCommonInterruptHandler

	KLOADCONTEXT
	iretq

; #42, 사용 안함
kISRNotUsed1:
	KSAVECONTEXT

	mov rdi, 42
	call kCommonInterruptHandler

	KLOADCONTEXT
	iretq

; #43, 사용 안함
kISRNotUsed2:
	KSAVECONTEXT

	mov rdi, 43
	call kCommonInterruptHandler

	KLOADCONTEXT
	iretq

; #44, 마우스
kISRMouse:
	KSAVECONTEXT

	mov rdi, 44
	call kCommonInterruptHandler

	KLOADCONTEXT
	iretq

; #45, 코프로세서
kISRCoprocessor:
	KSAVECONTEXT

	mov rdi, 45
	call kCommonInterruptHandler

	KLOADCONTEXT
	iretq

; #46, 하드 디스크 1
kISRHDD1:
	KSAVECONTEXT

	mov rdi, 46
	call kCommonInterruptHandler

	KLOADCONTEXT
	iretq

; #47, 하드 디스크 2
kISRHDD2:
	KSAVECONTEXT

	mov rdi, 47
	call kCommonInterruptHandler

	KLOADCONTEXT
	iretq

; #48, 이외의 모든 인터럽트
kISRETCInterrupt:
	KSAVECONTEXT

	mov rdi, 48
	call kCommonInterruptHandler

	KLOADCONTEXT
	iretq
