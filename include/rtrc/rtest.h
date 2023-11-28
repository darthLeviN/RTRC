/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   RTest.h
 * Author: roohi
 *
 * Created on July 2, 2019, 1:42 PM
 */
#pragma once


namespace rtrc
{
	/* performs testing of the library for "development" purposes. layers with 
	 * lower Index need to be performed first. 
	 * development tests must avoid any kind of dependence to the runtime environment
	 * purposes for DevTest can be as following:
	 * 1. ensure that the compiler works correctly with the software code regardless
	 * of the implementation of any STL library.
	 * 2. catch programming behavioural bugs inside the software itself.
	 * 3. catch incompatibility with softwares using rtrc headers by testing
	 * their data containers 
	 * 
	 * specifications :
	 * layer 1,2,3,4 are noexcept. 
     * group test functions that may include the first four layers are noexcept.
	 * 
	 */
	struct RDevTester
	{
		int devTestAll(bool printOutput) noexcept;
		
        
        // layer 1
		// done by some static asserts in builtintypecheck.h
        
		// layer 2
		// reserved.
		
		// layer 3
		int devTestTestUtility(bool printOutput) noexcept;
		
		// layer 4
		//int devTestExceptionLibrary(bool printOutput) noexcept; currently unimportant.
		
		// layer 5
		//void devTestMemoryHandlers(bool printOutput) noexcept(false); currently unimportant.
		
		// layer 6
		void devTestStringsAndIO(bool printOutput) noexcept(false);
		
		// layer 7
		void devTestParsers(bool printOutput) noexcept(false);
		
		// tests the layerI while ensuring lower layers have been tested.
		void devTestLayersUptoIndex(size_t layerI, bool printOutput) noexcept; 
		
		size_t testedLayer = 0; // a value of zero means first layer has not been tested.
	};
	
	/* performs a runtime test. ensuring that the integration with the running
	 * environment works properly. the test layers structured similarly to 
	 * RDevTester with the difference that dynamically linked libraries are included.
	 */
	struct RRunTester
	{
		int runTestALL(bool printOutput) noexcept;
		
        // layer 1
        // reserved.
        
		// layer 2
		// reserved.
		
		// layer 3
		int runTestTestUility(bool printOutput) noexcept;
		
		// layer 4
		int runTestExceptionLibrary(bool printOutput) noexcept;
		
		// layer 5
		void runTestMemoryHandlers(bool printOutput) noexcept(false);
		
		// layer 6
		void runTestStringsAndIO(bool printOutput) noexcept(false);
		
		// layer 7
		void runTestParsers(bool printOutput) noexcept(false);
		
		// tests the layerI while ensuring lower layers have been tested.
		int runTestLAyersUptoIndex(size_t layerI, bool printOutput) noexcept;
	};
	/* performs Testing of the integration with the runtime environment
	 */
}