

#define RTRC_CPP


#include <rtrc/graphical/rvulkan.h>
#include <GLFW/glfw3.h>
#include <rtrc/rtrc.h>
#include <rtrc/rmonitor.h>
#include <vulkan/vulkan.hpp>
#include <vector>
#include <cstdio>

namespace temprtrc
{
	void err_cb(int error, const char *desc)
	{
		//fprintf(stderr, "Error: %s\n", desc);
		// unimplemented
	}
	
	void createInstance() 
	{
		
	}
	
	void initVulkan()
	{
		createInstance();
	}
	
	
}

/* use glfwSetWindowAttrib to change the default window properties.
 * 
 */


using namespace temprtrc;
using namespace rtrc;
int main(int argc, char *argv[])
{
	if(!glfwInit()) // glfw initialization
	{
		printf("glfwInit() failed\n ");
		getchar();
		exit(-1);
	}
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // for vuklan api
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE); // window hidden on creation
	//glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
	_rtrcRawMouseMotionIsSupported = 
		glfwRawMouseMotionSupported() == GLFW_TRUE;
		
	
	float xDPI, yDPI;
	rtrc::monitor::getVisualDPI(NULL, xDPI, yDPI);
	printf("monitor dpi is %fx%f\n", (double)xDPI, (double)yDPI);
	
	
	
	
	uint32_t vkExtensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &vkExtensionCount, nullptr);
	std::vector<VkExtensionProperties> vkExtensions(vkExtensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &vkExtensionCount, vkExtensions.data());
	printf("%u vulkan extensions supported:\n", vkExtensionCount);
	for(uint32_t i = 0; i < vkExtensionCount ; ++i)
	{
		printf("\t%s\n", vkExtensions[i].extensionName);
	}
	int ret = -1;
	try
	{
		ret = RTRC_MAIN(argc,argv);
	}
	catch(mainAppObjInitializationFailure &error)
	{
		printf("%s\n", error);
		getchar();
		exit(-1);
	}
	glfwTerminate();
	return ret;
}
