/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   rthread.h
 * Author: roohi
 *
 * Created on July 25, 2019, 5:41 PM
 */
#include <thread>
#include <condition_variable>
#include <exception>
#include "rvtypes.h"
#pragma once
namespace rtrc { namespace threadl {
	
	template<proc_t<void,std::exception_ptr> finalExceptionHandler, proc_t<void,std::exception_ptr> initialexceptionHandler = [](std::exception_ptr){}>
	struct autocloseThread : std::thread
	{
		autocloseThread() noexcept {}
		autocloseThread( autocloseThread &&other) noexcept : std::thread(other) {}
		
		template<typename function, typename ...Args>
		explicit autocloseThread(function &&f, Args &&...args) : 
		std::thread([&excPtr](Args &&...args2)
		{
			try
			{
				f(std::move(args2)...);
			}
			catch(...)
			{
				excPtr = std::current_exception();
				initialexceptionHandler(excPtr);
			}
		}, std::move(args...)) {}
		autocloseThread( const autocloseThread &) = delete;
		~autocloseThread() noexcept {
			std::thread::join();
			finalExceptionHandler(excPtr);
		}
		std::exception_ptr excPtr;
	};
	
}}