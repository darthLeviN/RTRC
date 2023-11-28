/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   typeChecks.h
 * Author: roohi
 *
 * Created on July 11, 2019, 7:22 PM
 */
#pragma once

#include <limits>
#include <stdint.h>

namespace rtrc
{
namespace tpyeCheck
{
	
	// size checks
	static_assert(sizeof(char) == 1, "bad limits");
	static_assert(sizeof(wchar_t) <= 4 , "bad limits");
	static_assert(sizeof(char16_t) == 2, "bad limits");
	static_assert(sizeof(char32_t) == 4, "bad limits");
	static_assert(sizeof(int64_t) == 8, "bad limits");
	static_assert(sizeof(uint64_t) == 8, "bad limits");
	static_assert(sizeof(int32_t) == 4, "bad limits");
	static_assert(sizeof(uint32_t) == 4, "bad limits");
	static_assert(sizeof(int16_t) == 2, "bad limits");
	static_assert(sizeof(uint16_t) == 2, "bad limits");
	static_assert(sizeof(int8_t) == 1, "bad limits");
	static_assert(sizeof(uint8_t) == 1, "bad limits");
	static_assert(sizeof(int)<=sizeof(size_t), "bad limits");
	static_assert(sizeof(int)>sizeof(uint16_t), "bad limits");
	
	// limit checks 
	static_assert(std::numeric_limits<char16_t>::min() == 0, "bad limits");
	static_assert(std::numeric_limits<char32_t>::min() == 0, "bad limits");
	static_assert(std::numeric_limits<uint64_t>::min() == 0, "bad limits");
	static_assert(std::numeric_limits<uint32_t>::min() == 0, "bad limits");
	static_assert(std::numeric_limits<uint16_t>::min() == 0, "bad limits");
	static_assert(std::numeric_limits<uint8_t>::min() == 0, "bad limits");
	static_assert(std::numeric_limits<size_t>::min() == 0, "bad limits");
}
}