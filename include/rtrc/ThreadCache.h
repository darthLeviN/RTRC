/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ThreadCache.h
 * Author: roohi
 *
 * Created on June 29, 2019, 3:35 PM
 */
#pragma once
#include <string>

namespace rtrc
{
    // can self log
    struct cachedLogStorage
    {
	struct StringAllocator // is responsible for flushing
	{
	    size_t fileID;
	};
	
	cachedLogStorage(); // reserves space for the log.
	cachedLogStorage(const cachedLogStorage &) = delete;
	cachedLogStorage & operator=(const cachedLogStorage &) = delete
	
	void writeRecord(const char *); 
	void flush();
	std::basic_string<wchar_t,StringAllocator> strBuff;
    };
	
	
    
    struct ThreadCache
    {
	
    };
}

