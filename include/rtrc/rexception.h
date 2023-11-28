/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   RException.h
 * Author: roohi
 *
 * Created on July 3, 2019, 9:53 PM
 */
#pragma once
#include <string_view>
#include <exception>
namespace rtrc
{
	struct RbasicException : std::exception
	{
		template<typename ...Args>
		RbasicException(Args... args) : message(args...) {}
		//std::string_view message;
		std::string message;
		
		const char *what() const noexcept override
		{
			return message.c_str();
		}
	};
	
	struct rImplementationError : RbasicException
	{
		rImplementationError(const char *msg) : RbasicException(msg) {}
	};
	
	struct rDeadLockException : RbasicException
	{
		rDeadLockException(const char *msg) noexcept : RbasicException(msg) {}
	};
	
	struct rNotYetException : RbasicException
	{
		rNotYetException(const char *msg) noexcept : RbasicException(msg) {}
	};
	
	struct rTimeoutException : RbasicException
	{
		rTimeoutException(const char *msg) noexcept : RbasicException(msg) {}
	};
	
	struct resourceException : RbasicException
	{
		resourceException(const char *msg) noexcept : RbasicException(msg) {}
	};
	
	
	struct shortbuffException : RbasicException
	{
		shortbuffException(const char *msg) noexcept : RbasicException(msg) {}
	};
	
	struct strFormatException : RbasicException
	{
		strFormatException(const char *msg) noexcept : RbasicException(msg) {}
	};
	
	struct allocationException : RbasicException
	{
		allocationException(const char *msg) noexcept : RbasicException(msg) {}
	};
	
	struct invalidFunctionCall : RbasicException
	{
		invalidFunctionCall(const char *msg) noexcept : RbasicException(msg) {}
	};
	
	struct invalidAccessRange : RbasicException
	{
		invalidAccessRange(const char *msg) noexcept : RbasicException(msg) {}
	};
	
	
	
	
	struct virtualConstManipulation : RbasicException
	{
		virtualConstManipulation(const char *msg) noexcept : RbasicException(msg) {}
	};
	
	struct usingUninitializedObject : RbasicException
	{
		usingUninitializedObject(const char *msg) noexcept : RbasicException(msg) {}
	};
	
	struct outOfMainThreadCall : RbasicException
	{
		outOfMainThreadCall(const char *msg) noexcept : RbasicException(msg) {}
	};
	
	struct invalidCallState : RbasicException
	{
		invalidCallState(const char *msg) noexcept : RbasicException(msg) {}
	};
	
	
	struct criticalFailure : RbasicException
	{
		criticalFailure(const char *msg) noexcept : RbasicException(msg) {}
	};
	
	struct mainAppObjInitializationFailure : criticalFailure
	{
		mainAppObjInitializationFailure(const char *msg) noexcept : criticalFailure(msg) {}
	};
	
	struct criticalCleanupFailure : criticalFailure
	{
		criticalCleanupFailure(const char *msg) noexcept : criticalFailure(msg) {}
	};
	
	struct badEnvironmentRequirement : mainAppObjInitializationFailure
	{
		badEnvironmentRequirement(const char *msg) noexcept : mainAppObjInitializationFailure(msg) {}
	};
	
	struct graphicalError : RbasicException
	{
		graphicalError(const char *msg) noexcept : RbasicException(msg) {}
	};
	
	struct presentationError : graphicalError
	{
		presentationError(const char *msg) noexcept : graphicalError(msg) {}
	};
	
	struct graphicalObjectInitializationFailed : graphicalError
	{
		graphicalObjectInitializationFailed(const char *msg) noexcept : graphicalError(msg) {}
	};
	
	struct resourceExpired : RbasicException
	{
		resourceExpired(const char *msg) noexcept : RbasicException(msg) {}
	};
	
	struct outOfGraphicsMemory : graphicalError
	{
		outOfGraphicsMemory(const char *msg) noexcept : graphicalError(msg) {}
	};
	
	struct invalidMutexLock : RbasicException
	{
		invalidMutexLock(const char *msg) noexcept : RbasicException(msg) {}
	};
	
	struct rInputReadException : RbasicException
	{
		rInputReadException(const char *msg) noexcept : RbasicException(msg) {}
	};
	
	struct rEOFException : rInputReadException
	{
		rEOFException(const char *msg) noexcept : rInputReadException(msg) {}
	};
	
	struct rReadTryAgainException : rInputReadException
	{
		rReadTryAgainException(const char *msg) : rInputReadException(msg) {}
	};
	
}
