/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   glextensionMgr.h
 * Author: roohi
 *
 * Created on August 13, 2019, 5:27 AM
 */
#include <GLFW/glfw3.h>
#pragma once
namespace rtrc { namespace glextmgr {
	inline bool checkGlExt(const char *name)
	{
		return glfwExtensionSupported(name);
	}
}
}
