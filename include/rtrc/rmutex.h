/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   rmutex.h
 * Author: roohi
 *
 * Created on August 29, 2019, 3:05 PM
 */
#pragma once
#include <mutex>
#include <shared_mutex>
#include <memory>

#include "rexception.h"
namespace rtrc {
	
	/* template<typename lockT>
	 * struct ounlockable
	 * not implemented yet.
	 * functions :
	 *	void unlock() 
	 * does not have a lock function.
	 */ 
	
	struct updatableResourceMutex
	{
		/* with this mutex, readers can know if the resource has been updated or not.
		 * also when a writer is waitning no other reader can get in.
		 * 
		 * 
		 * each reader must have a std::shared_ptr<char> to check for updates.
		 * 
		 */
		
		typedef std::shared_ptr<char> feed_t;
		
		updatableResourceMutex(const updatableResourceMutex &) = delete;
		updatableResourceMutex(updatableResourceMutex &&) = delete;
		updatableResourceMutex &operator=(const updatableResourceMutex &) = delete;
		
		updatableResourceMutex()
			: updateHandler(std::make_shared<char>())
		{
			
		}
		
		
		
		std::unique_lock<std::mutex> getWriterLock()
		{
			std::unique_lock<std::mutex> inL(_inM);
			std::unique_lock<std::shared_mutex> pL(_pM); // we have full access to updateHandler
			updateHandler = std::make_shared<char>();
			
			// as long is this lock is not unlockd, nothing can get in.
			return std::move(inL);
		}
		
		// if the feed is outdated returns false. true otherwise.
		bool getReaderLock(std::shared_lock<std::shared_mutex> &readerLock, feed_t &updateFeed)
		{
			std::unique_lock<std::mutex> inL(_inM);
			 
			readerLock = std::shared_lock<std::shared_mutex>(_pM); // we can read from updateHandlers now
			if(updateFeed != updateHandler)
			{
				updateFeed = updateHandler;
				return false;
			}
			return true;
		}
		
		bool checkReaderMutex(std::shared_mutex *readersMutex) const noexcept
		{
			return readersMutex == &_pM;
		}
		
	private:
		
		
		
		std::mutex _inM;
		std::shared_mutex _pM;
		
		// update Handler is read when _pM is locked in any more, 
		// but it is written to only when _pM is locked in unique mode.
		feed_t updateHandler;
	};
	
	// writer attempts to lock this mutex will block any next reader request and allow the shared_mutex to be emptied. 
	struct wqSharedMutex
	{
		wqSharedMutex(const wqSharedMutex &) = delete;
		wqSharedMutex(wqSharedMutex &&) = delete;
		wqSharedMutex &operator=(const wqSharedMutex &) = delete;
		
		wqSharedMutex() {}
		
		std::shared_lock<std::shared_mutex> getReaderLock()
		{
			std::unique_lock<std::mutex> inL(_inM);
			return std::shared_lock<std::shared_mutex>(_pM);
		}
		
		std::shared_lock<std::shared_mutex> getReaderLock(std::unique_lock<std::mutex> &&writerLock)
		{
			if(writerLock.mutex() != &_inM) throw invalidMutexLock("attempt to get reader lock with invalid writer lock");
			return std::shared_lock<std::shared_mutex>(_pM); 
		}
		
		std::unique_lock<std::mutex> getWriterLock()
		{
			std::unique_lock<std::mutex> inL(_inM);
			std::unique_lock<std::shared_mutex> pL(_pM); // only one writer has control of the mutex now.
			// as long is this lock is not unlockd, nothing can get in.
			return std::move(inL);
		}
		
		
		bool checkReaderMutex(std::shared_mutex *readersMutex) const noexcept
		{
			return readersMutex == &_pM;
		}
	private:
		std::mutex _inM;
		std::shared_mutex _pM;
	};
}

