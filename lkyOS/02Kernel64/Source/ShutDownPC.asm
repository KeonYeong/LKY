[BITS 32]

global kShutDownPC

kShutDownPC:
	; 64 -> 32
	mov eax, cr0 
	or eax, 0xC0000000
	xor eax, 0x80000000
	mov cr0, eax 
	mov ecx, 0xc0000080
	rdmsr
	xor eax, 0x0100
	wrmsr
	mov eax, cr4 
	xor eax, 0x20
	mov cr4, eax 
	; 32->16
	mov eax, 0x0 
	mov cr0, eax 
	; Shutdown, ACPI 사용을 위해 특정 인자를 두고, BIOS의 interrupt 15를 발생
	mov ax, 0x5037
	mov cx, 0x0003
	mov bx, 0x0001
	int 0x15
