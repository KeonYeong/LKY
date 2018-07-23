#include "RTC.h"

// CMOS 메모리에서 RTC 컨트롤러가 저장해논 현재 시간을 읽는 함수
void kReadRTCTime(BYTE* pbHour, BYTE* pbMinute, BYTE* pbSecond){
	BYTE bData;

	// 시 저장하기
	// CMOS 메모리 어드레스 레지스터(포트는 0x70)에 시간이 저장되있는 레지스터를 지정한다
	kOutPortByte(RTC_CMOSADDRESS, RTC_ADDRESS_HOUR);
	// 이제 데이터 레지스터(포트 0x71)에서 읽는다
	bData = kInPortByte(RTC_CMOSDATA);

	*pbHour = RTC_BCDTOBINARY(bData); // CMOS에 전부 BCD방식으로 저장되어 있기 때문에 Binary 방식으로 바꿔준다
	
	// 분 저장하기
	// 같은 맥락
	kOutPortByte(RTC_CMOSADDRESS, RTC_ADDRESS_MINUTE);
	bData = kInPortByte(RTC_CMOSDATA);

	*pbMinute = RTC_BCDTOBINARY(bData);

	// 초 저장하기
	// 같은 맥락
	kOutPortByte(RTC_CMOSADDRESS, RTC_ADDRESS_SECOND);
	bData = kInPortByte(RTC_CMOSDATA);

	*pbSecond = RTC_BCDTOBINARY(bData);
}

// CMOS 메모리에서 현재 일자를 읽는 함수
void kReadRTCDate(WORD* pwYear, BYTE* pbMonth, BYTE* pbDayOfMonth, BYTE* pbDayOfWeek){
	BYTE bData;

	// 연도 저장하기
	// 시간 저장하는 것과 같은 원리
	kOutPortByte(RTC_CMOSADDRESS, RTC_ADDRESS_YEAR);
	bData = kInPortByte(RTC_CMOSDATA);

	*pwYear = RTC_BCDTOBINARY(bData) + 2000;

	// 월 저장하기
	kOutPortByte(RTC_CMOSADDRESS, RTC_ADDRESS_MONTH);
	bData = kInPortByte(RTC_CMOSDATA);

	*pbMonth = RTC_BCDTOBINARY(bData);

	// 일 저장하기
	kOutPortByte(RTC_CMOSADDRESS, RTC_ADDRESS_DAYOFMONTH);
	bData = kInPortByte(RTC_CMOSDATA);

	*pbDayOfMonth = RTC_BCDTOBINARY(bData);

	// 요일 저장하기
	kOutPortByte(RTC_CMOSADDRESS, RTC_ADDRESS_DAYOFWEEK);
	bData = kInPortByte(RTC_CMOSDATA);

	*pbDayOfWeek = RTC_BCDTOBINARY(bData);
}

// 요일을 실제 이름으로 바꿔주는 함수
char* kConvertDayOfWeekToString(BYTE bDayOfWeek){
	static char* vpcDayOfWeekString[8] = {"Error", "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

	// 요일 범위 초과시 에러
	if(bDayOfWeek >= 8) return vpcDayOfWeekString[0];

	return vpcDayOfWeekString[bDayOfWeek];
}
