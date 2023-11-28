

#pragma once
#include <GLFW/glfw3.h>
namespace rtrc
{
namespace rglfwl
{

static inline constexpr size_t GLFW_MOUSE_BUTTON_to_array_index(int GLFW_MOUSE_BUTTON_value)
{
	return GLFW_MOUSE_BUTTON_value;
}

static inline constexpr size_t GLFW_KEY_to_array_index(int GLFW_KEY_value)
{
	return GLFW_KEY_value - GLFW_KEY_UNKNOWN;
}
	
}	
}