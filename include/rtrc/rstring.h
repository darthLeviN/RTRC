/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   RString.h
 * Author: roohi
 *
 * Created on June 28, 2019, 8:25 PM
 */

#pragma once
#include "compileOptions.h"
#include <iostream>
#include <string>
//#include "rmemory.h"
#include <memory>
#include <cstring>
#include <charconv>
#include <stdio.h>
#include "rexception.h"
//#include "rtestutility.h"
//#include "RRand.h"
#include <type_traits>
#include <cinttypes>
#include "compiletime.h"
namespace rtrc { namespace stringsl {
	
	

	
	enum txtEncs {
		ascii
	};
	
	// list of namespace members is as follows :
	template<typename charType = char, uint16_t bufferElCount = 0>
	struct Rsnprintf {}; // snprintf similar function with pre array allocation and error handling
	
	template<typename charType>
	struct nullTerminator {}; // generic nullterminator
	
	template<typename charType>
	struct emptyStr {}; // generic empty string
	
	template<typename charType>
	struct hashtagStr {}; // generic hashtag string "#"
	
	/* template<typename charType, uint16_t ArSize>
	 * struct nullTermedStrAr
	 * 
	 * pre allocated array with a special tag and direct member access. do the nulltermination manually.
	 * can be used at compile time.
	 */
	
	/* template<typename charType, uint16_t terminatedSize>
	 * constexpr nullTermedStrAr<char,terminatedSize> ctstr(const charType (&c_str)[terminatedSize])
	 * 
	 * convert char array or literal to nullTermedStrAr.
	 */
	
	/* template<typename charType>
	 * constexpr void strCopyWithMoveCur(charType *&cur, const charType *src, uint16_t size)
	 * 
	 * utility function used to copy to string while moving cur forward.
	 */
	
	/* template<typename charType, uint16_t leftS, uint16_t rightS>
	 * constexpr nullTermedStrAr<charType, leftS+rightS-1> operator+(
	 * const nullTermedStrAr<charType, leftS> leftstr, const nullTermedStrAr<charType, rightS> rightstr)
	 * 
	 * concatenates the two compile-time strings at compile time. can be used to concate more strings without any run-time overhead.
	 */
	
	/* template<typename charType, uint16_t ...sizes>
	 *		nullTermedStrAr<ctsum(sizes...)-sizeof...(sizes)+1, char> concat(
	 *		const nullTermedStrAr<sizes,charType>... nars)
	 * 
	 * concatenates any number of strings at run-time with a single allocation.
	 */
	
	template<typename charType>
	struct ctemptystr // 
	{
		// a nullTermedStrAr version of a empty string
		static constexpr auto value = ctstr(emptyStr<charType>::value);
	};
	
	/* converts a numT value to hex at run-time. use data and size to access string buffer. has a nullTermedStrAr base.
	 */
	template<typename numT, bool with0X = true, bool upper = true, bool fullLength = true, typename charType = char>
	struct toHex 
	{};
	
	template<>
	struct nullTerminator<char>
	{
		static constexpr char value = '\0';
	};
	
	template<>
	struct nullTerminator<wchar_t>
	{
		static constexpr wchar_t value = L'\0';
	};
	
	template<>
	struct nullTerminator<char16_t>
	{
		static constexpr char16_t value = u'\0';
	};
	
	template<>
	struct nullTerminator<char32_t>
	{
		static constexpr char32_t value = U'\0';
	};
	
	
	template<>
	struct emptyStr<char>
	{
		static constexpr char value[] = "";
	};
	
	template<>
	struct emptyStr<wchar_t>
	{
		static constexpr wchar_t value[] = L"";
	};
	
	template<>
	struct emptyStr<char16_t>
	{
		static constexpr char16_t value[] = u"";
	};
	
	template<>
	struct emptyStr<char32_t>
	{
		static constexpr char32_t value[] = U"";
	};
	
	template<>
	struct hashtagStr<char>
	{
		static constexpr char value[] = "#";
	};
	
	template<>
	struct hashtagStr<wchar_t>
	{
		static constexpr wchar_t value[] = L"#";
	};
	
	template<>
	struct hashtagStr<char16_t>
	{
		static constexpr char16_t value[] = u"#";
	};
	
	template<>
	struct hashtagStr<char32_t>
	{
		static constexpr char32_t value[] = U"#";
	};
	
	template<typename charType, uint16_t ArSize>
	struct nullTermedStrAr
	{
		/* difference of this class with std::array is :
		 * 1. has a different name to prevent confusion with non null Terminated Arrays, 
		 * although nullTermination should be done manually if implementing any constructors are avoided.
		 * 2. gives direct access to data and size members.
		 * 
		 * developer note : see if implementing constructors without array like initialization is possible.
		 */
		inline constexpr operator const char*() const { return data; }
		// this type is to avoid unsafe usage.
		static constexpr uint16_t size = ArSize;
		charType data[ArSize];
	};
	
	
	
	template<uint16_t bufferElCount>
	struct Rsnprintf<char, bufferElCount> : nullTermedStrAr<char, bufferElCount>
	{
		typedef nullTermedStrAr<char, bufferElCount> baseAr;
		using baseAr::data;
		using baseAr::size;
		template<typename ...Args>
		inline Rsnprintf(Args ...args) noexcept(false)
		{
			int ret = snprintf(data, size, args...);
			if(ret < 0)
				throw strFormatException("Rsnprintf encoding failed");
			//else if((size_t)ret + 1 > size)
				//throw shortbuffException("Rsnprintf called with a short buffer");
		}
		
	};
	
	template<uint16_t bufferElCount>
	struct Rsnprintf<wchar_t, bufferElCount> : nullTermedStrAr<wchar_t, bufferElCount>
	{
		using nullTermedStrAr<wchar_t,bufferElCount>::data;
		using nullTermedStrAr<wchar_t,bufferElCount>::size;
		template<typename ...Args>
		inline Rsnprintf(Args ...args) noexcept(false)
		{
			int ret = swprintf(data, size, args...);
			if(ret < 0)
				throw strFormatException("Rsnprintf encoding failed");
			//else if((size_t)ret + 1 > size)
				//throw shortbuffException("Rsnprintf called with a short buffer");
		}
	};
	
	
	template<typename charType, uint16_t terminatedSize>
	inline constexpr nullTermedStrAr<charType,terminatedSize> ctstr(const charType (&c_str)[terminatedSize])
	{
		nullTermedStrAr<charType,terminatedSize> retAr{};
		for(uint16_t i = 0; i < terminatedSize - 1; ++i)
		{
			retAr.data[i] = c_str[i];
		}
		retAr.data[terminatedSize - 1] = nullTerminator<charType>::value;
		return retAr;
	}
	
	template<typename charType>
	inline constexpr void strCopyWithMoveCur(charType *&cur, const charType *src, uint16_t size)
	{
		// used to be called multiple times to iterate through an arrray during compile time
		charType *endPtr = cur + size;
		while(cur != endPtr)
		{
			*cur = *src;
			++cur;
			++src;
		}
	}
	
	template<typename charType, uint16_t leftS, uint16_t rightS>
	inline constexpr nullTermedStrAr<charType,leftS+rightS-1> operator+(
		const nullTermedStrAr<charType,leftS> leftstr, const nullTermedStrAr<charType,rightS> rightstr)
	{
		nullTermedStrAr<charType,leftS+rightS-1> retAr{};
		charType *cur = retAr.data;
		strCopyWithMoveCur(cur, leftstr.data, leftS - 1);
		strCopyWithMoveCur(cur, rightstr.data, rightS - 1);
		*cur = nullTerminator<charType>::value;
		return retAr;
	}
	
	template<typename charType, uint16_t ...sizes>
	inline constexpr nullTermedStrAr<charType,ctsum(sizes...)-sizeof...(sizes)+1> concat(
		const nullTermedStrAr<charType,sizes> &... nars)
	{	
		constexpr uint16_t returnSize = ctsum(sizes...) - sizeof...(sizes) + 1; 
		constexpr auto sizesAr = ctArgArray(sizes...); // array of string sizes along with null terminators.
		auto cstrAr = ctArgArray(((const charType*)nars.data)...); // array of strings.
		nullTermedStrAr<charType,returnSize> retAr;
		uint16_t cur = 0;
		for(uint16_t i = 0; i < sizeof...(sizes); ++i)
		{
			std::memcpy( retAr.data + cur, cstrAr[i], sizesAr[i] != 0 ? sizesAr[i] - 1 : 0);
			cur += sizesAr[i] != 0 ? sizesAr[i] - 1 : 0;
		}
		retAr.data[returnSize-1] = '\0';
		return retAr;
	}
	
	template<typename charType, typename intertType>
	struct hexInsert
	{
	};
	
	template<>
	struct hexInsert<char, uint8_t>
	{
		// sizes are checked by builtintypecheck.h
		static constexpr auto upperIns = ctstr(PRIX8);
		static constexpr auto lowerIns = ctstr(PRIx8);
	};
	
	template<>
	struct hexInsert<char, uint16_t>
	{
		// sizes are checked by builtintypecheck.h
		static constexpr auto upperIns = ctstr(PRIX16);
		static constexpr auto lowerIns = ctstr(PRIx16);
	};
	
	template<>
	struct hexInsert<char, uint32_t>
	{
		// sizes are checked by builtintypecheck.h
		static constexpr auto upperIns = ctstr(PRIX32);
		static constexpr auto lowerIns = ctstr(PRIx32);
	};
	
	template<>
	struct hexInsert<char, uint64_t>
	{
		// sizes are checked by builtintypecheck.h
		static constexpr auto upperIns = ctstr(PRIX64);
		static constexpr auto lowerIns = ctstr(PRIx64);
	};
	
	template<typename insertType>
	struct hexInsert<wchar_t, insertType>
	{
		// sizes are checked by builtintypecheck.h
		static constexpr auto upperIns = ctstr(L"llX");
		static constexpr auto lowerIns = ctstr(L"llx");
	};
	
	
	
	template<typename charType, typename intType, bool full>
	struct hexPrecInsert
	{
	};
	
	template<typename charType, typename intType>
	struct hexPrecInsert<charType,intType,false>
	{
		static constexpr auto precIns = ctemptystr<charType>::value;
	};
	
	
	template<>
	struct hexPrecInsert<char,uint8_t, true>
	{
		static constexpr auto precIns = ctstr(".2");
	};
	
	template<>
	struct hexPrecInsert<char,uint16_t, true>
	{
		static constexpr auto precIns = ctstr(".4");
	};
	
	template<>
	struct hexPrecInsert<char,uint32_t, true>
	{
		static constexpr auto precIns = ctstr(".8");
	};
	
	template<>
	struct hexPrecInsert<char,uint64_t,true>
	{
		static constexpr auto precIns = ctstr(".16");
	};
	
	template<>
	struct hexPrecInsert<wchar_t,uint8_t, true>
	{
		static constexpr auto precIns = ctstr(L".2");
	};
	
	template<>
	struct hexPrecInsert<wchar_t,uint16_t, true>
	{
		static constexpr auto precIns = ctstr(L".4");
	};
	
	template<>
	struct hexPrecInsert<wchar_t,uint32_t, true>
	{
		static constexpr auto precIns = ctstr(L".8");
	};
	
	template<>
	struct hexPrecInsert<wchar_t,uint64_t,true>
	{
		static constexpr auto precIns = ctstr(L".16");
	};
	
	
	
	template<typename charType, bool alt /*= false*/>
	struct hashTagInsert
	{
		static constexpr auto hashtagIns = ctemptystr<charType>::value;
	};
	
	
	template<typename charType>
	struct hashTagInsert<charType, true>
	{
		static constexpr auto hashtagIns = ctstr(hashtagStr<charType>::value);
	};
	
	template<typename numT, bool with0X, bool upper, bool fullLength>
	struct toHex<numT,with0X,upper,fullLength, char> /*the order sensitive*/ : hexInsert<char,numT>, hexPrecInsert<char,numT,fullLength>, 
		hashTagInsert<char,with0X>, Rsnprintf<char,(with0X ? 2:0)+sizeof(numT)*2+1>
	{
		static constexpr uint16_t buffElCount = (with0X ? 2:0)+sizeof(numT)*2+1;
		typedef hexInsert<char,numT> Inserter;
		typedef hexPrecInsert<char,numT,fullLength> precInserter;
		typedef hashTagInsert<char,with0X> hashtagInserter;
		typedef Rsnprintf<char,(with0X ? 2:0)+sizeof(numT)*2+1> printer;
		using Inserter::lowerIns;
		using Inserter::upperIns;
		using precInserter::precIns;
		using hashtagInserter::hashtagIns;
		using printer::data;
		using printer::size;
		static_assert(std::is_unsigned<numT>::value, "current version only supports unsigned types");
		
		inline toHex(numT number) noexcept(false)
			: printer(ctstr("%") + hashtagIns + precIns + (upper ? upperIns : lowerIns),number)
		{
			//printf(ctstr("%") + hashtagIns + precIns + (upper ? upperIns : lowerIns) + ctstr("\n"),number);
			//try{
			//throw new RbasicException("eh");//}
			//catch(...) {}
		}	
	};
	
	
	
}}