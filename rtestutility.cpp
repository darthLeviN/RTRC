/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <rtrc/rtestutility.h>
#include <random>
#include <limits>
#include <stdio.h>
#include <chrono>
#include <memory>

using namespace rtrc;




TestUtil::ures128_t TestUtil::Arraysum(const uint64_t *data, uint32_t size) noexcept
{
	ures128_t res;
	constexpr uint64_t lowSectioner = (uint64_t) 0x80000000ull;
	for(uint32_t i = 0; i < size; ++i)
	{
		uint64_t lowPart = data[i] % lowSectioner;
		uint64_t highPart = data[i] / lowSectioner;
		res.low += lowPart;
		res.high += highPart;
	}
	res.high += res.low / lowSectioner;
	res.low = (res.low % lowSectioner)+((res.high % lowSectioner)* lowSectioner);
	res.high = res.high / lowSectioner;
	
	return res;
}

TestUtil::res128_t TestUtil::Arraysum(const int64_t *data, uint32_t size) noexcept
{
	constexpr int64_t lowSectioner = (int64_t) 0x80000000ull; // positive number
	//constexpr int64_t highSection = std::numeric_limits<int64_t>.max() - (int64_t)0xffffffffull; // this is a positive number
	if(std::numeric_limits<int32_t>::max() < size)
	{
		printf("ArraysumFailed, reason : array size bigger than int32_t::max");
		exit(1);
	}
	res128_t res;
	for(uint32_t i = 0; i < size; ++i)
	{
		int64_t lowPart = data[i] % lowSectioner;
		int64_t highPart = data[i] / lowSectioner;
		res.low += lowPart;
		res.high += highPart;
	}
	res.high += res.low / lowSectioner;
	res.low = (res.low % lowSectioner)+((res.high % lowSectioner )* lowSectioner);
	res.high = res.high / lowSectioner;
	
	return res;
}


uint64_t TestUtil::Arrayavg(const uint64_t* data, uint32_t size) noexcept
{
	
	uint64_t res = 0;
	uint64_t remain = 0;
	for(uint32_t i = 0; i < size; ++i)
	{
		res += data[i]/size;
		remain += data[i] % size;
		res += remain / size;
		remain %= size;
	}
	return res;
}

int64_t TestUtil::Arrayavg(const int64_t* data, uint32_t size) noexcept
{
	
	
	int64_t res = 0;
	int64_t remain = 0;
	for(uint32_t i = 0; i < size; ++i)
	{
		res += data[i]/size;
		remain += data[i] % size;
		res += remain / size;
		remain %= size;
	}
	return res;
}

/////////////////////////// TestUtil::randomGenerator

uint64_t TestUtil::randomGenerator::randomSeed() noexcept
{
	return std::chrono::high_resolution_clock::now().time_since_epoch().count();
}

TestUtil::randomGenerator::randomGenerator() noexcept
	: mt19_64(randomSeed()), 
	mt19(randomSeed())
{
	if(std::numeric_limits<std::uint64_t>::min() < mt19_64.min() || 
    std::numeric_limits<std::uint64_t>::max() > mt19_64.max())
	{
		printf("bad mt19937_64 limits min = %llu, max = %llu \n", mt19_64.min(), mt19_64.max());
		exit(1);
	}
	for(size_t i = 0; i < 956; ++i)
	{
		mt19_64();
	}
	if(std::numeric_limits<std::uint32_t>::min() < mt19.min() || 
    std::numeric_limits<std::uint32_t>::max() > mt19.max())
	{
		printf("bad mt19937 limits min = %llu, max = %llu \n", mt19.min(), mt19.max());
		exit(1);
	}
	for(size_t i = 0; i < 701; ++i)
	{
		mt19();
	}
}


uint64_t TestUtil::randomGenerator::genUInt64() noexcept
{   
	return mt19_64();
}

uint64_t TestUtil::randomGenerator::genUInt64(uint64_t min, uint64_t max) noexcept
{
	
	std::uniform_int_distribution<uint64_t> u64dist(min,
        max);
	return u64dist(mt19_64);
}

uint32_t TestUtil::randomGenerator::genUInt32() noexcept
{
	return mt19();
}
uint32_t TestUtil::randomGenerator::genUInt32(uint32_t min, uint32_t max) noexcept
{
	
	std::uniform_int_distribution<uint32_t> u32dist(min,
        max);
	return u32dist(mt19);
}



int64_t TestUtil::randomGenerator::genInt64() noexcept
{
    std::uniform_int_distribution<int64_t> i64dist(std::numeric_limits<int64_t>::min(),
        std::numeric_limits<int64_t>::max());
	return i64dist(mt19_64);
}

int64_t TestUtil::randomGenerator::genInt64(int64_t min, int64_t max) noexcept
{
	std::uniform_int_distribution<int64_t> i64dist(min,
        max);
	return i64dist(mt19_64);
}

int32_t TestUtil::randomGenerator::genInt32() noexcept
{
    std::uniform_int_distribution<int32_t> i32dist(std::numeric_limits<int32_t>::min(),
        std::numeric_limits<int32_t>::max());
	return i32dist(mt19);
}

int32_t TestUtil::randomGenerator::genInt32(int32_t min, int32_t max) noexcept
{
	std::uniform_int_distribution<int32_t> i32dist(min,
        max);
	return i32dist(mt19);
}

double TestUtil::randomGenerator::genDouble(double min, double max) noexcept
{
    if(min>=max) return 0.0;
    std::uniform_real_distribution<double> realdist(min,max);
    return realdist(mt19_64);
    
}

float TestUtil::randomGenerator::genFloat(float min, float max) noexcept
{
    if(min>=max) return 0.0f;
    std::uniform_real_distribution<float> realdist(min,max);
    return realdist(mt19_64);
}

char TestUtil::randomGenerator::genChar() noexcept
{
    std::uniform_int_distribution<> chardist(std::numeric_limits<char>::min(),
        std::numeric_limits<char>::max());
    return chardist(mt19_64);
}

wchar_t TestUtil::randomGenerator::genWChar() noexcept
{
    std::uniform_int_distribution<wchar_t> chardist(std::numeric_limits<wchar_t>::min(),
        std::numeric_limits<wchar_t>::max());
    return chardist(mt19_64);
}

char16_t TestUtil::randomGenerator::genChar16() noexcept
{
    std::uniform_int_distribution<> chardist(std::numeric_limits<char16_t>::min(),
        std::numeric_limits<char16_t>::max());
    return chardist(mt19_64);
}

char32_t TestUtil::randomGenerator::genChar32() noexcept
{
    std::uniform_int_distribution<char32_t> chardist(std::numeric_limits<char32_t>::min(),
        std::numeric_limits<char32_t>::max());
    return chardist(mt19_64);
}

bool TestUtil::randomGenerator::genBool(double trueChance) noexcept
{
    
    std::bernoulli_distribution bd(trueChance);
    return bd(mt19);
}


int TestUtil::randomGenerator::test(bool printOutput) noexcept
{
	/* developer notes :
	 * random generators with min and max argument remain untested.
	 */
	
	constexpr uint32_t sampleSize = 1024u*4u;
	printf("randomGenerator testing with a sample size of %llu\n", sampleSize);
	{
		//
		// signed types test
		//
		int64_t i64min = 0, i64max = 0;
		int64_t i64avg = 0;
		int64_t i64avgtheory = 0;
		int64_t i64diffOfavg = 0;
		int64_t * const i64a = (int64_t *)std::malloc(sampleSize*sizeof(int64_t));
		if(i64a == nullptr)
		{
			printf("could not allocate randomGenerator::test array\n");
			exit(1);
		}
		{
			//
			// genChar() test
			//
			for(uint32_t i = 0; i < sampleSize; ++i)
			{
				int64_t number = genChar();
				i64a[i] = number;
			}
			printf("\n");
			i64min = std::numeric_limits<char>::min();
			i64max = std::numeric_limits<char>::max();
			i64avg = Arrayavg(i64a,sampleSize);
			i64avgtheory = (i64min + i64max) / 2;
			i64diffOfavg = i64avg - i64avgtheory;
			printf("getChar() avgdiff= %lld, %f error \n", i64diffOfavg, 
				2.0* (double) i64diffOfavg/((double)i64max-(double)i64min));
		}
		{
			//
			// genWChar() test (note it may be signed or unsigned)
			//
			for(uint32_t i = 0; i < sampleSize; ++i)
			{
				int64_t number = genWChar();
				i64a[i] = number;
			}
			i64min = std::numeric_limits<int32_t>::min();
			i64max = std::numeric_limits<int32_t>::max();
			i64avg = Arrayavg(i64a,sampleSize);
			i64avgtheory = (i64min + i64max) / 2;
			i64diffOfavg = i64avg - i64avgtheory;
			printf("getWChar() avgdiff= %lld, %f error \n", i64diffOfavg, 
				2.0* (double) i64diffOfavg/((double)i64max-(double)i64min)/2.0);
		}
		{
			//
			// genInt32() test.
			//
			for(uint32_t i = 0; i < sampleSize; ++i)
			{
				int64_t number = genInt32();
				i64a[i] = number;
			}
			i64min = std::numeric_limits<int32_t>::min();
			i64max = std::numeric_limits<int32_t>::max();
			i64avg = Arrayavg(i64a,sampleSize);
			i64avgtheory = (i64min + i64max) / 2;
			i64diffOfavg = i64avg - i64avgtheory;
			printf("getInt32() avgdiff= %lld, %f error \n", i64diffOfavg, 
				2.0* (double) i64diffOfavg/((double)i64max-(double)i64min)/2.0);
		}
		{
			//
			// genInt64() test.
			//
			for(uint32_t i = 0; i < sampleSize; ++i)
			{
				int64_t number = genInt64();
				i64a[i] = number;
			}
			i64min = std::numeric_limits<int64_t>::min();
			i64max = std::numeric_limits<int64_t>::max();
			i64avg = Arrayavg(i64a,sampleSize);
			i64avgtheory = (i64min + i64max) / 2;
			i64diffOfavg = i64avg - i64avgtheory;
			printf("getInt64() avgdiff= %lld, %f error \n", i64diffOfavg, 
				2.0* (double) i64diffOfavg/((double)i64max-(double)i64min)/2.0);
		}
		std::free(i64a);
	}
	{
		//
		// UNSIGNED types test
		//
		uint64_t u64min = 0, u64max = 0;
		uint64_t u64avg = 0, u64avgtheory = 0, u64diffOfavg = 0;
		uint64_t *u64a = (uint64_t *)std::malloc(sampleSize*sizeof(uint64_t));
		if(u64a == nullptr)
		{
			printf("could not allocate randomGenerator::test array\n");
			exit(1);
		}
		{
			//
			// genChar16 test
			//
			for(uint32_t i = 0; i < sampleSize; ++i)
			{
				uint64_t number = genChar16();
				u64a[i] = number;
			}
			u64min = 0;//std::numeric_limits<char16_t>::min();
			u64max = std::numeric_limits<char16_t>::max();
			u64avg = Arrayavg(u64a,sampleSize);
			u64avgtheory = u64max / 2;
			u64diffOfavg = u64avg > u64avgtheory ? u64avg - u64avgtheory : u64avgtheory - u64avg;
			printf("getChar16() avgdiff= %llu, %f error \n", u64diffOfavg, 
				2.0* (double) u64diffOfavg/((double)u64max-(double)u64min)/2.0);
		}
		{
			//
			// genChar32 test
			//
			for(uint32_t i = 0; i < sampleSize; ++i)
			{
				uint64_t number = genChar32();
				u64a[i] = number;
			}
			u64min = 0;//std::numeric_limits<char32_t>::min();
			u64max = std::numeric_limits<char32_t>::max();
			u64avg = Arrayavg(u64a,sampleSize);
			u64avgtheory = u64max / 2;
			u64diffOfavg = u64avg > u64avgtheory ? u64avg - u64avgtheory : u64avgtheory - u64avg;
			printf("getChar32() avgdiff= %llu, %f error \n",u64diffOfavg, 
				2.0* (double) u64diffOfavg/((double)u64max-(double)u64min)/2.0);
		}
		{
			//
			// genUInt32 test
			//
			for(uint32_t i = 0; i < sampleSize; ++i)
			{
				uint64_t number = genUInt32();
				u64a[i] = number;
			}
			u64min = 0;//std::numeric_limits<uint32_t>::min();
			u64max = std::numeric_limits<uint32_t>::max();
			u64avg = Arrayavg(u64a,sampleSize);
			u64avgtheory = u64max/ 2;
			u64diffOfavg = u64avg > u64avgtheory ? u64avg - u64avgtheory : u64avgtheory - u64avg;
			printf("getUInt32() avgdiff= %llu, %f error \n", u64diffOfavg, 
				2.0* (double) u64diffOfavg/((double)u64max-(double)u64min)/2.0);
		}
		{
			//
			// genUInt64 test
			//
			for(uint32_t i = 0; i < sampleSize; ++i)
			{
				uint64_t number = genUInt64();
				u64a[i] = number;
			}
			u64min = 0;//std::numeric_limits<uint64_t>::min();
			u64max = std::numeric_limits<uint64_t>::max();
			u64avg = Arrayavg(u64a,sampleSize);
			u64avgtheory = u64max / 2;
			u64diffOfavg = u64avg > u64avgtheory ? u64avg - u64avgtheory : u64avgtheory - u64avg;
			printf("getUInt64() avgdiff= %llu, %f error \n", u64diffOfavg, 
				2.0* (double) u64diffOfavg/((double)u64max-(double)u64min)/2.0);
		}
		
		std::free(u64a);
	}
	return 0;
}

int TestUtil::test_RTestUtility(bool printOutput) noexcept
{
	TestUtil::randomGenerator rg;
	rg.test(printOutput);
	return 0;
}
