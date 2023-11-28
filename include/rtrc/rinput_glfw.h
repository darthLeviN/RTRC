/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   rinput_glfw.h
 * Author: roohi
 *
 * Created on September 30, 2019, 3:08 PM
 */
#pragma once
#include "rinput.h"
#include <array>
#include <GLFW/glfw3.h>
#include <functional>
namespace rtrc
{
namespace uinl
{

/* thread safety : because of glfw limits, this struct is not designed to be 
 * owned outside of the main thread, although userInputOptions_core is designed 
 * to be able to handle multithreading, thus use of userInputOptions_core in 
 * other threads is allowed.
 */
struct glfw_inputManager : userInputOptions_core
{
	typedef std::function<void()> keyCallback_t;
	glfw_inputManager()
		: userInputOptions_core{}
	{
	}
		
	static constexpr size_t GLFW_KEY_to_array_index(int GLFW_KEY_value)
	{
		return GLFW_KEY_value - GLFW_KEY_UNKNOWN;
	}
	
	
	
private:
	std::array<keyCallback_t,GLFW_KEY_to_array_index(GLFW_KEY_LAST)+1> _keyCallbacks;
	double _camRotateSpeed = 0.7;
};

}
}
