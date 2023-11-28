/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "RTRC/builtintypecheck.h"

using namespace RTRC::tpyeCheck;

////////////////////////////////////////// char -> x ////////////////////////////////////////
wchar_t chartowchar(char value ) noexcept
{
	return (wchar_t)value;
}
char16_t chartochar16(char value ) noexcept
{
	return (char16_t)value;
}
char32_t chartochar32(char value ) noexcept
{
	return (char32_t)value;
}
int8_t chartoi8(char value ) noexcept
{
	return (int8_t)value;
}
uint8_t chartou8(char value ) noexcept
{
	return (uint8_t) value;
}
int16_t chartoi16(char value ) noexcept
{
	return (int16_t) value;
}
uint16_t chartou16(char value ) noexcept
{
	return (uint16_t) value;
}
int32_t chartoi32(char value ) noexcept
{
	return (int32_t) value;
}
uint32_t chartou32(char value ) noexcept
{
	return (uint32_t) value;
}
int64_t chartoi64(char value ) noexcept
{
	return (int64_t) value;
}
uint64_t chartou64(char value ) noexcept
{
	return (uint64_t) value;
}

////////////////////////////////////////// wchar_t -> x ////////////////////////////////////////
char wchartochar(wchar_t value ) noexcept
{
	return (char) value;
}
char16_t wchartochar16(wchar_t value ) noexcept
{
	return (char16_t)value;
}
char32_t wchartochar32(wchar_t value ) noexcept
{
	return (char32_t)value;
}
int8_t wchartoi8(wchar_t value ) noexcept
{
	return (int8_t)value;
}
uint8_t wchartou8(wchar_t value ) noexcept
{
	return (uint8_t) value;
}
int16_t wchartoi16(wchar_t value ) noexcept
{
	return (int16_t) value;
}
uint16_t wchartou16(wchar_t value ) noexcept
{
	return (uint16_t) value;
}
int32_t wchartoi32(wchar_t value ) noexcept
{
	return (int32_t) value;
}
uint32_t wchartou32(wchar_t value ) noexcept
{
	return (uint32_t) value;
}
int64_t wchartoi64(wchar_t value ) noexcept
{
	return (int64_t) value;
}
uint64_t wchartou64(wchar_t value ) noexcept
{
	return (uint64_t) value;
}

////////////////////////////////////////// char16_t -> x ////////////////////////////////////////
char char16tochar(char16_t value ) noexcept
{
	return (char) value;
}
wchar_t char16towchar(char16_t value ) noexcept
{
	return (wchar_t)value;
}
char32_t char16tochar32(char16_t value ) noexcept
{
	return (char32_t)value;
}
int8_t char16toi8(char16_t value ) noexcept
{
	return (int8_t)value;
}
uint8_t char16tou8(char16_t value ) noexcept
{
	return (uint8_t) value;
}
int16_t char16toi16(char16_t value ) noexcept
{
	return (int16_t) value;
}
uint16_t char16tou16(char16_t value ) noexcept
{
	return (uint16_t) value;
}
int32_t char16toi32(char16_t value ) noexcept
{
	return (int32_t) value;
}
uint32_t char16tou32(char16_t value ) noexcept
{
	return (uint32_t) value;
}
int64_t char16toi64(char16_t value ) noexcept
{
	return (int64_t) value;
}
uint64_t char16tou64(char16_t value ) noexcept
{
	return (uint64_t) value;
}

////////////////////////////////////////// char32_t -> x ////////////////////////////////////////
char char32tochar(char32_t value ) noexcept
{
	return (char) value;
}
wchar_t char32towchar(char32_t value ) noexcept
{
	return (wchar_t)value;
}
char16_t char32tochar16(char32_t value ) noexcept
{
	return (char16_t)value;
}
int8_t char32toi8(char32_t value ) noexcept
{
	return (int8_t)value;
}
uint8_t char32tou8(char32_t value ) noexcept
{
	return (uint8_t) value;
}
int16_t char32toi16(char32_t value ) noexcept
{
	return (int16_t) value;
}
uint16_t char32tou16(char32_t value ) noexcept
{
	return (uint16_t) value;
}
int32_t char32toi32(char32_t value ) noexcept
{
	return (int32_t) value;
}
uint32_t char32tou32(char32_t value ) noexcept
{
	return (uint32_t) value;
}
int64_t char32toi64(char32_t value ) noexcept
{
	return (int64_t) value;
}
uint64_t char32tou64(char32_t value ) noexcept
{
	return (uint64_t) value;
}

////////////////////////////////////////// i8 -> x ////////////////////////////////////////
char i8tochar(int8_t value ) noexcept
{
	return (char) value;
}
wchar_t i8towchar(int8_t value ) noexcept
{
	return (wchar_t)value;
}
char16_t i8tochar16(int8_t value ) noexcept
{
	return (char16_t)value;
}
char32_t i8tochar32(int8_t value ) noexcept
{
	return (char32_t)value;
}
uint8_t i8tou8(int8_t value ) noexcept
{
	return (uint8_t) value;
}
int16_t i8toi16(int8_t value ) noexcept
{
	return (int16_t) value;
}
uint16_t i8tou16(int8_t value ) noexcept
{
	return (uint16_t) value;
}
int32_t i8toi32(int8_t value ) noexcept
{
	return (int32_t) value;
}
uint32_t i8tou32(int8_t value ) noexcept
{
	return (uint32_t) value;
}
int64_t i8toi64(int8_t value ) noexcept
{
	return (int64_t) value;
}
uint64_t i8tou64(int8_t value ) noexcept
{
	return (uint64_t) value;
}

////////////////////////////////////////// u8 -> x ////////////////////////////////////////
char u8tochar(uint8_t value ) noexcept
{
	return (char) value;
}
wchar_t u8towchar(uint8_t value ) noexcept
{
	return (wchar_t)value;
}
char16_t u8tochar16(uint8_t value ) noexcept
{
	return (char16_t)value;
}
char32_t u8tochar32(uint8_t value ) noexcept
{
	return (char32_t)value;
}
int8_t u8toi8(uint8_t value ) noexcept
{
	return (int8_t)value;
}
int16_t u8toi16(uint8_t value ) noexcept
{
	return (int16_t) value;
}
uint16_t u8tou16(uint8_t value ) noexcept
{
	return (uint16_t) value;
}
int32_t u8toi32(uint8_t value ) noexcept
{
	return (int32_t) value;
}
uint32_t u8tou32(uint8_t value ) noexcept
{
	return (uint32_t) value;
}
int64_t u8toi64(uint8_t value ) noexcept
{
	return (int64_t) value;
}
uint64_t u8tou64(uint8_t value ) noexcept
{
	return (uint64_t) value;
}

////////////////////////////////////////// i16 -> x ////////////////////////////////////////
char i16tochar(int16_t value ) noexcept
{
	return (char) value;
}
wchar_t i16towchar(int16_t value ) noexcept
{
	return (wchar_t)value;
}
char16_t i16tochar16(int16_t value ) noexcept
{
	return (char16_t)value;
}
char32_t i16tochar32(int16_t value ) noexcept
{
	return (char32_t)value;
}
int8_t i16toi8(int16_t value ) noexcept
{
	return (int8_t)value;
}
uint8_t i16tou8(int16_t value ) noexcept
{
	return (uint8_t) value;
}
uint16_t i16tou16(int16_t value ) noexcept
{
	return (uint16_t) value;
}
int32_t i16toi32(int16_t value ) noexcept
{
	return (int32_t) value;
}
uint32_t i16tou32(int16_t value ) noexcept
{
	return (uint32_t) value;
}
int64_t i16toi64(int16_t value ) noexcept
{
	return (int64_t) value;
}
uint64_t i16tou64(int16_t value ) noexcept
{
	return (uint64_t) value;
}

////////////////////////////////////////// u16 -> x ////////////////////////////////////////
char u16tochar(uint16_t value ) noexcept
{
	return (char) value;
}
wchar_t u16towchar(uint16_t value ) noexcept
{
	return (wchar_t)value;
}
char16_t u16tochar16(uint16_t value ) noexcept
{
	return (char16_t)value;
}
char32_t u16tochar32(uint16_t value ) noexcept
{
	return (char32_t)value;
}
int8_t u16toi8(uint16_t value ) noexcept
{
	return (int8_t)value;
}
uint8_t u16tou8(uint16_t value ) noexcept
{
	return (uint8_t) value;
}
int16_t u16toi16(uint16_t value ) noexcept
{
	return (int16_t) value;
}
int32_t u16toi32(uint16_t value ) noexcept
{
	return (int32_t) value;
}
uint32_t u16tou32(uint16_t value ) noexcept
{
	return (uint32_t) value;
}
int64_t u16toi64(uint16_t value ) noexcept
{
	return (int64_t) value;
}
uint64_t u16tou64(uint16_t value ) noexcept
{
	return (uint64_t) value;
}

////////////////////////////////////////// i32 -> x ////////////////////////////////////////
char i32tochar(int32_t value) noexcept
{
	return (char) value;
}
wchar_t i32towchar(int32_t value) noexcept
{
	return (wchar_t)value;
}
char16_t i32tochar16(int32_t value) noexcept
{
	return (char16_t)value;
}
char32_t i32tochar32(int32_t value) noexcept
{
	return (char32_t)value;
}
int8_t i32toi8(int32_t value) noexcept
{
	return (int8_t)value;
}
uint8_t i32tou8(int32_t value) noexcept
{
	return (uint8_t) value;
}
int16_t i32toi16(int32_t value) noexcept
{
	return (int16_t) value;
}
uint16_t i32tou16(int32_t value ) noexcept
{
	return (uint16_t) value;
}
uint32_t i32tou32(int32_t value ) noexcept
{
	return (uint32_t) value;
}
int64_t i32toi64(int32_t value ) noexcept
{
	return (int64_t) value;
}
uint64_t i32tou64(int32_t value ) noexcept
{
	return (uint64_t) value;
}

////////////////////////////////////////// u32 -> x ////////////////////////////////////////
char u32tochar(uint32_t value) noexcept
{
	return (char) value;
}
wchar_t u32towchar(uint32_t value) noexcept
{
	return (wchar_t)value;
}
char16_t u32tochar16(uint32_t value) noexcept
{
	return (char16_t)value;
}
char32_t u32tochar32(uint32_t value) noexcept
{
	return (char32_t)value;
}
int8_t u32toi8(uint32_t value) noexcept
{
	return (int8_t)value;
}
uint8_t u32tou8(uint32_t value) noexcept
{
	return (uint8_t) value;
}
int16_t u32toi16(uint32_t value) noexcept
{
	return (int16_t) value;
}
uint16_t u32tou16(uint32_t value ) noexcept
{
	return (uint16_t) value;
}
int32_t u32toi32(uint32_t value ) noexcept
{
	return (int32_t) value;
}
int64_t u32toi64(uint32_t value ) noexcept
{
	return (int64_t) value;
}
uint64_t u32tou64(uint32_t value ) noexcept
{
	return (uint64_t) value;
}

////////////////////////////////////////// i64 -> x ////////////////////////////////////////
char i64tochar(int64_t value) noexcept
{
	return (char) value;
}
wchar_t i64towchar(int64_t value) noexcept
{
	return (wchar_t)value;
}
char16_t i64tochar16(int64_t value) noexcept
{
	return (char16_t)value;
}
char32_t i64tochar32(int64_t value) noexcept
{
	return (char32_t)value;
}
int8_t i64toi8(int64_t value) noexcept
{
	return (int8_t)value;
}
uint8_t i64tou8(int64_t value) noexcept
{
	return (uint8_t) value;
}
int16_t i64toi16(int64_t value) noexcept
{
	return (int16_t) value;
}
uint16_t i64tou16(int64_t value ) noexcept
{
	return (uint16_t) value;
}
int32_t i64toi32(int64_t value ) noexcept
{
	return (int32_t) value;
}
uint32_t i64tou32(int64_t value ) noexcept
{
	return (uint32_t) value;
}
uint64_t i64tou64(int64_t value ) noexcept
{
	return (uint64_t) value;
}

////////////////////////////////////////// u64 -> x ////////////////////////////////////////
char u64tochar(uint64_t value) noexcept
{
	return (char) value;
}
wchar_t u64towchar(uint64_t value) noexcept
{
	return (wchar_t) value;
}
char16_t u64tochar16(uint64_t value) noexcept
{
	return (char16_t)value;
}
char32_t u64tochar32(uint64_t value) noexcept
{
	return (char32_t)value;
}
int8_t u64toi8(uint64_t value) noexcept
{
	return (int8_t)value;
}
uint8_t u64tou8(uint64_t value) noexcept
{
	return (uint8_t) value;
}
int16_t u64toi16(uint64_t value) noexcept
{
	return (int16_t) value;
}
uint16_t u64tou16(uint64_t value ) noexcept
{
	return (uint16_t) value;
}
int32_t u64toi32(uint64_t value ) noexcept
{
	return (int32_t) value;
}
uint32_t u64tou32(uint64_t value ) noexcept
{
	return (uint32_t) value;
}
int64_t u64toi64(uint64_t value ) noexcept
{
	return (int64_t) value;
}