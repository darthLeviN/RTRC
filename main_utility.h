

#pragma once

#include "mainDataLoader.h"

// rtrc includes
#include <rtrc/rtrc.h>
#include <rtrc/rstring.h>
#include <rtrc/rglfw.h>

// rtrc/graphical includes
#include <rtrc/graphical/ravlib.h>
#include <rtrc/graphical/rvulkan.h>
#include <rtrc/graphical/rvulkan_image_pipe.h>
#include <rtrc/graphical/rviewPort_core.h>
#include <rtrc/graphical/basicVkVideoPipeGroup.h>


// glfw includes
#include <GLFW/glfw3.h>

// std includes
#include <stdlib.h>
#include <fstream>
#include <vector>
#include <memory>
#include <mutex>
#include <chrono>
#include <thread>
#include <array>
#include <tuple>

namespace main
{

//
// definition of the used types
//
using appInstance_t = rtrc::vkl::rvkWSIAppInstance<rtrc::rglfwl::rglfwWindow>;

using vidPipe_t = rtrc::vkPipes::basicVkVideoPipeGroup<appInstance_t>;

typedef rtrc::vkl::mainGraphicsPuller<appInstance_t, vidPipe_t::rvkPipeGroup_t> grPuller_t;

const char * feed_url = nullptr;
const char * demuxer_name = nullptr;
AVInputFormat *demuxer = nullptr;

//
// window initial height and width
//
static constexpr size_t initialW = 400;
static constexpr size_t	initialH = 400;

//
// vulkan global variables 
//
// rendering
bool continueRendering = true; // no mutex needed, 
std::mutex renderCountersM;
size_t allRenderCounter = 0;


// UI related variables
std::mutex mainVPM;
std::shared_ptr<rtrc::layoutl::rviewPort_desktop<appInstance_t, grPuller_t>> mainVP;

// change this!.
std::ofstream logFile(Rsnprintf<char,100>("%s/rtrc.log",rtrc::logl::rtrc_logs_dir).data);


static VKAPI_ATTR VkBool32 VKAPI_CALL vkDebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {
	using namespace std;

    fprintf(stderr, "vulkan validation layer: %s\n", pCallbackData->pMessage);
	logFile << "vulkan validation layer: " << pCallbackData->pMessage << endl;
	//mainLogger.logText(pCallbackData->pMessage);

    return VK_FALSE;
}

void parse_main_input_options(int argc, char *argv[])
{
	for(size_t i = 0; i < (argc - 1); ++i)
	{
		if(strcmp(argv[i],"-i") == 0)
		{
			feed_url = argv[++i];
		}
		else if(strcmp(argv[i],"-d") == 0)
		{
			demuxer_name = argv[++i];
		}
		
	}
	if( !feed_url )
	{
		printf("no input was given");
		mainLogger.logText("no input was given");
		exit(-1);
	}
	if(demuxer_name)
	{
		demuxer = av_find_input_format("mpegts");
		if(!demuxer)
		{
			printf("demuxer not found");
			mainLogger.logText("demuxer not found");
			exit(-1);
		}
	}
}

auto make_appInstance()
{
	using namespace rtrc;
	using namespace rtrc::vkl;
	//using namespace std;
	auto window = std::make_unique<rtrc::rglfwl::rglfwWindow>(initialW,initialH,"fff");
	uint32_t requiredGlfwVkExtensionCount = 0;
	const char **requiredGlfwVkExtensions = nullptr;
	//VK_KHR_sampler_ycbcr_conversion
	requiredGlfwVkExtensions = glfwGetRequiredInstanceExtensions(&requiredGlfwVkExtensionCount);
	std::vector<const char *> vkExtensions(requiredGlfwVkExtensions, requiredGlfwVkExtensions + requiredGlfwVkExtensionCount);
	const char *vkValidationLayers[] = {"VK_LAYER_KHRONOS_validation", "VK_LAYER_LUNARG_api_dump"}; // activated in debug builds.
	swapchainProperties swapchainOptions;
	swapchainOptions._surfaceFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	swapchainOptions._surfaceFormat.format = VK_FORMAT_B8G8R8A8_UNORM;
	swapchainOptions._presentMode = 
			//VkPresentModeKHR::VK_PRESENT_MODE_MAILBOX_KHR;
			//VkPresentModeKHR::VK_PRESENT_MODE_FIFO_KHR; // change this for fps test
			VkPresentModeKHR::VK_PRESENT_MODE_IMMEDIATE_KHR;
	rvkPhysicalDeviceFeatures reqFeatures {};
	reqFeatures._devFeatures.geometryShader = VK_TRUE;
	reqFeatures._devFeatures.fillModeNonSolid = VK_TRUE;
	reqFeatures._devFeatures.depthClamp = VK_TRUE;
	reqFeatures._samplerYcbcrConversion = VK_TRUE;
	reqFeatures._devFeatures.samplerAnisotropy = VK_TRUE;
	std::shared_ptr<appInstance_t> appInstance(std::make_shared<appInstance_t>(vkExtensions, vkValidationLayers, 1, nullptr, reqFeatures, 
	vkDebugCallback, std::move(window), swapchainOptions));
	return appInstance;
}

template<typename appInstanceT>
void initAndConfigureMainVP(const std::shared_ptr<appInstanceT> &appInstance, const std::shared_ptr<grPuller_t> &mRPuller)
{
	using namespace rtrc::layoutl;
	std::lock_guard<std::mutex> vplk(mainVPM);
	mainVP = std::make_shared<rviewPort_desktop<appInstance_t, grPuller_t>>(appInstance, mRPuller, true);
	mainVP->configureInputs();
	auto dragCallback = [](decltype(mainVP)::element_type &vp, std::array<double,2> startPos, 
	std::array<double,2> drag, rtrc::uinl::rtrc_user_input_trigger_types trigger)
	{
		// vp.pointers_xxx functions are safe to call from this callback.
		vp.pointers_RotateView(drag[0],drag[1]);
	};
	mainVP->setAdditiveDragCallback(dragCallback);
}


template<typename appInstanceT>
void pollUIEvents(const std::shared_ptr<appInstanceT> &appInstance, size_t &pollCount)
{
	while(!appInstance->getWindow()->shouldClose())
	{
            mainLogger.logPerformance("pullsing glfw events");
            glfwWaitEvents();
            mainLogger.logPerformance("glfw events pulled");
            appInstance->getWindow()->pollInputEvents(glfwGetTime());
            appInstance->getWindow()->pollManualWindowEvents();
            main_thread_queue_handler.pullQueue();
            ++pollCount;
	}
}

}