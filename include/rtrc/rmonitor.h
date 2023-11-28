/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   monitorInfo.h
 * Author: roohi
 *
 * Created on August 20, 2019, 8:13 AM
 */

#pragma once
#include <GLFW/glfw3.h>
#include "rtrc.h"
#include <thread>
#include "rexception.h"

namespace rtrc { namespace monitor {
	inline void getPhysicalDPI(GLFWmonitor *monitor, float &xDPI, float &yDPI)
	{
		if(std::this_thread::get_id() != rtrc_main_thread_id)
		{
			throw outOfMainThreadCall("programming bug, critical call out of main thread");	
		}
		if(!monitor) monitor = glfwGetPrimaryMonitor();
		int milliW, milliH;
		glfwGetMonitorPhysicalSize(monitor, &milliW, &milliH);
		float inchW = milliW * 0.0393701f;
		float inchH = milliH * 0.0393701f;
		const GLFWvidmode *monitorVidmode = glfwGetVideoMode(monitor);
		if(!monitorVidmode) throw invalidCallState("cannot retrive monitor GLFWvidmode of monitor");
		xDPI = monitorVidmode->width / inchW;
		yDPI = monitorVidmode->height / inchH;
	}
	
	inline void getVisualDPI(GLFWmonitor *monitor, float &xDPI, float &yDPI)
	{
		if(std::this_thread::get_id() != rtrc_main_thread_id)
		{
			throw outOfMainThreadCall("programming bug, critical call out of main thread");	
		}
		if(!monitor) monitor = glfwGetPrimaryMonitor();
		int milliW, milliH;
		glfwGetMonitorPhysicalSize(monitor, &milliW, &milliH);
		float inchW = milliW * 0.0393701f;
		float inchH = milliH * 0.0393701f;
		const GLFWvidmode *monitorVidmode = glfwGetVideoMode(monitor);
		if(!monitorVidmode) throw invalidCallState("cannot retrive monitor GLFWvidmode of monitor");
		xDPI = monitorVidmode->width / inchW;
		yDPI = monitorVidmode->height / inchH;
		
		float xScale, yScale;
		glfwGetMonitorContentScale(monitor, &xScale, &yScale);
		xDPI *= xScale;
		yDPI *= yScale;
	}

}
}




