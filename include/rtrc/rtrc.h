/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   rtrc.h
 * Author: roohi
 *
 * Created on August 10, 2019, 4:10 AM
 */

#pragma once
#include <thread>
#include <fstream>
#include "rstring.h"
#include "rlog.h"
#include <functional>
#include <mutex>
#include <exception>

#ifdef RTRC_CPP
extern bool _rtrcRawMouseMotionIsSupported = false;
extern const std::thread::id rtrc_main_thread_id = std::this_thread::get_id();
extern rtrc::logl::rlogger<false> mainLogger("main_log");

#else
extern bool _rtrcRawMouseMotionIsSupported;
extern const std::thread::id rtrc_main_thread_id;
extern rtrc::logl::rlogger<false> mainLogger;
#endif

#define SHADERS_FOLDER_PREFIX ""

static inline bool isThreadMain() { return std::this_thread::get_id() == rtrc_main_thread_id; }
static constexpr bool is_big_endian()
{
    union {
        uint32_t i;
        char c[4];
    } bint = {0x01020304};
    return bint.c[0] == 1; 
}

static inline bool rtrcRawMouseMotionIsSupported()
{
	return _rtrcRawMouseMotionIsSupported;
}

struct main_exec_queue
{
	static constexpr size_t max_queue_size = 1000;
	main_exec_queue(const main_exec_queue &) = delete;
	main_exec_queue(main_exec_queue &&other)
	{
		other.pullQueue();
	}
	main_exec_queue &operator=(const main_exec_queue &) = delete;
	main_exec_queue() noexcept
	{
#ifndef NDEBUG
		if(!isThreadMain())
			throw std::runtime_error("trying to call main_exec_queue() outside of main thread.");
#endif
		_commands.reserve(100);
	}
	
	/* append commands that need to be executed in the main thread.
	 * some system related library functions like glfw functions are required to
	 * execute in the main thread and cannot be put in the main thread directly.
	 * these functions are mainly going to free some dynamic resources.
	 */
	void appendCommand(const std::function<void()> &func)
	{
		std::lock_guard<std::mutex> lk(_gm);
		if(_commands.size() >= max_queue_size)
			throw std::runtime_error("insufficient main_exec_queue::max_queue_size");
		_commands.push_back(func);
	}
	
	void pullQueue()
	{
		mainLogger.logPerformance("calling main_exec_queue::pullQueue");
#ifndef NDEBUG
		if(!isThreadMain())
			throw std::runtime_error("trying to call main_exec_queue::pullQueue outside of main thread.");
#endif
		std::lock_guard<std::mutex> lk(_gm);
		// executing queue commands in the order of appending.
		for( const auto &func  : _commands )
		{
			func();
		}
		// emptying the queue.
		_commands.clear();
		mainLogger.logPerformance("main_exec_queue::pullQueue called");
	}
	
	~main_exec_queue()
	{
#ifndef NDEBUG
		if(!isThreadMain())
			throw std::runtime_error("trying to call ~main_exec_queue() outside of main thread.");
#endif
		pullQueue();
	}
private:
	
	std::mutex _gm;
	std::vector<std::function<void()>> _commands;
};

#ifdef RTRC_CPP
main_exec_queue main_thread_queue_handler = main_exec_queue();
#else
extern main_exec_queue main_thread_queue_handler;
#endif

int RTRC_MAIN(int argc, char *argv[]);


