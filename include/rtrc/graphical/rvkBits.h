/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   rvkBits.h
 * Author: roohi
 *
 * Created on October 7, 2019, 6:45 PM
 */
#pragma once
#include "../rtrc.h"
namespace rtrc {
namespace vkl{
	

typedef double (*getDepthProc_t)(uint32_t/*texel*/);

// precision : 0.0001/1
static inline double getDepth_D24_UNORM_S8_UINT(uint32_t texel)
{
	uint32_t value;
	if(is_big_endian())
	{
		value = texel&0x00ffffffull;
	}
	else
	{
		value = texel&0xffffff00ull; // experimental, no little endian machine to test
	}
	// normalize
	return (double)value/(double)0x00ffffffull;
}

constexpr auto _createGetDepthProcs()
{
	std::array<getDepthProc_t,VK_FORMAT_ASTC_12x12_SRGB_BLOCK+1> retArr{};
	
	retArr[VK_FORMAT_D24_UNORM_S8_UINT] = getDepth_D24_UNORM_S8_UINT;
	
	return retArr;
}

static constexpr auto getDepthProcs = _createGetDepthProcs();
	


}
}
