#include "Types.h"
#include "AssemblyUtility.h"
#include "Keyboard.h"

// 키보드 컨트롤러 상태 체크 함수
BOOL kIsOutputBufferFull(void){
	// 비트 0이 output관련이기에 0x01과 &를 한다
	if(kInPortByte(0x64) & 0x01) // 0x64는 상태 레지스터 포트번호
		return TRUE;
	return FALSE;
}

BOOL kIsInputBufferFull(void){
	// 비트 1이 input관련이기에 0x02과 &를 한다
	if(kInPortByte(0x64) & 0x02)
		return TRUE;
	return FALSE;
}

// 키보드 활성화 함수
BOOL kActivateKeyboard(void)
{
	int i, j;

	// 키보드의 컨트롤 레지스터 (상태와 마찬가지로 0x64) 하지만 상위 비트를 씀 0xAE를 전달하여 활성화
	// 이건 키보드 컨트롤러에서 활성화
	kOutPortByte(0x64, 0xAE);

	// 이제 입력버퍼를 1로 바꿨으니 키보드가 가져갈 때까지 기다림, 헌데 루프를 도는데 루프 제한은 0xFFFF (이걸로도 충분하다고 함)
	// 키보드가 가져가게 되면 입력버퍼는 0이 된다
	for(i = 0; i < 0xFFFF ; i ++){
		if(kIsInputBufferFull() == FALSE)
			break;
	}

	// 이제 키보드 자체를 활성화 입력버퍼에 0xF4를 줌으로써 직접 활성화한다
	kOutPortByte(0x60, 0xF4);

	// 키보드가 ACK를 보내야만 활성화가 완료된 것이다, 따라서 출력 버퍼를 체크한다 ACK는 0xFA이다
	// 키보드가 ACK 전에 다른 키 데이터를 보낼 수도 있어서 100가지의 출력을 체크하게 된다
	for(i = 0; i < 100; i ++){
		// 이제 내부에서 아까 했던것과 마찬가지로 0xFFFF까지만 신호가 있는 지 체크한다
		for(j = 0; j < 0xFFFF; j ++){
			// 출력 버퍼가 찼는 지 검사
			if(kIsOutputBufferFull() == TRUE)
				break;
		}

		// 0xFFFF 후에는 버퍼가 차있든 말든 무시하고 걍 읽어본다, ACK의 신호(0xFA)와 같다면 TRUE를 반환하고 종료한다
		if(kInPortByte(0x60) == 0xFA)
			return TRUE;
	}
	return FALSE;
}

// 키보드의 스캔 코드 읽기 (스캔코드는 키보드가 출력하는 값)
BYTE kGetKeyboardScanCode(void){
	// 우선 Output 버퍼가 찰 때까지 기다리다가
	while(kIsOutputBufferFull()==FALSE);

	// 차는 순간 읽어들이고 반환한다
	return kInPortByte(0x60);
}

// 키보드 출력포트에다가 특정 커맨드로 A20게이트 활성화하는 함수
void kEnableA20Gate(void){
	BYTE bOutputPortData;
	int i;

	// 출력 포트 값을 읽는 커맨드 전송(0xD0 커맨드)
	kOutPortByte(0x64, 0xD0);

	// 출력 포트 값을 읽어보기
	for(i = 0; i < 0xFFFF; i ++){
		if(kIsOutputBufferFull() == TRUE)
			break;
	}

	// 버퍼에서 읽어온 뒤 2번째 비트 (비트[1])를 1로 설정하여 A20 활성화를 설정
	bOutputPortData = kInPortByte(0x60);
	bOutputPortData |= 0x02;

	// 입력 버퍼 비었을 때 거기에 출력 포트 데이터, 커맨드 입력으로 활성화한다
	// 비기를 기다림
	for(i = 0; i < 0xFFFF; i ++){
		if(kIsInputBufferFull() == FALSE)
			break;
	}

	// 입력 버퍼 출력포트로 복사하는 커맨드를 컨트롤 레지스터에 입력
	kOutPortByte(0x64, 0xD1);

	// 입력 버퍼에 아까 만든 데이터를 입력
	kOutPortByte(0x60, bOutputPortData);
}

// 프로세서 리셋하는 것인데 A20 게이트와 달라진 점은 읽을 필요도 없고 그냥 0이란 값을 출력 포트에 넣으면 된다
void kReboot(void){
	int i;

	for(i = 0; i < 0xFFFF; i ++){
		if(kIsInputBufferFull() == FALSE)
			break;
	}

	kOutPortByte(0x64, 0xD1);

	kOutPortByte(0x60, 0x00);

	while(1);
}

// 키보드 LED (넘락, 캡스락, 스크롤락) 상태 제어하는 함수, 인자로 설정 여부를 1비트로 받는다(1 = ON, 0 = OFF)
BOOL kChangeKeyboardLED(BOOL bCapsLockOn, BOOL bNumLockOn, BOOL bScrollLockOn){
	int i, j;

	// LED 변경 커맨드를 전송한다 (커맨드 = 0xED), LED 변경은 오직 입력버퍼로만 이루어진다
	for (i = 0; i < 0xFFFF; i ++){
		if(kIsInputBufferFull() == FALSE)
			break;
	}

	// 입력 버퍼에 커맨드 (0xED)전송
	kOutPortByte(0x60, 0xED);
	// 후 가져갔는 지 확인
	for(i = 0; i < 0xFFFF; i ++){
		if(kIsInputBufferFull() == FALSE)
			break;
	}

	// Activate할때처럼 ACK를 확인하는데 100개의 데이터를 보며 확인한다
	for(i = 0; i < 100; i ++){
		for(j = 0; j < 0xFFFF; j ++){
			if(kIsOutputBufferFull() == TRUE)
				break;
		}

		//ACK 체크
		if(kInPortByte(0x60) == 0xFA)
			break;
	}

	// 루프 끝에 닿았다면 ACK가 안온것, 그러면 fail
	if(i >= 100) 
		return FALSE;

	// 이제 원하는 LED 상태를 입력 버퍼에 씀
	kOutPortByte(0x60, (bCapsLockOn << 2) | (bNumLockOn << 1) | bScrollLockOn);
	// 후 확인
	for(i = 0; i < 0xFFFF; i ++){
		if(kIsInputBufferFull() == FALSE)
			break;
	}

	// 다시 ACK 체크
	for(i = 0; i < 100; i ++){
		for(j = 0; j < 0xFFFF; j ++){
			if(kIsOutputBufferFull() == TRUE)
				break;
		}

		// ACK 체크
		if(kInPortByte(0x60) == 0xFA)
			break;
	}

	// 100 이면 실패
	if(i >= 100)
		return FALSE;

	return TRUE;
}

// 키보드 상태 관리하는 매니저
static KEYBOARDMANAGER gs_stKeyboardManager = {0, };

// 스캔 코드를 그대로 아스키 코드로 반환하는 테이블, 스캔코드를 index로 해서 이 배열에 주면 배열이 가지고 있는 값이 해당 아스키 코드이다
static KEYMAPPINGENTRY gs_vstKeyMappingTable[ KEY_MAPPINGTABLEMAXCOUNT ] =
{
	/*  0   */  {   KEY_NONE        ,   KEY_NONE        },
	/*  1   */  {   KEY_ESC         ,   KEY_ESC         },
	/*  2   */  {   '1'             ,   '!'             },
	/*  3   */  {   '2'             ,   '@'             },
	/*  4   */  {   '3'             ,   '#'             },
	/*  5   */  {   '4'             ,   '$'             },
	/*  6   */  {   '5'             ,   '%'             },
	/*  7   */  {   '6'             ,   '^'             },
	/*  8   */  {   '7'             ,   '&'             },
	/*  9   */  {   '8'             ,   '*'             },
	/*  10  */  {   '9'             ,   '('             },
	/*  11  */  {   '0'             ,   ')'             },
	/*  12  */  {   '-'             ,   '_'             },
	/*  13  */  {   '='             ,   '+'             },
	/*  14  */  {   KEY_BACKSPACE   ,   KEY_BACKSPACE   },
	/*  15  */  {   KEY_TAB         ,   KEY_TAB         },
	/*  16  */  {   'q'             ,   'Q'             },
	/*  17  */  {   'w'             ,   'W'             },
	/*  18  */  {   'e'             ,   'E'             },
	/*  19  */  {   'r'             ,   'R'             },
	/*  20  */  {   't'             ,   'T'             },
	/*  21  */  {   'y'             ,   'Y'             },
	/*  22  */  {   'u'             ,   'U'             },
	/*  23  */  {   'i'             ,   'I'             },
	/*  24  */  {   'o'             ,   'O'             },
	/*  25  */  {   'p'             ,   'P'             },
	/*  26  */  {   '['             ,   '{'             },
	/*  27  */  {   ']'             ,   '}'             },
	/*  28  */  {   '\n'            ,   '\n'            },
	/*  29  */  {   KEY_CTRL        ,   KEY_CTRL        },
	/*  30  */  {   'a'             ,   'A'             },
	/*  31  */  {   's'             ,   'S'             },
	/*  32  */  {   'd'             ,   'D'             },
	/*  33  */  {   'f'             ,   'F'             },
	/*  34  */  {   'g'             ,   'G'             },
	/*  35  */  {   'h'             ,   'H'             },
	/*  36  */  {   'j'             ,   'J'             },
	/*  37  */  {   'k'             ,   'K'             },
	/*  38  */  {   'l'             ,   'L'             },
	/*  39  */  {   ';'             ,   ':'             },
	/*  40  */  {   '\''            ,   '\"'            },
	/*  41  */  {   '`'             ,   '~'             },
	/*  42  */  {   KEY_LSHIFT      ,   KEY_LSHIFT      },
	/*  43  */  {   '\\'            ,   '|'             },
	/*  44  */  {   'z'             ,   'Z'             },
	/*  45  */  {   'x'             ,   'X'             },
	/*  46  */  {   'c'             ,   'C'             },
	/*  47  */  {   'v'             ,   'V'             },
	/*  48  */  {   'b'             ,   'B'             },
	/*  49  */  {   'n'             ,   'N'             },
	/*  50  */  {   'm'             ,   'M'             },
	/*  51  */  {   ','             ,   '<'             },
	/*  52  */  {   '.'             ,   '>'             },
	/*  53  */  {   '/'             ,   '?'             },
	/*  54  */  {   KEY_RSHIFT      ,   KEY_RSHIFT      },
	/*  55  */  {   '*'             ,   '*'             },
	/*  56  */  {   KEY_LALT        ,   KEY_LALT        },
	/*  57  */  {   ' '             ,   ' '             },
	/*  58  */  {   KEY_CAPSLOCK    ,   KEY_CAPSLOCK    },
	/*  59  */  {   KEY_F1          ,   KEY_F1          },
	/*  60  */  {   KEY_F2          ,   KEY_F2          },
	/*  61  */  {   KEY_F3          ,   KEY_F3          },
	/*  62  */  {   KEY_F4          ,   KEY_F4          },
	/*  63  */  {   KEY_F5          ,   KEY_F5          },
	/*  64  */  {   KEY_F6          ,   KEY_F6          },
	/*  65  */  {   KEY_F7          ,   KEY_F7          },
	/*  66  */  {   KEY_F8          ,   KEY_F8          },
	/*  67  */  {   KEY_F9          ,   KEY_F9          },
	/*  68  */  {   KEY_F10         ,   KEY_F10         },
	/*  69  */  {   KEY_NUMLOCK     ,   KEY_NUMLOCK     },
	/*  70  */  {   KEY_SCROLLLOCK  ,   KEY_SCROLLLOCK  },

	/*  71  */  {   KEY_HOME        ,   '7'             },
	/*  72  */  {   KEY_UP          ,   '8'             },
	/*  73  */  {   KEY_PAGEUP      ,   '9'             },
	/*  74  */  {   '-'             ,   '-'             },
	/*  75  */  {   KEY_LEFT        ,   '4'             },
	/*  76  */  {   KEY_CENTER      ,   '5'             },
	/*  77  */  {   KEY_RIGHT       ,   '6'             },
	/*  78  */  {   '+'             ,   '+'             },
	/*  79  */  {   KEY_END         ,   '1'             },
	/*  80  */  {   KEY_DOWN        ,   '2'             },
	/*  81  */  {   KEY_PAGEDOWN    ,   '3'             },
	/*  82  */  {   KEY_INS         ,   '0'             },
	/*  83  */  {   KEY_DEL         ,   '.'             },
	/*  84  */  {   KEY_NONE        ,   KEY_NONE        },
	/*  85  */  {   KEY_NONE        ,   KEY_NONE        },
	/*  86  */  {   KEY_NONE        ,   KEY_NONE        },
	/*  87  */  {   KEY_F11         ,   KEY_F11         },
	/*  88  */  {   KEY_F12         ,   KEY_F12         }
};

// 이제 스캔 코드를 아스키 코드로 꾸는 함수
// 우선은 스캔 코드를 아스키로 변환할때 해당 문자가 조합된 키로 반환해야 하는지 확인하는 함수를 만든다
BOOL kIsUseCombinedCode(BYTE bScanCode){
	BYTE bDownScanCode;
	BOOL bUseCombinedKey = FALSE;

	// 7F는 01111111이고, 최상위 비트는 뗐을 때 1로 되기에 해당 부분을 0으로 하여 &함으로써 눌렸을 때 스캔 코드로 만든다
	bDownScanCode = bScanCode & 0x7F;

	// 알파벳은 Shift와 Caps에 따라 조합키로 변경
	if(kIsAlphabetScanCode(bDownScanCode)==TRUE){
		// gs_stKeyboardManager 은 키보드 상태 관리하는 자료구조 KEYBOARDMANAGER
		if(gs_stKeyboardManager.bShiftDown ^ gs_stKeyboardManager.bCapsLockOn)
			bUseCombinedKey = TRUE;
		else bUseCombinedKey = FALSE;
	}

	// 숫자와 기호 키일 경우 Shift
	else if(kIsNumberOrSymbolScanCode(bDownScanCode)==TRUE){
		if(gs_stKeyboardManager.bShiftDown == TRUE)
			bUseCombinedKey = TRUE;
		else bUseCombinedKey = FALSE;
	}

	// 숫자 패드의 키면 Num Lock
	// 확장 키와 0xE0을 제외하면 겹치기 때문에, 확장 키가 아닐 경우만
	else if((kIsNumberPadScanCode(bDownScanCode)==TRUE) && (gs_stKeyboardManager.bExtendedCodeIn == FALSE)){
		if(gs_stKeyboardManager.bNumLockOn == TRUE)
			bUseCombinedKey = TRUE;
		else bUseCombinedKey = FALSE;
	}

	return bUseCombinedKey;
}

// 위에서 쓴 스캔 코드의 그룹 여부 판단 함수들
BOOL kIsAlphabetScanCode(BYTE bScanCode){
	if(('a' <= gs_vstKeyMappingTable[bScanCode].bNormalCode) && ('z' >= gs_vstKeyMappingTable[bScanCode].bNormalCode))
		return TRUE;
	return FALSE;
}
BOOL kIsNumberOrSymbolScanCode(BYTE bScanCode){
	// 숫자 패드, 확장 키 범위를 제외한 상황(2 ~ 53)에서 알파벳이 아니라면 숫자, 기호라는 것
	if((2 <= bScanCode) && (53 >= bScanCode) && (kIsAlphabetScanCode(bScanCode) == FALSE))
		return TRUE;
	return FALSE;
}
BOOL kIsNumberPadScanCode(BYTE bScanCode){
	// 숫자 패드는 71 ~ 83임
	if((71 <= bScanCode) && (83 >= bScanCode))
		return TRUE;
	return FALSE;
}

// 이제 조합 키일 경우(Shift 등) 상태 갱신 함수
void UpdateCombinationKeyStatusAndLED(BYTE bScanCode){
	BOOL bDown;
	BYTE bDownScanCode;
	BOOL bLEDStatusChanged = FALSE;

	// 최상위 비트 확인해서 눌려있는 상태인지 체크하고 저장, 1이면 뗀 것
	if(bScanCode & 0x80){
		bDown = FALSE;
		bDownScanCode = bScanCode & 0x7F;
	}
	else{
		bDown = TRUE;
		bDownScanCode = bScanCode;
	}

	// 42, 54 = Shift, 58 = 캡스락, 69 = 넘락, 70 = 스크롤락, 나머지 무시 락들일 경우 LED도 바꿔야함
	switch(bDownScanCode){
		case 42:
		case 54:
			gs_stKeyboardManager.bShiftDown = bDown;
			break;
		case 58:
			if(bDown == TRUE){
				gs_stKeyboardManager.bCapsLockOn ^= TRUE;
				bLEDStatusChanged = TRUE;
			}
			break;
		case 69:
			if(bDown == TRUE){
				gs_stKeyboardManager.bNumLockOn ^= TRUE;
				bLEDStatusChanged = TRUE;
			}
			break;
		case 70:
			if(bDown == TRUE){
				gs_stKeyboardManager.bScrollLockOn ^= TRUE;
				bLEDStatusChanged = TRUE;
			}
			break;
		default:
			break;
	}

	// 락들일 경우 LED 상태도 바꿈
	if(bLEDStatusChanged == TRUE)
		kChangeKeyboardLED(gs_stKeyboardManager.bCapsLockOn, gs_stKeyboardManager.bNumLockOn, gs_stKeyboardManager.bScrollLockOn);
}

BOOL kConvertScanCodeToASCIICode(BYTE bScanCode, BYTE* pbASCIICode, BOOL* pbFlags){
	BOOL bUseCombinedKey;

	// PAUSE 키가 이전에 눌렸으면, 2번동안은 무시하기
	if(gs_stKeyboardManager.iSkipCountForPause > 0){
		gs_stKeyboardManager.iSkipCountForPause--;
		return FALSE;
	}

	// Pause 키 일 경우
	if(bScanCode == 0xE1){
		*pbASCIICode = KEY_PAUSE;
		*pbFlags = KEY_FLAGS_DOWN;
		gs_stKeyboardManager.iSkipCountForPause = KEY_SKIPCOUNTFORPAUSE;
		return TRUE;
	}
	// 확장 키 코드일 경우, 실제 값은 다음 키 값이므로 플래그만 설정 후 종료
	else if(bScanCode == 0xE0){
		gs_stKeyboardManager.bExtendedCodeIn = TRUE;
		return FALSE;
	}

	// 조합 키 반환 여부 체크
	bUseCombinedKey = kIsUseCombinedCode(bScanCode);

	// 실제 키 값 설정
	if(bUseCombinedKey == TRUE)
		*pbASCIICode = gs_vstKeyMappingTable[bScanCode & 0x7F].bCombinedCode;
	else *pbASCIICode = gs_vstKeyMappingTable[bScanCode & 0x7F].bNormalCode;

	// 확장 키 유무 설정
	if(gs_stKeyboardManager.bExtendedCodeIn == TRUE){
		*pbFlags = KEY_FLAGS_EXTENDEDKEY;
		gs_stKeyboardManager.bExtendedCodeIn = FALSE;
	}
	else *pbFlags = 0;

	// 눌러짐 또는 떨어짐 유무 설정
	if((bScanCode & 0x80) == 0) // 눌려있을 경우
		*pbFlags |= KEY_FLAGS_DOWN;

	// 조합 키 갱신 부분
	UpdateCombinationKeyStatusAndLED(bScanCode);
	return TRUE;
}
