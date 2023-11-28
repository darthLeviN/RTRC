/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   RTestUtility.h
 * Author: roohi
 *
 * Created on July 4, 2019, 2:23 PM
 */

#pragma once
#include <cstddef>
#include <cstdint>
#include <random>
#include <array>
#include <limits>
#include <memory>
#include <chrono>
#include <thread>
#include "rvtypes.h"
#include "compiletime.h"
namespace rtrc
{
	
namespace TestUtil
{
// used for return based informing of error
	


enum RErrorCodes : int
{
	ok = 0, badRandomGenerator, badExceptionLibrary
};

template<typename elT,uint32_t S>
struct TArray
{
	elT data[S];
	const uint32_t size = S;
};

template<typename elT>
struct TDArray
{
	TDArray() = delete;
	TDArray(const TDArray<elT> &) = delete;
	TDArray(TDArray<elT> &&) = delete;
	TDArray(uint32_t s) : size(s), data((elT *)malloc(s*sizeof(elT)))
	{
		if(data == nullptr)
		{
			printf("failed to initialize TDArray\n");
			exit(1);
		}
	}
	~TDArray() {std::free(data);}
	const uint32_t size;
	elT * const data;
};

// test rational, non efficient, with a more clear and readable code.
struct TRational
{
	TRational() = delete;
    
    
	// assigns a rational number in the [lowerLimit,upperLimit] range. cannot fail.
	TRational(int32_t numerator, uint32_t denominator) noexcept : num(numerator), den(denominator) {};
	const int num;
	const uint32_t den;
};

struct res128_t
{
	res128_t() : low(0), high(0) {}
	int64_t low;
	int64_t high;
};

struct ures128_t
{
	ures128_t() : low(0), high(0) {}
	uint64_t low;
	uint64_t high;
};


ures128_t Arraysum(const uint64_t *data, uint32_t size) noexcept;


res128_t Arraysum(const int64_t *data, uint32_t size) noexcept;

uint64_t Arrayavg(const uint64_t *data, uint32_t size) noexcept;


int64_t Arrayavg(const int64_t *data, uint32_t size) noexcept;



// generates any random generated sequence for testing purposes.
struct randomGenerator
{
	static uint64_t randomSeed() noexcept;
    randomGenerator() noexcept;
	
    uint64_t genUInt64() noexcept;
	uint64_t genUInt64(uint64_t min, uint64_t max) noexcept;
    uint32_t genUInt32() noexcept;
	uint32_t genUInt32(uint32_t min, uint32_t max) noexcept;
	int64_t genInt64() noexcept;
	int64_t genInt64(int64_t min, int64_t max) noexcept;
	int32_t genInt32() noexcept;
	int32_t genInt32(int32_t min, int32_t max) noexcept;
	double genDouble(double min, double max) noexcept; // generates a random double.
	float genFloat(float min, float max) noexcept; // generates a random float.
	char genChar() noexcept; // generates a random char.
	wchar_t genWChar() noexcept; // generates a random wchar_t.
	char16_t genChar16() noexcept; // generates a random char16_t.
	char32_t genChar32() noexcept; // generates a random char32_t.
	// generates a random bool with trueChance for it to be true.
	bool genBool(double trueChance) noexcept; 

	// note on string generators : characters or sequences like '\r', '\33[2k', '\033[A may erase the console.
	
	// nulltermChance : the minimum chance for each character to be '\0'
	// uses 'char' type
	template<uint32_t strLength>
	TArray<char,strLength> genCharString(double nulltermChance) noexcept
	{
		TArray<char,strLength> retAr;
		for(uint32_t i = 0; i < strLength; ++i)
		{
			if(genBool(nulltermChance))
			{
				retAr.data[i] = '\0';
			}
			else
			{
				retAr.data[i] = genChar();
			}
		}
		return retAr;
	}

	// nulltermChance : the minimum chance for each character to be L'\0'
	// uses 'wchar_t' type. uses 'L' prefix for character literals.
	template<uint32_t strLength>
	TArray<wchar_t,strLength> genWCharString(double nulltermChance) noexcept
	{
		TArray<wchar_t,strLength> retAr;
		for(size_t i = 0; i < strLength; ++i)
		{
			if(genBool(nulltermChance))
			{
				retAr.data[i] = L'\0';
			}
			else
			{
				retAr.data[i] = genWChar();
			}
		}
		return retAr;
	}

	// nulltermChance : the minimum chance for each character to be u'\0'
	// uses 'char16_t' type. uses 'u' prefix for character literals.
	template<uint32_t strLength>
	TArray<char16_t,strLength> genChar16String(double nulltermChance) noexcept
	{
		TArray<char16_t,strLength> retAr;
		for(size_t i = 0; i < strLength; ++i)
		{
			if(genBool(nulltermChance))
			{
				retAr.data[i] = u'\0';
			}
			else
			{
				retAr.data[i] = genChar16();
			}
		}
		return retAr;
	}

	// nulltermChance : the minimum chance for each character to be U'\0'
	// uses 'char32_t' type. uses 'U' prefux for character literals.
	template<uint32_t strLength>
	TArray<char32_t,strLength> genChar32String(double nulltermChance) noexcept
	{
		TArray<char32_t,strLength> retAr;
		for(uint32_t i = 0; i < strLength; ++i)
		{
			if(genBool(nulltermChance))
			{
				retAr.data[i] = U'\0';
			}
			else
			{
				retAr.data[i] = genChar32();
			}
		}
		return retAr;
	}
	
	std::mt19937_64 mt19_64;
    std::mt19937_64 mt19;
    
	int test(bool printOutput) noexcept;
};

template<typename chronoDurationT>
struct oneSecond;

template<>
struct oneSecond<std::chrono::nanoseconds>
{
	static constexpr int64_t value = 1000*1000*1000;
};

template<>
struct oneSecond<std::chrono::microseconds>
{
	static constexpr int64_t value = 1000*1000;
};

template<>
struct oneSecond<std::chrono::milliseconds>
{
	static constexpr int64_t value = 1000;
};

template<>
struct oneSecond<std::chrono::seconds>
{
	static constexpr int64_t value = 1;
};

struct functionTester
{
	functionTester(voidproc_t p) noexcept : proc(p) {};
	
	
	template<typename chronoDurationT = std::chrono::nanoseconds> 
	static void /*__attribute__((optimize("O0")))*/ _run(functionTester *ft, size_t times, int64_t *ret) noexcept
	{
		
		auto startTime = std::chrono::high_resolution_clock::now();
		if(ft->proc)
		for(size_t i = 0;i < times; ++i)
		{
			ft->proc();
		}
		auto endTime = std::chrono::high_resolution_clock::now();
		*ret = std::chrono::duration_cast<chronoDurationT>(endTime - startTime).count();
		 
	}
	
	
	template<typename chronoDurationT = std::chrono::nanoseconds> 
	int64_t run(size_t times) noexcept
	{
		int64_t ret;
		_run(this, times, &ret);
		return ret;
	}
	
	template<typename chronoDurationT = std::chrono::nanoseconds>
	int64_t antiStallRun(size_t times)
	{
		int64_t ret;
		std::thread th(_run<chronoDurationT>,this,times,&ret);
		th.join();
		return ret;
	}
	
	double ASgetReturnFrequency(size_t times)
	{
		double runValue = antiStallRun(times);
		return (1.0e9/runValue)*times;
	}
	
	double getReturnFrequency(size_t times)
	{
		double runValue = run(times);
		return (1.0e9/runValue)*times;
	}
	
	voidproc_t proc;
};

template<typename ...Args>
struct tinvokers
{
template<proc_t<void,Args...> ...procs>
struct randomInvoker
{
	/* other forms for a non compile time call to ctArgArray is possible.
	 */
	randomInvoker()
		: funcs(ctArgArray<proc_t<void,Args...>>(procs...))
	{
	}
	
	/* runs a function by picking one with a equal chance for all.
	 */
	void run(randomGenerator &gen, Args... args)
	{
		funcs[gen.genUInt32(0,sizeof...(procs)-1)](args...);
	}
	
	static void _run(randomInvoker<procs...> *th, randomGenerator *gen, Args... args)
	{
		th->run(*gen,args...);
	}
	
	/* calls run(args) N times in separate threads.
	 */
	template<size_t N>
	void multiThreadRun(randomGenerator &gen, Args... args)
	{
		std::thread threads[N];
		for(size_t i = 0; i < N; ++i)
		{
			threads[i] = std::thread(_run,this,&gen,args...);
		}
		for(size_t i = 0; i < N; ++i)
		{
			threads[i].join();
		}
	}
	std::array<proc_t<void,Args...>,sizeof...(procs)> funcs;
};};


int test_RTestUtility(bool printOutput) noexcept;
}
}