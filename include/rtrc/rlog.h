/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   RLog.h
 * Author: roohi
 *
 * Created on June 28, 2019, 2:42 PM
 */
#pragma once
#include <memory>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <fstream>
#include <array>
#include "rmemory.h"
#include "rstring.h"
#include <mutex>
#include <GLFW/glfw3.h>
#include <thread>

using namespace rtrc::stringsl;
namespace rtrc { namespace logl {
	
	//static rtrc::stringsl::Rsnprintf<char,100> rtrc_logs_dir("%s%s", getenv("userprofile"), "/logs");
	static constexpr char rtrc_logs_dir[] = "logs";
    // used for tagging/naming important objects 
	template<bool debuggingMutex = true>
	struct rlogger
	{
		rlogger() = delete;
		rlogger(const rlogger &) = delete;
		rlogger(rlogger &&) = delete;
		rlogger &operator=(const rlogger &) = delete;
		
		rlogger(const char *name)
			: logFile1(rtrc::stringsl::Rsnprintf<char,100>("%s/%s_1.log",rtrc_logs_dir, name).data),
			logFile2(rtrc::stringsl::Rsnprintf<char,100>("%s/%s_2.log",rtrc_logs_dir, name).data),
			ioFile(rtrc::stringsl::Rsnprintf<char,100>("%s/%s_io.log",rtrc_logs_dir, name).data),
			mlkFile(rtrc::stringsl::Rsnprintf<char,100>("%s/%s_mutex_lock.log",rtrc_logs_dir, name).data),
			varLogFile(rtrc::stringsl::Rsnprintf<char,100>("%s/%s_vars.log",rtrc_logs_dir, name).data),
			performanceFile(rtrc::stringsl::Rsnprintf<char,100>("%s/%s_performance.log",rtrc_logs_dir, name).data)
		{
			
		}
		
		void logText(const char *text)
		{
			std::lock_guard<std::mutex> lk(logfile1M);
			logFile1 << text << rtrc::stringsl::Rsnprintf<char, 40>("(time: %f, thread_id: ", glfwGetTime()).data <<
				std::this_thread::get_id() << ")\n";
			logFile1.flush();
		}
		
		void logPerformance(const char *text)
		{
			// dev note : create a higher performance function.
			std::lock_guard<std::mutex> lk(performanceFileM);
			performanceFile << text << rtrc::stringsl::Rsnprintf<char, 40>("(time: %f, thread_id: ", glfwGetTime()).data <<
				std::this_thread::get_id() << ")\n";
			//logFile1.flush(); no flushing.
		}
		
		void logTraffic_GPUtoCPU(const char *text)
		{
			std::lock_guard<std::mutex> lk(traffic_GPUtoCPUFileM);
			traffic_GPUtoCPUFile << text << rtrc::stringsl::Rsnprintf<char, 40>("(time: %f, thread_id: ", glfwGetTime()).data <<
				std::this_thread::get_id() << ")\n";
		}
		
		void logTraffic_CPUtoGPU(const char *text)
		{
			std::lock_guard<std::mutex> lk(traffic_CPUtoGPUFileM);
			traffic_CPUtoGPUFile << text << rtrc::stringsl::Rsnprintf<char, 40>("(time: %f, thread_id: ", glfwGetTime()).data <<
				std::this_thread::get_id() << ")\n";
		}
		
		void logTraffic_generic(const char *text)
		{
			std::lock_guard<std::mutex> lk(traffic_genericFileM);
			traffic_genericFile << text << rtrc::stringsl::Rsnprintf<char, 40>("(time: %f, thread_id: ", glfwGetTime()).data <<
				std::this_thread::get_id() << ")\n";
		}

		void logText2(const char *text)
		{
			std::lock_guard<std::mutex> lk(logfile2M);
			logFile2 << text << rtrc::stringsl::Rsnprintf<char, 40>("(time: %f, thread_id: ", glfwGetTime()).data <<
				std::this_thread::get_id() << ")\n";
			logFile2.flush();
		}
		
		void logIo(const char *text)
		{
			std::lock_guard<std::mutex> lk(ioFileM);
			ioFile << text << rtrc::stringsl::Rsnprintf<char, 40>("(time: %f, thread_id: ", glfwGetTime()).data <<
				std::this_thread::get_id() << ")\n";
			ioFile.flush();
		}
		
		void logMutexLock(const char *text)
		{
			if(debuggingMutex)
			{
				std::lock_guard<std::mutex> lk(mlkFileM);
				mlkFile << text << rtrc::stringsl::Rsnprintf<char, 40>("(time: %f, thread_id: ", glfwGetTime()).data <<
					std::this_thread::get_id() << ")\n";
				mlkFile.flush();
			}
		}
		
		void logVar(const char *text)
		{
#ifndef NDEBUG
			std::lock_guard<std::mutex> lk(varLogFileM);
			varLogFile << text << rtrc::stringsl::Rsnprintf<char, 40>("(time: %f, thread_id: ", glfwGetTime()).data <<
				std::this_thread::get_id() << ")\n";
			varLogFile.flush();
#endif
		}
		
		std::mutex logfile1M;
		std::ofstream logFile1;
		std::mutex logfile2M;
		std::ofstream logFile2;
		std::mutex ioFileM;
		std::ofstream ioFile;
		std::mutex mlkFileM;
		std::ofstream mlkFile;
		std::mutex varLogFileM;
		std::ofstream varLogFile;
		std::mutex performanceFileM;
		std::ofstream performanceFile;
		std::mutex traffic_CPUtoGPUFileM;
		std::ofstream traffic_CPUtoGPUFile;
		std::mutex traffic_GPUtoCPUFileM;
		std::ofstream traffic_GPUtoCPUFile;
		std::mutex traffic_genericFileM;
		std::ofstream traffic_genericFile;
	};


}}
