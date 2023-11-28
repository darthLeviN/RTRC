
#pragma once
#include "decs/rvulkan_decs.h"
#include <vulkan/vulkan.hpp>
#include "../rexception.h"
#include <cstring>
#include <optional>
#include <set>
#include "../rmemory.h"
#include <limits>
#include <algorithm>
#include <memory>
#include <type_traits>
#include <shared_mutex>
#include "../rmutex.h"
#include "../rtrc.h"
#include <cstdint>
#include <thread>
#include <condition_variable>
#include <chrono>
#include <functional>
#include "rvulkan_core.h"
#include "rimage_core.h"
#include "rvulkan_image.h"
#include "rvkBits.h"
#include "../dcsynch.h"
#include <queue>
#include <boost/circular_buffer.hpp>
#include <glm//glm.hpp>
#include "../compiletime.h"

using namespace rtrc::memoryl;
using namespace rtrc::rimagel;
namespace rtrc { namespace vkl {
	

// move constructor
template<typename firstGrPipeT, typename ...grPipeGroupTs>
inline renderPassLock<firstGrPipeT, grPipeGroupTs...>::renderPassLock(renderPassLock &&moved)
	: _imageIndex(moved._imageIndex), 
	_cmdBuffersArr(moved._cmdBuffersArr), _perPassBuffers(std::move(moved._perPassBuffers)), 
	_pL(std::move(moved._pL)), _updateViewport(moved._updateViewport)
	, _scFeed(std::move(moved._scFeed))
{

}

// main constructor
template<typename firstGrPipeT, typename ...grPipeGroupTs>
inline renderPassLock<firstGrPipeT, grPipeGroupTs...>::renderPassLock(const uint32_t &imageIndex, const bool &updateViewport, 
	const std::shared_ptr<rvkCmdBuffers> &cmdBuffers,
std::shared_lock<std::shared_mutex> &&pL, const std::shared_ptr<char> &scFeed,
const std::shared_ptr<perPassBuffer_t<firstGrPipeT>> &fperPassBuffer,
const std::shared_ptr<grPipeGroupTs> &...otherGrPipeGroups)
	: _imageIndex(imageIndex), 
	_cmdBuffersArr{cmdBuffers,otherGrPipeGroups->getDefaultCmdBuffers()...}, _pL(std::move(pL)), 
	_updateViewport(updateViewport), _scFeed(scFeed),
	_perPassBuffers(fperPassBuffer, otherGrPipeGroups->getPerPassBuffer(imageIndex)...)
{
}

template<typename firstGrPipeT, typename ...grPipeGroupTs>
inline renderPassLock<firstGrPipeT, grPipeGroupTs...>::~renderPassLock()
{
	// currently does nothing.
}

// gets a command buffer of a pipe index as a template argument.
template<typename firstGrPipeT, typename ...grPipeGroupTs>
template<size_t pipeIndex>
inline const VkCommandBuffer &renderPassLock<firstGrPipeT, grPipeGroupTs...>::getCmdBuffer() const
{
#ifndef NDEBUG
	if(!_pL.owns_lock()) throw invalidCallState("renderPassLock is already unlockd");
#endif
	return _cmdBuffersArr[pipeIndex]->_cmdBuffers[_imageIndex];
}

// gets a command buffer of a pipe index as a function argument.
template<typename firstGrPipeT, typename ...grPipeGroupTs>
inline const VkCommandBuffer &renderPassLock<firstGrPipeT, grPipeGroupTs...>::getCmdBuffer(size_t pipeIndex) const
{
#ifndef NDEBUG
	if(!_pL.owns_lock()) throw invalidCallState("renderPassLock is already unlockd");
#endif
	return _cmdBuffersArr[pipeIndex]->_cmdBuffers[_imageIndex];
}

// gets a perPassBuffer of a pipe index as a template argument.
// note pipe index cannot be a function argument because that would mean
// having different return types.
template<typename firstGrPipeT, typename ...grPipeGroupTs>
template<size_t pipeIndex>
inline const auto &renderPassLock<firstGrPipeT, grPipeGroupTs...>::getPerPassBuffer()
{ 
#ifndef NDEBUG
	if(!_pL.owns_lock()) throw invalidCallState("renderPassLock is already unlockd");
#endif
	return std::get<pipeIndex>(_perPassBuffers);
}

// unlocks the main lock. call once after no member function needs to be 
// accessed anymore.
template<typename firstGrPipeT, typename ...grPipeGroupTs>
inline void renderPassLock<firstGrPipeT, grPipeGroupTs...>::unlock()
{
	_pL.unlock();
}

// call before unlock to issue the end of the recording of all commands 
// buffers.
template<typename firstGrPipeT, typename ...grPipeGroupTs>
inline void renderPassLock<firstGrPipeT, grPipeGroupTs...>::endRecording()
{
#ifndef NDEBUG
	if(!_pL.owns_lock()) throw invalidCallState("renderPassLock is already unlockd,");
#endif
	if(!_jobEnded)
	{
		// does not do an overflow check
		for(const auto &cmdBuffers : _cmdBuffersArr)
		{
			cmdBuffers->endI(_imageIndex);
		}
	}
#ifndef NDEBUG
	else
		throw graphicalError("tried to end recording of renderPassLock more than once");
#endif
}

template<typename firstGrPipeT, typename ...grPipeGroupTs>
inline bool renderPassLock<firstGrPipeT, grPipeGroupTs...>::isLocked() noexcept
{
	return _pL.owns_lock();
}

template<typename firstGrPipeT, typename ...grPipeGroupTs>
inline uint32_t renderPassLock<firstGrPipeT, grPipeGroupTs...>::getImageIndex() noexcept { return _imageIndex; }
template<typename firstGrPipeT, typename ...grPipeGroupTs>
inline auto renderPassLock<firstGrPipeT, grPipeGroupTs...>::getScFeed() noexcept { return _scFeed; }
template<typename firstGrPipeT, typename ...grPipeGroupTs>
inline std::shared_lock<std::shared_mutex> &renderPassLock<firstGrPipeT, grPipeGroupTs...>::getLock() noexcept { return _pL; }

// surfaceCreator must be callable with (VkInstance, VkSurfaceKHR *, VkAllocationCallbacks *) and return VkResult
//template<typename surfaceCreator_t>
template<typename windowT>
inline rvkWSIAppInstance<windowT>::rvkWSIAppInstance(std::vector<const char *> &vkReqExtensions,
const char **validationLayers, uint32_t validationLayerCount,
VkAllocationCallbacks *pAllocator, const rvkPhysicalDeviceFeatures &reqDevFeatures,
PFN_vkDebugUtilsMessengerCallbackEXT debugCallback, //const windowDestroyer_t &windowDestroyer,
std::unique_ptr<windowT> &&window, //const setResizeCallback_t &setResizeCallback,
//surfaceCreator_t surfaceCreator,
const swapchainProperties &swapChainOptions)
	: vulkanAppInstance(vkReqExtensions,
	validationLayers, 
	validationLayerCount, pAllocator, debugCallback), _window(std::move(window)),
		_swapchainOptions(swapChainOptions)
{
	//_setResizeCallback = setResizeCallback;

	bool VK_KHR_surface_enabled = false;
	for(const auto reqExt : vkReqExtensions)
	{
		if(!VK_KHR_surface_enabled)
			if(strcmp(reqExt,"VK_KHR_surface") == 0) VK_KHR_surface_enabled = true;
	}

	if(!VK_KHR_surface_enabled) 
		throw badEnvironmentRequirement(
		"\"VK_KHR_surface\" not requested but WSI vulkan instance requested\n");
	if(_window->createSurface(getInstance(), &_khrSurface, getpAllocator()) != VK_SUCCESS)
		throw mainAppObjInitializationFailure(
			"WSI surfaceCreator function failed!");
	_createlogicalDeviceWSI(reqDevFeatures); // edit to add arguments later.
	// now swapchains can be created.
	_window->setResizeCallback(_defaultResizeCallback, true); // setting the default resize callback and call it once to create the swapchain.
}

template<typename windowT>
inline rvkWSIAppInstance<windowT>::~rvkWSIAppInstance() noexcept
{
	_window->setResizeCallback({});
	if(_window)_window.release();
	vkQueueWaitIdle(_presentQueue);
	vkQueueWaitIdle(_graphicsQueue);
	_cleanUpSwapchainImgvs();
	_cleanUpSwapchainPresentSemaphores();
	vkDestroySwapchainKHR(getLogicalDevice(), _lastSwapchain, getpAllocator());
	vkDestroySurfaceKHR(getInstance(), _khrSurface, getpAllocator());
}

/* updates swapchain and notifies waiting functions.
 * call to this is thread safe.
 * blocks until the swapchain is recreated.
 */
template<typename windowT>
inline void rvkWSIAppInstance<windowT>::updateSwapchain(VkExtent2D &reqExtent)
{
	if(reqExtent.height && reqExtent.width)
	{
		_createSwapchain(reqExtent);
		std::unique_lock<std::mutex> slk(_statesMutex);
		_swapchainOutOfDate = false;
		_swapchainUptoDate = true;
		slk.unlock();
		_swapchainUpdateCV.notify_all();
	}
}

// a compile time programmed basic function to rate physical devices.
template<typename windowT>
inline constexpr uint32_t rvkWSIAppInstance<windowT>::ratePhysicalDevice(const VkPhysicalDeviceProperties &deviceProps) noexcept
{
	uint32_t score = 1;
	std::string ldevName(deviceProps.deviceName);
	std::transform(ldevName.begin(), ldevName.end(), ldevName.begin(), [](unsigned char c){return std::tolower(c);});
	// quick fix.
	if(strstr(ldevName.c_str(), "intel" )) 
	{
		printf("excluding %s\n", deviceProps.deviceName);
		return 0;
	}
	if(deviceProps.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		score += 1000;
	
	score += deviceProps.limits.maxImageDimension2D;
	return score;
}


// can also be called after locking _statesMutex
// creates/recreates swapChain. but does nothing notify anything else.
template<typename windowT>
inline void rvkWSIAppInstance<windowT>::_createSwapchain( VkExtent2D &reqExtent)
{
	mainLogger.logText("creating swapchain");
	VkPhysicalDevice physicalDevice = getPhysicalDevice();
	std::unique_lock<std::mutex> wlk(_swapchainUrMutex.getWriterLock());
	waitForGraphicsIdle();
	waitForPresentIdle();
	//vkQueueWaitIdle(_presentQueue);
	//vkQueueWaitIdle(_graphicsQueue);
	try
	{

		_cleanupSwapchain();
		_swapchainOptions._extent = reqExtent;
		_swapchainSupDetails.resetCapabilities(physicalDevice, _khrSurface);
		_swapchainSupDetails.getSwapExtent(_swapchainOptions._extent);
		const VkSurfaceCapabilitiesKHR &capabilities = _swapchainSupDetails.getCapabilities();
		uint32_t imageCount = capabilities.minImageCount + 1+30;
		if(capabilities.maxImageCount != 0) // special value, 0 : unlimited
		{
			imageCount = std::clamp(imageCount, capabilities.minImageCount, capabilities.maxImageCount);
		}

		// options is set, now we create the swapChain.
		SwapchainCreateInfoKHR scci;
		scci.surface = _khrSurface;
		scci.minImageCount = imageCount;
		scci.imageFormat = _swapchainOptions._surfaceFormat.format;
		scci.imageColorSpace = _swapchainOptions._surfaceFormat.colorSpace;
		scci.imageExtent = _swapchainOptions._extent; // all images share the same extent

		scci.imageArrayLayers = _swapchainOptions._imageLayerCount;
		scci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // this option needs future management.

		uint32_t queueFamilyIndices[] = { _graphicsQueueIndex, _presentQueueIndex };
		// cheap way to do it
		if(_graphicsQueueIndex != _presentQueueIndex) {
			scci.imageSharingMode = VK_SHARING_MODE_CONCURRENT; // not the best performance.
			scci.queueFamilyIndexCount = 2;
			scci.pQueueFamilyIndices = queueFamilyIndices;
		} else 
		{
			scci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			scci.queueFamilyIndexCount = 0; // optional
			scci.pQueueFamilyIndices = nullptr; // optional
		}

		scci.preTransform = capabilities.currentTransform; // can be managed via arguments.
		scci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // can be used for some visual effects if needed.
		scci.presentMode = _swapchainOptions._presentMode;
		scci.clipped = VK_TRUE; // can be managed via arguments. VK_TRUE : best performance

		//
		// SHOULD be managed for resizable windows in future.
		//
		scci.oldSwapchain = VK_NULL_HANDLE;
		if(vkCreateSwapchainKHR(getLogicalDevice(), &scci, getpAllocator(), &_lastSwapchain) != VK_SUCCESS)
		{
			throw mainAppObjInitializationFailure("could not create swapchain");
		};

		_recreateSwapchainImgsAndImgVs();
		_swapchainPresentSemaphores.resize(_swapchainImgVs.size());
	}
	catch(...)
	{
		_cleanupSwapchain();
		std::rethrow_exception(std::current_exception());
	}
	wlk.unlock();
	mainLogger.logText("swapchain created");
}

template<typename windowT>
inline void rvkWSIAppInstance<windowT>::_cleanupSwapchain() noexcept
{
	_cleanUpSwapchainImgvs();
	_cleanUpSwapchainImages(); // must come after no image view is left.
	_cleanUpSwapchainPresentSemaphores();
	vkDestroySwapchainKHR(getLogicalDevice(), _lastSwapchain, getpAllocator());
	_lastSwapchain == VK_NULL_HANDLE;
}


template<typename windowT>
inline bool rvkWSIAppInstance<windowT>::checkswapChainSupportDetails(const swapChainSupportDetails &swcsd, 
VkPhysicalDevice physicalDevice) const noexcept
{
	// this may need further specifications from the arguments.
	return !swcsd.getFormats().empty() && !swcsd.getPresentModes().empty();
	return true;
}

// creates the logical device, call once in constructor.
template<typename windowT>
inline void rvkWSIAppInstance<windowT>::_createlogicalDeviceWSI(const rvkPhysicalDeviceFeatures &reqFeatures, 
const VkPhysicalDeviceProperties &reqProps)
{
	//
	// getting a list of all available physical devices.
	//
	uint32_t deviceCount = 0;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	vkEnumeratePhysicalDevices(getInstance(), &deviceCount, nullptr);
	uint32_t maxScore{};


	VkPhysicalDeviceProperties foundProps {};
	rvkPhysicalDeviceFeatures foundFeatures {};
	if(deviceCount == 0)
		throw badEnvironmentRequirement("vulkan capable device not found");
	std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
	vkEnumeratePhysicalDevices(getInstance(), &deviceCount, physicalDevices.data());
	const VkPhysicalDevice *pFoundDevice = nullptr;

	std::vector<const char *> devReqExts = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};
	//
	// looping in physical devices to find the best one.
	//
	std::optional<uint32_t> graphicsQIndex, presentQIndex;
	for(const VkPhysicalDevice &physicalDevice : physicalDevices)
	{

		//checking device props

		vkGetPhysicalDeviceProperties(physicalDevice, &foundProps);
		printf("reviewing of device %s\n", foundProps.deviceName);
		//vkGetPhysicalDeviceFeatures(physicalDevice, &foundFeatures);
		
		if(!checkVkDeviceExtensions(physicalDevice, devReqExts))
		{
			printf("bad device extension support\n");
			continue;
		}
		if(!reqFeatures.checkFeatures(physicalDevice))
		{
			printf("bad device feature support\n");
			continue;
		}
		if(!checkVkDeviceProperties(foundProps, reqProps))
		{
			printf("bad device device required physical properties\n");
			continue;
		}

		// checking swap chain support
		//VkSurfaceFormatKHR reqFormat;
		//reqFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		//reqFormat.format = VK_FORMAT_B8G8R8A8_UNORM;
		//VkPresentModeKHR reqPresentMode = 
			//VkPresentModeKHR::VK_PRESENT_MODE_MAILBOX_KHR; // may need to be multiple ones in the future

		// warning : this implementation of swapchainSupDetails supports only one physical device.
		_swapchainSupDetails.reInitialize(physicalDevice, _khrSurface);
		if(!checkswapChainSupportDetails(_swapchainSupDetails, physicalDevice))
		{
			printf("bad swapchain support\n");
			continue; // this one is not good.
		}
		if(!_swapchainSupDetails.hasFormat(_swapchainOptions._surfaceFormat))
		{
			printf("bad swapchain format support\n");
			continue;
		}
		if(!_swapchainSupDetails.hasPresentationMode(_swapchainOptions._presentMode))
		{
			printf("bad swapchain presentation mode support\n");
			continue;
		}
			

		// checking device queue family capabilities.
		// support is needed for VK_QUEUE_GRAPHICS_BIT and vkGetPhysicalDeviceSurfaceSupportKHR
		//

		uint32_t foundScore = ratePhysicalDevice(foundProps);
		if( foundScore /*device not excluded*/ && (!pFoundDevice || foundScore > maxScore))
		{
			printf("further reviewing of device %s with the score of %llu\n", foundProps.deviceName,
					foundScore);
			// Testing device queue family, do nothing if not suitable.
			uint32_t qFamilyPropsCount = 0;
			
			vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &qFamilyPropsCount, nullptr);
			std::vector<VkQueueFamilyProperties> qFamilyProps(qFamilyPropsCount);
			vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &qFamilyPropsCount, qFamilyProps.data());

			uint32_t i = 0; 
			for( const auto &qfProps : qFamilyProps)
			{
				// graphicsQIndex
				if(!graphicsQIndex.has_value())
				{
					if(qfProps.queueCount > 0 && qfProps.queueFlags & VK_QUEUE_GRAPHICS_BIT)
					{
						printf(" graphics queue index %u found\n", i);
						//foundQueueFamilyIndex = i; // used to create logical device
						graphicsQIndex = i;
					}
				}
				// 
				if(graphicsQIndex.has_value())
				{
					VkBool32 sskhr = VK_FALSE;
					VkResult res = vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice,
					i, _khrSurface, &sskhr);
					if(sskhr) presentQIndex = i;

					if(presentQIndex.has_value())
					{
						if(_findSwapchainDepthFormatAndTiling(physicalDevice))
						{
							pFoundDevice = &physicalDevice;
							//_deviceProperties = foundProps;
							foundFeatures = reqFeatures;
							maxScore = foundScore;
							break;
						}
					}
				}			
				++i;
			}
		}
	}
	if(!pFoundDevice)
	{
		throw badEnvironmentRequirement("no found vulkan capable device met the requested requirements");
	}
	physicalDevice = *pFoundDevice;
	VkPhysicalDeviceMemoryProperties deviceMemProps;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &deviceMemProps);

	// now we create logical device and queues.
	size_t dqcisCount = 2;
	if(graphicsQIndex.value() == presentQIndex.value())
	{
		dqcisCount = 1; // cheap solution. requested indexes have to be unique.
	}
	std::vector<DeviceQueueCreateInfo> dqcis(dqcisCount);
	float priority = 1.0;
	auto &grdqci = dqcis[0];
	grdqci.queueCount = 1;
	grdqci.queueFamilyIndex = graphicsQIndex.value();
	grdqci.pQueuePriorities = &priority;
	if(dqcisCount == 2)
	{
		auto &prdqci = dqcis[1];
		prdqci.queueCount = 1;
		prdqci.queueFamilyIndex = presentQIndex.value();
		prdqci.pQueuePriorities = &priority;
	}


	VkDeviceCreateInfo dvci{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
	//dvci.pNext = &_deviceFeatures.getFeatures2();
	dvci.pQueueCreateInfos = dqcis.data();
	dvci.queueCreateInfoCount = dqcis.size(); // may need changing if more queue infos are used.
	dvci.pEnabledFeatures = nullptr;
	dvci.enabledExtensionCount = devReqExts.size();
	dvci.ppEnabledExtensionNames = devReqExts.data();
	createLogicalDevice(physicalDevice, dvci, foundFeatures, deviceMemProps);
	// now that device was created in _logicalDevice, we retrieve the created queue
	_graphicsQueueIndex = graphicsQIndex.value();
	vkGetDeviceQueue(getLogicalDevice(), graphicsQIndex.value(), 0, &_graphicsQueue);
	_presentQueueIndex = presentQIndex.value();
	vkGetDeviceQueue(getLogicalDevice(), presentQIndex.value(), 0, &_presentQueue);


}

/* recreates the swapchain depth images. must call _cleanUpSwapchainImages()
 * after use in destructor and before reuse. 
 */
template<typename windowT>
inline void rvkWSIAppInstance<windowT>::_recreateSwapchainDepthImages(size_t imageCount)
{
#ifndef NDEBUG
	if(_swapchainDepthImgs.size())
		throw mainAppObjInitializationFailure("could not recreate swapchain"
			" depth images, invalid size for _swapchainDepthImgs while "
			"calling _recreateSwapchainDepthImages, probably missed a call "
			"to _cleanUpSwapchainImages()");
#endif
	const VkExtent2D &extent = _swapchainOptions._extent; // last swapchain options updated before calling this.
	VkFormat format = _swapchainOptions._depthAndStencilImageFormat;//VK_FORMAT_D24_UNORM_S8_UINT; // add a search for supported formats in future.
	VkFormatProperties fmtProps;
	getFormatProperties(fmtProps, format);// this can be called only once in the constructor.
	for(size_t i = 0; i < imageCount; ++i)
	{
		_swapchainDepthImgs.emplace_back(getLogicalDevice(), getpAllocator(), getDeviceMemoryProps(),
			extent.width, extent.height, 1, format, fmtProps, _swapchainOptions._depthAndStencilImageTiling,
			VK_SHARING_MODE_EXCLUSIVE, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);


	}
}

// Clean up created image views after use in destructor and before reuse. 
// or modify this for automatic cleanup.
template<typename windowT>
inline void rvkWSIAppInstance<windowT>::_recreateSwapchainImgsAndImgVs()
{
	//_cleanUpSwapchainImgvs(); this will be called by _cleanupSwapchain();
	// _getSwapchainColorImages also updates some values thus should be called first.
	auto vkColorImages = _getSwapchainColorImages(); // may need to choose whether this is needed or not.
	uint32_t imageCount = vkColorImages.size();
	if(imageCount == 0)
	{
		throw mainAppObjInitializationFailure("swapchain has no images");
	}

	_recreateSwapchainDepthImages(imageCount);

	_swapchainImgVs.resize(imageCount);

	VkImageViewCreateInfo civci = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
	civci.viewType = VK_IMAGE_VIEW_TYPE_2D;
	civci.format = _swapchainOptions._surfaceFormat.format;

	civci.subresourceRange.baseMipLevel = 0;
	civci.subresourceRange.levelCount = 1;
	civci.subresourceRange.baseArrayLayer = 0;
	civci.subresourceRange.layerCount = _swapchainOptions._imageLayerCount;
	civci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

	VkImageViewCreateInfo divci = civci;
	divci.format = _swapchainOptions._depthAndStencilImageFormat;
	divci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

	for(uint32_t i = 0; i < imageCount; ++i)
	{
		VkImage &vkColorImage = vkColorImages[i];

		civci.image = vkColorImage;

		if(vkCreateImageView(getLogicalDevice(), &civci, getpAllocator(), &_swapchainImgVs[i].colorV) 
			!= VK_SUCCESS)
			throw mainAppObjInitializationFailure("could not create swapchain color image view");
		divci.image = _swapchainDepthImgs[i]._hImage;
		if(vkCreateImageView(getLogicalDevice(), &divci, getpAllocator(), &_swapchainImgVs[i].depthV)
			!= VK_SUCCESS)
			throw mainAppObjInitializationFailure("could not create swapchain depth image view");

	}
}

// returns the swapchain color images
template<typename windowT>
inline std::vector<VkImage> rvkWSIAppInstance<windowT>::_getSwapchainColorImages() noexcept
{
	uint32_t imageCount;
	vkGetSwapchainImagesKHR(getLogicalDevice(), _lastSwapchain, &imageCount, nullptr);
	std::vector<VkImage> images(imageCount);
	vkGetSwapchainImagesKHR(getLogicalDevice(), _lastSwapchain, &imageCount, images.data());
	_swapchainOptions._imageCount = imageCount;
	return std::move(images);
}

// cleans up manually allocated swapchain images
template<typename windowT>
inline void rvkWSIAppInstance<windowT>::_cleanUpSwapchainImages() noexcept
{
	_swapchainDepthImgs.clear();

}

// cleans up swapchain image views
template<typename windowT>
inline void rvkWSIAppInstance<windowT>::_cleanUpSwapchainImgvs() noexcept
{
	for(swapchainImgVGroup &imgv : _swapchainImgVs)
	{
		vkDestroyImageView(getLogicalDevice(), imgv.colorV, getpAllocator());
		vkDestroyImageView(getLogicalDevice(), imgv.depthV, getpAllocator());
	}
	_swapchainImgVs.clear();

}

// Cleans up presentation semaphores for ALL of the swapchain images.
template<typename windowT>
inline void rvkWSIAppInstance<windowT>::_cleanUpSwapchainPresentSemaphores() noexcept
{
	auto hLogicalDevice = getLogicalDevice();
	auto pAllocator = getpAllocator();
	for(auto &semArr : _swapchainPresentSemaphores)
	{
		for(auto sem : semArr)
		{
			vkDestroySemaphore(hLogicalDevice, sem, pAllocator);
		}
		semArr.clear();
	}
}

/* changes the swapchain presentation semaphores with new ones.
 * old semaphores for the same image index, can be freed safely.
*/
template<typename windowT>
inline void rvkWSIAppInstance<windowT>::_changeSwapchainPresentSemaphores(uint32_t imageIndex, uint32_t semCount, VkSemaphore *sems)
{
	auto hLogicalDevice = getLogicalDevice();
	auto pAllocator = getpAllocator();
	auto &semArr = _swapchainPresentSemaphores[imageIndex];
	for( auto sem : semArr)
	{
		vkDestroySemaphore(hLogicalDevice, sem, pAllocator);
	}
	semArr.resize(semCount);
	memcpy(semArr.data(), sems, sizeof(VkSemaphore)*semCount);
}

// Configures the swapchain image settings in _swapchainOptions.
template<typename windowT>
inline bool rvkWSIAppInstance<windowT>::_findSwapchainDepthFormatAndTiling(VkPhysicalDevice phyDev)
{
	VkFormatProperties fProps;
	std::array<VkFormat,3> preferredFormats { VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT_S8_UINT};
	for(auto fmt : preferredFormats )
	{
		vkGetPhysicalDeviceFormatProperties(phyDev, fmt, &fProps);
		if( fProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT == 
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
		{
			_swapchainOptions._depthAndStencilImageFormat = fmt;
			_swapchainOptions._depthAndStencilImageTiling = VK_IMAGE_TILING_OPTIMAL;
			return true;
		}
	}
	return false;
}


// if swapchain needs to be reconfigured, returns true. in that case, the readerLock gets unlocked(invalidated)
template<typename windowT>
inline bool rvkWSIAppInstance<windowT>::acquireNextImageKHR(std::shared_lock<std::shared_mutex> &readerLock, uint32_t &imageIndex, VkSemaphore &semaphore, VkFence fence, uint64_t timeout)
{
	auto hLogicalDevice = getLogicalDevice();
	auto pAllocator = getpAllocator();

	// if function succeeds(usual situations), there won't be any performance issue.
	if(!_swapchainUrMutex.checkReaderMutex(readerLock.mutex())) 
		throw graphicalError("read access to swapchain without a valid lock");
	std::unique_lock<std::mutex> slk;
	VkResult acqRes = vkAcquireNextImageKHR(getLogicalDevice(), _lastSwapchain, timeout, 
		semaphore, fence, &imageIndex);
	bool needUpdate = false;


	switch(acqRes)
	{
		case VK_SUCCESS :
			break;
		case VK_SUBOPTIMAL_KHR :
		{
			std::lock_guard<std::mutex> slk(_statesMutex);
			_swapchainUptoDate = false;
			goto end;
			break;
		}
		case VK_ERROR_OUT_OF_DATE_KHR :
		{
			readerLock.unlock(); // warning!. this lock allows relocking.
			std::lock_guard<std::mutex> slk(_statesMutex);
			_swapchainUptoDate = false;
			_swapchainOutOfDate = true;
			needUpdate = true;
			break;
		}
		default:
		{
			vkDestroySemaphore(getLogicalDevice(), semaphore, getpAllocator());
			semaphore = VK_NULL_HANDLE;
			throw graphicalError("vkAcquireNextImageKHR failed");
		}
	}

	if(needUpdate && semaphore != VK_NULL_HANDLE) 
	{
		// previous semaphore may be useless now.
		vkDestroySemaphore(hLogicalDevice, semaphore, pAllocator);
		constexpr VkSemaphoreCreateInfo sCI{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
		if(
			vkCreateSemaphore(hLogicalDevice, &sCI, pAllocator, &semaphore) != VK_SUCCESS) 
			throw graphicalError("could not create semaphore");
	}
	end:
	return needUpdate;
}

// the lock guarantees that from the time that the image was aquired, there was no swapchain reconstruction.
template<typename windowT>
inline void rvkWSIAppInstance<windowT>::singlePresent(std::shared_lock<std::shared_mutex> &readerLock,
uint32_t imageIndex, uint32_t semaphoresCount, VkSemaphore *pSemaphores)
{
#ifndef NDEBUG
	if(!_swapchainUrMutex.checkReaderMutex(readerLock.mutex())) 
		throw graphicalError("read access to swapchain without a valid lock");
#endif
	VkPresentInfoKHR presentInfo{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &_lastSwapchain;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.waitSemaphoreCount = semaphoresCount;
	presentInfo.pWaitSemaphores = pSemaphores;
	VkResult res;
	{
		std::lock_guard<std::mutex> qlk(_presentQueueM);
		res = vkQueuePresentKHR(_presentQueue, &presentInfo); 

	}
	if(res != VK_SUCCESS)
	{
		if(res == VK_ERROR_OUT_OF_DATE_KHR)
		{
			_changeSwapchainPresentSemaphores(imageIndex, semaphoresCount, pSemaphores);
			throw presentationError("could not present swapchain image, swapchain is out of date");
		}
		//else
		throw graphicalError("could not present swapchain image");
	}


	_changeSwapchainPresentSemaphores(imageIndex, semaphoresCount, pSemaphores);
}

template<typename windowT>
inline VkResult rvkWSIAppInstance<windowT>::submitToGraphics(uint32_t submitCount, const VkSubmitInfo *pSubmits, VkFence fence)
{
	std::lock_guard<std::mutex> qlk(_graphicsQueueM);
	return vkQueueSubmit(_graphicsQueue, submitCount, pSubmits, fence);
}

template<typename windowT>
inline void rvkWSIAppInstance<windowT>::waitForGraphicsIdle()
{
	std::lock_guard<std::mutex> qlk(_graphicsQueueM);
	vkQueueWaitIdle(_graphicsQueue);
}

template<typename windowT>
inline void rvkWSIAppInstance<windowT>::waitForPresentIdle()
{
	std::lock_guard<std::mutex> qlk(_presentQueueM);
	vkQueueWaitIdle(_presentQueue);
}



template<typename windowT>
inline const auto &rvkWSIAppInstance<windowT>::getLastSwapchainOptions() const noexcept { return _swapchainOptions; }
template<typename windowT>
inline const auto &rvkWSIAppInstance<windowT>::getLastSwapchainImageViews() const noexcept { return _swapchainImgVs; }
template<typename windowT>
inline const auto &rvkWSIAppInstance<windowT>::getLastSwapchainDepthImgs() const noexcept { return _swapchainDepthImgs; }

// don't use queue data directly for operations that require synchronization with the queue in the vulkan specifications.
template<typename windowT>
inline const auto &rvkWSIAppInstance<windowT>::getGraphicsQueueIndex() const noexcept { return _graphicsQueueIndex; }
template<typename windowT>
inline auto rvkWSIAppInstance<windowT>::getGraphicsQueue() const noexcept { return _graphicsQueue; }
template<typename windowT>
inline const auto &rvkWSIAppInstance<windowT>::getPresentQueueIndex() const noexcept { return _presentQueueIndex; }
template<typename windowT>
inline auto rvkWSIAppInstance<windowT>::getPresentQueue() const noexcept { return _presentQueue; }

template<typename windowT>
inline auto &rvkWSIAppInstance<windowT>::getWindow() noexcept { return _window; }



// acquires a reader lock, waits and blocks if the swapchain is outdated to prevent repeating errors.
template<typename windowT>
inline bool rvkWSIAppInstance<windowT>::getSwapchainReaderLock(std::shared_lock<std::shared_mutex> &readerLock, 
updatableResourceMutex::feed_t &feed)
{
	std::unique_lock<std::mutex> slk(_statesMutex);
	//if(_lastSwapchain == VK_NULL_HANDLE)
		//throw graphicalError("swapchain was not created"); // this will result in a single validation check for most operations.
	if(_swapchainOutOfDate)
	{
		_swapchainUpdateCV.wait(slk, [this]() { return !_swapchainOutOfDate;});
	}
	return _swapchainUrMutex.getReaderLock(readerLock, feed); 
}


template<VkShaderStageFlagBits stageBits>
template<typename appInstanceT>
inline rvkShaderModule<stageBits>::rvkShaderModule(const std::shared_ptr<appInstanceT> &appInstance, const std::vector<char> &code, 
const char *pName)
	: _appInstance(appInstance), _pName(pName)
{

	VkShaderModuleCreateInfo smci = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
	smci.codeSize = code.size();
	smci.pCode = reinterpret_cast<const uint32_t *>(code.data());
	if(vkCreateShaderModule(_appInstance->getLogicalDevice(), &smci, 
		_appInstance->getpAllocator(), &_shaderModule) != VK_SUCCESS)
		throw graphicalObjectInitializationFailed( "failed to create shader module");
}
template<VkShaderStageFlagBits stageBits>
inline rvkShaderModule<stageBits>::~rvkShaderModule()
{
	vkDestroyShaderModule(_appInstance->getLogicalDevice(), _shaderModule, _appInstance->getpAllocator());
}

template<VkShaderStageFlagBits stageBits>
inline auto &rvkShaderModule<stageBits>::getShaderModule() { return _shaderModule; } 
template<VkShaderStageFlagBits stageBits>
inline auto &rvkShaderModule<stageBits>::getAppInstance() { return _appInstance.get(); }
template<VkShaderStageFlagBits stageBits>
inline const char *rvkShaderModule<stageBits>::getpName() { return _pName; }
template<VkShaderStageFlagBits stageBits>
inline VkPipelineShaderStageCreateInfo rvkShaderModule<stageBits>::getShaderStageCI() const noexcept
{
	VkPipelineShaderStageCreateInfo ssci{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
	ssci.stage = _stageFlag;
	ssci.module = _shaderModule;
	ssci.pName = _pName;

	return ssci;
}


// i am unsure wether to unify samplers, buffers and extraResources or not.
template<typename perPassBufferT>
inline commonVkPipeSettings<perPassBufferT>::commonVkPipeSettings(std::vector<VkVertexInputBindingDescription> &&bindingDescs, 
std::vector<VkVertexInputAttributeDescription> &&attrDescs, std::vector<std::vector<VkDescriptorSetLayoutBinding>> &&descSetLBsArr,
	rvkDescSameUpdater &&descSetsConfigurator,
	std::vector<std::shared_ptr<void>> &&extraResources)
	: _inputBindingDescs(std::move(bindingDescs)), _inputAttrDescs(std::move(attrDescs)), _descSetLBsArr(std::move(descSetLBsArr)),
	_descSetsConfigurator(std::move(descSetsConfigurator)),
	_extraResources(std::move(extraResources))
{

}


template<typename perPassBufferT>
inline bool commonVkPipeSettings<perPassBufferT>::dynamicViewport() { return false; }
// describes vertex attributes
template<typename perPassBufferT>
inline pipeVertexInputStateCI commonVkPipeSettings<perPassBufferT>::vertexInputStateCI()
{
	// copy verctors then use their rvale reference as input of the constructor.
	return {std::vector<VkVertexInputBindingDescription>(_inputBindingDescs),
		std::vector<VkVertexInputAttributeDescription>(_inputAttrDescs)};
}

// describes the drawing method.
template<typename perPassBufferT>
inline VkPipelineInputAssemblyStateCreateInfo commonVkPipeSettings<perPassBufferT>::inputAssemblyStateCI()
{
	VkPipelineInputAssemblyStateCreateInfo iasci { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
	iasci.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	iasci.primitiveRestartEnable = VK_FALSE; // what is this?

	return iasci;
}

// cheap temporary function
template<typename perPassBufferT>
inline std::vector<VkViewport> commonVkPipeSettings<perPassBufferT>::viewPorts(const swapchainProperties &scopts)
{
	std::vector<VkViewport> vps(1);
	vps[0].width = (float) scopts._extent.width;
	vps[0].height = (float) scopts._extent.height;
	vps[0].minDepth = 0.0f;
	vps[0].maxDepth = 1.0f;
	vps[0].x = 0.0f;
	vps[0].y = 0.0f;

	return vps;
}


// defines viewport clip.
template<typename perPassBufferT>
inline pipeViewportStateCI commonVkPipeSettings<perPassBufferT>::viewportStateCI(const swapchainProperties &scopts)
{	
	std::vector<VkRect2D> scissors(1);
	scissors[0].extent = scopts._extent;
	scissors[0].offset = {0,0}; //optional?

	return pipeViewportStateCI(viewPorts(scopts), std::move(scissors));	
}

// defines rasterizer.
template<typename perPassBufferT>
inline VkPipelineRasterizationStateCreateInfo commonVkPipeSettings<perPassBufferT>::restrizationStateCI()
{
	VkPipelineRasterizationStateCreateInfo rasterizer =
	{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;

	//rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.cullMode = VK_CULL_MODE_NONE;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;

	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f; // optional
	rasterizer.depthBiasClamp = 0.0f; // optional
	rasterizer.depthBiasSlopeFactor = 0.0f; //optional

	return rasterizer;
}

// defines multisampling
template<typename perPassBufferT>
inline VkPipelineMultisampleStateCreateInfo commonVkPipeSettings<perPassBufferT>::multisamplerStateCI()
{
	VkPipelineMultisampleStateCreateInfo mul = 
	{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };

	mul.sampleShadingEnable = VK_FALSE;
	mul.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	mul.minSampleShading = 1.0f; // optional
	mul.pSampleMask = nullptr; // optional
	mul.alphaToCoverageEnable = VK_FALSE; // optional
	mul.alphaToOneEnable = VK_FALSE; // optional

	return mul;
}

// defines color blend settings
template<typename perPassBufferT>
inline colorBlendStateCI commonVkPipeSettings<perPassBufferT>::colorBlendAttState()
{
	std::vector<colorBlendAttachmentState> attStates(1);
	attStates[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | 
		VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
		VK_COLOR_COMPONENT_A_BIT;
	attStates[0].blendEnable = VK_FALSE;
	colorBlendStateCI cbsci(std::move(attStates));
	cbsci._ci.logicOpEnable = VK_FALSE;

	return cbsci;
}

// descriptor sets create info.
template<typename perPassBufferT>
inline std::vector<descSetLayoutCI> commonVkPipeSettings<perPassBufferT>::descSetLayout()
{
	std::vector<descSetLayoutCI> ret;
	ret.reserve(_descSetLBsArr.size());
	// for each descriptor set configuration in _descSetLBsArr
	for(auto &descSetLBs : _descSetLBsArr ) 
	{
		// create a descSetLayoutCI for that descriptor set configuration.
		ret.emplace_back(std::vector<VkDescriptorSetLayoutBinding>(descSetLBs));
	}

	return ret;
	//return descSetLayoutCI(std::vector<VkDescriptorSetLayoutBinding>(_descSetLBs));
}

template<typename perPassBufferT>
inline VkPipelineDepthStencilStateCreateInfo commonVkPipeSettings<perPassBufferT>::depthStencilStateCI()
{
	VkPipelineDepthStencilStateCreateInfo ret
	{VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
	ret.depthTestEnable = VK_TRUE;
	ret.depthWriteEnable = VK_TRUE;
	ret.depthCompareOp = VK_COMPARE_OP_LESS;
	ret.depthBoundsTestEnable = VK_FALSE;
	ret.minDepthBounds = 0.0f;
	ret.maxDepthBounds = 1.0f;

	ret.stencilTestEnable = VK_FALSE;
	ret.front = {};
	ret.back = {};

	return ret;
}



// defines 
template<typename perPassBufferT>
inline rvkRenderPassCI commonVkPipeSettings<perPassBufferT>::renderPassCI(const swapchainProperties &scopts)
{
	return {attachmentDescriptions(scopts), subpassDependencies() ,subpassDescs()};
}

// this will get more arguments in the future.
template<typename perPassBufferT>
inline std::vector<VkFramebufferCreateInfo> commonVkPipeSettings<perPassBufferT>::framebuffersCI(VkRenderPass &renderPass, 
const std::vector<swapchainImgVGroup> &swapchainImageViews, const swapchainProperties &scopts)
{
	size_t imageCount = swapchainImageViews.size();
	std::vector<VkFramebufferCreateInfo> fbuffersCI(imageCount);
	VkFramebufferCreateInfo cisample = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
	cisample.renderPass = renderPass;
	cisample.attachmentCount = 2; // size of attachmentDescriptions, number of image views for each framebuffer.
	cisample.width = scopts._extent.width;  // all swapchain images share the same extent.
	cisample.height = scopts._extent.height;
	cisample.layers = scopts._imageLayerCount;

	for(size_t i = 0; i < imageCount; ++i)
	{
		fbuffersCI[i] = cisample;
		fbuffersCI[i].pAttachments = &swapchainImageViews[i].colorV; // first attachment is color, second is depth and stencil
	}

	return fbuffersCI;
}

template<typename perPassBufferT>
inline rvkRenderPassBeginInfo commonVkPipeSettings<perPassBufferT>::renderPassBeginInfo(VkRenderPass renderPass, 
VkFramebuffer framebuffer, const swapchainProperties &scopts)
{
	std::vector<VkClearValue> clearColors(2);
	clearColors[0].color = {0.0f,0.0f,0.0f,1.0f};
	clearColors[1].depthStencil = { 1.0f, 0 };
	rvkRenderPassBeginInfo rpbi(std::move(clearColors));

	rpbi._beginInfo.renderPass = renderPass;
	rpbi._beginInfo.framebuffer = framebuffer;
	rpbi._beginInfo.renderArea.offset = {0, 0};
	rpbi._beginInfo.renderArea.extent = scopts._extent;

	return rpbi;
}

template<typename perPassBufferT>
inline const rvkDescSameUpdater &commonVkPipeSettings<perPassBufferT>::getDescSetsConfigurator() { return _descSetsConfigurator; }

/* describes the data in attached images.
 * An attachment description describes the properties of an attachment 
 * including its format, sample count, and how its contents are treated 
 * at the beginning and end of each render pass instance.
 * 
 * indexes in the attachmentDescription array, are referred by VkAttachmentReference structs.
 */
template<typename perPassBufferT>
inline std::vector<VkAttachmentDescription> commonVkPipeSettings<perPassBufferT>::attachmentDescriptions(const swapchainProperties &scopts)
{
	// the way this is done, may change in the future.

	std::vector<VkAttachmentDescription> catts(2);
	VkAttachmentDescription catt{};
	catt.format = scopts._surfaceFormat.format;
	catt.samples = VK_SAMPLE_COUNT_1_BIT; // may need changing in future.
	catt.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	catt.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	catt.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	catt.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	catt.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	catt.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	catts[0] = catt;
	catt.format = scopts._depthAndStencilImageFormat;
	catt.finalLayout = //VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	catt.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
	//catt.
	catts[1] = catt;
	return catts;
}

/* VkSubPassDependecies handle synchronization between pipeline Stages
 */
template<typename perPassBufferT>
inline std::vector<VkSubpassDependency> commonVkPipeSettings<perPassBufferT>::subpassDependencies()
{
	
	/*std::vector<VkSubpassDependency> deps(1,VkSubpassDependency{});
	return deps;*/
	return {};
}

/* indexes in subpass's pColorAttachments desc are directly referenced in 
 * fragment shader's output location.
 * A subpass represents a phase of rendering that reads and writes a subset 
 * of the attachments in a render pass.
 * 
 * a subpass description specifies the index of it's attachments and their layout.
 */
template<typename perPassBufferT>
inline rvkSubpassDescs commonVkPipeSettings<perPassBufferT>::subpassDescs()
{
	/* each subpass describption references a arrays of VkAttachmentReference 
	 * to represent fragment's outputs by it's pColorAttachments members.
	 * 
	 * 
	 */

	rvkAttachmentReferences attRefs{};
	attRefs.colorAttRefs.resize(1);
	attRefs.colorAttRefs[0].attachment = 0; // index in the attachmentDescriptions
	attRefs.colorAttRefs[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference dAsAttRef {};
	dAsAttRef.attachment = 1;
	dAsAttRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	attRefs.depthAndStencilAttachment = dAsAttRef;

	// array of groups of fragment's output descriptions.
	std::vector<rvkAttachmentReferences> AttRefsAr; //{std::move(attRefs)} };
	AttRefsAr.push_back(std::move(attRefs));
	rvkSubpassDescs sdesc(std::move(AttRefsAr));
	sdesc._subpassDescs[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

	return sdesc;
}

template<typename settingsT, typename appInstanceT>
inline rvkGrPipeGroup<settingsT, appInstanceT>::rvkGrPipeGroup(const appInstance_t &appInstance, settings_t &&settings,const vrShader_t &vrShader, const frShader_t &frShader,
	const geoShader_t &geoShader)
	: settings_t(std::move(settings)),_appInstance(appInstance), _vrShader(vrShader), _frShader(frShader), _geoShader(geoShader)
{
	/* bug : arguments may originate from different instances, currently 
	 * this is left unmanaged because not all of the software requirements are known.
	 * simple fixes may be added but they may introduce incompatiblity.
	 */
	_create<true>();

}

template<typename settingsT, typename appInstanceT>
inline void rvkGrPipeGroup<settingsT, appInstanceT>::grbindCmdBuffer(VkCommandBuffer cmdBuffer)
{
	vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _graphicsPipeline);
}

// it's unlikely that it happens but having a writeLock to the swapchain will result in a trap when calling this.
template<typename settingsT, typename appInstanceT>
template<typename ...Args> // Args are shared pointer types of the next graphics pipelines
inline auto rvkGrPipeGroup<settingsT, appInstanceT>::beginRenderRecording(VkSemaphore &khrSemaphore, VkFence khrFence, VkSubpassContents contents,
recordCallerFlusher_t flusher, const std::shared_ptr<Args> &... otherGrPipeGroups)
{

	auto hLogicalDevice = _appInstance->getLogicalDevice();
	auto pAllocator = _appInstance->getpAllocator();

	std::shared_lock<std::shared_mutex> pL;
	uint32_t imageIndex;
	bool reconSc = false;

	/* there is two points which in the function is informaed that reconfiguration is needed
	 * 1. when acquiring mutex, the function is informed that the swapchain it used before is outdated.
	 * 2. when acquiring a image from the swapchain, which can happen because the operating system invalidates the swapchain
	 * based on the _settings member, then the function decides to rebuild the whole pipeline or just the frame buffers.
	*/ 
	if(!_appInstance->getSwapchainReaderLock(pL,_swapchainRefreshFeed))
	{
		reconSc = true;
	}
	doOver:
	while(_appInstance->acquireNextImageKHR(pL, imageIndex, khrSemaphore, khrFence))
	{
		// pL is unlocked(invalidated)
		reconSc = true;
		_appInstance->getSwapchainReaderLock(pL,_swapchainRefreshFeed);
	}
	// outside of previous loop, the read key is valid.
	if(reconSc)
	{
		if(settings_t::dynamicViewport())_reconfigureSwapchain(); // just recreate Framebuffers
		else // recreate the whole pipeline because it's static.
		{
			if(flusher)flusher();
			_reCreate();
			reconSc = false;
			//if(!_appInstance->getSwapchainReaderLock(pL,_swapchainRefreshFeed))
			//{
				//goto doOver;

			//}
			//else
				//reconSc = false;
		}
	}

	// begin recording
	_defaultCmdBuffers->beginI(imageIndex, 0); /*** for now we have no flags ***/
	// binding pipeline to command buffer.
	grbindCmdBuffer(_defaultCmdBuffers->_cmdBuffers[imageIndex]); 
	// binding descriptor sets for uniforms
	if(_descSets.size())
	{
		vkCmdBindDescriptorSets(_defaultCmdBuffers->_cmdBuffers[imageIndex],
			VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 0, _descSets[imageIndex].size(),
			_descSets[imageIndex].data(), 0, nullptr);
	}

	rvkRenderPassBeginInfo rpbi = settings_t::renderPassBeginInfo(_renderPass,
		_swapchainFramebuffers[imageIndex], _appInstance->getLastSwapchainOptions());
	// begin render pass
	vkCmdBeginRenderPass(_defaultCmdBuffers->_cmdBuffers[imageIndex], &rpbi.getRef(), contents);

	return renderPassLock<type, Args...>
		(imageIndex, reconSc, _defaultCmdBuffers, std::move(pL), _swapchainRefreshFeed,
		_perPassBuffers[imageIndex], otherGrPipeGroups...);//postRenderProc);
}

/*void grCmdDraw(renderPassLock<1> &rpL, uint32_t vertexCount, uint32_t instanceCount,
	uint32_t firstVertex, uint32_t firstInstance)
{
	vkCmdDraw(rpL.getCmdBuffer(), vertexCount, instanceCount,
		firstVertex, firstInstance);
}*/
template<typename settingsT, typename appInstanceT>
inline std::vector<VkViewport> rvkGrPipeGroup<settingsT, appInstanceT>::viewPorts() { return settings_t::viewPorts(_appInstance->getLastSwapchainOptions()); }

template<typename settingsT, typename appInstanceT>
inline rvkGrPipeGroup<settingsT, appInstanceT>::~rvkGrPipeGroup()
{
	auto hLogicalDevice = _appInstance->getLogicalDevice();
	auto pAllocator = _appInstance->getpAllocator();

	_cleanupPipe();
}

template<typename settingsT, typename appInstanceT>
inline const auto &rvkGrPipeGroup<settingsT, appInstanceT>::getPerPassBuffer(const size_t &imageIndex)
{
	return _perPassBuffers[imageIndex];
}

template<typename settingsT, typename appInstanceT>
inline const auto &rvkGrPipeGroup<settingsT, appInstanceT>::getDefaultCmdBuffers()
{
	return _defaultCmdBuffers;
}

//auto &getSettingsManager() { return _settings; }

/* use a external flusher function before use to make sure pipeline isn't 
 * in use anymore.
 */
template<typename settingsT, typename appInstanceT>
inline void rvkGrPipeGroup<settingsT, appInstanceT>::_reCreate()
{
	_cleanupPipe();
	_create<false>();
}

template<typename settingsT, typename appInstanceT>
template<bool lockSwapchain>
inline void rvkGrPipeGroup<settingsT, appInstanceT>::_create()
{
	auto hLogicalDevice = _appInstance->getLogicalDevice();
	auto pAllocator = _appInstance->getpAllocator();

	//
	// creating CreateInfo objects
	//
	std::vector<VkPipelineShaderStageCreateInfo> shaderStageCIs;
	shaderStageCIs.reserve(3);
	if(_vrShader) shaderStageCIs.emplace_back(_vrShader->getShaderStageCI());
	if(_frShader) shaderStageCIs.emplace_back(_frShader->getShaderStageCI());
	if(_geoShader) shaderStageCIs.emplace_back(_geoShader->getShaderStageCI());


	rvkRenderPassCI rPassCI(settings_t::renderPassCI(_appInstance->getLastSwapchainOptions()));

	std::vector<descSetLayoutCI> descSetLCIs(settings_t::descSetLayout());
	size_t descSetLCount = descSetLCIs.size();
	_descSetLayouts.resize(descSetLCount, VK_NULL_HANDLE);
	for(size_t i = 0; i < descSetLCount ; ++i)
	{
		if(vkCreateDescriptorSetLayout(hLogicalDevice, &descSetLCIs[i]._ci, pAllocator, &_descSetLayouts[i]) != VK_SUCCESS)
			throw graphicalError("could not create VkSecriptorSetLayout");
	}
	VkPipelineLayoutCreateInfo pipeLayoutCI{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
	pipeLayoutCI.setLayoutCount = _descSetLayouts.size();
	pipeLayoutCI.pSetLayouts = _descSetLayouts.data();
	colorBlendStateCI colorBSCI(settings_t::colorBlendAttState());
	VkPipelineMultisampleStateCreateInfo multisampleCI(settings_t::multisamplerStateCI());
	VkPipelineRasterizationStateCreateInfo resterStateCI(settings_t::restrizationStateCI());
	pipeViewportStateCI vpStateCI(
		settings_t::viewportStateCI(_appInstance->getLastSwapchainOptions()));
	VkPipelineInputAssemblyStateCreateInfo inputAssStateCI(settings_t::inputAssemblyStateCI());
	pipeVertexInputStateCI vertexISCI(settings_t::vertexInputStateCI());
	VkPipelineDepthStencilStateCreateInfo depthStencilStateCI(settings_t::depthStencilStateCI());

	//
	// creating necessary objects to initialize the pipeline.
	//
	_createPipelineLayout(pipeLayoutCI);
	_createRenderPass(rPassCI);
	//
	// now that renderPass is created, we are free to create the framebuffers.
	//
	if(lockSwapchain)
	{
		std::shared_lock<std::shared_mutex> swapchainReadlock;
		_appInstance->getSwapchainReaderLock(swapchainReadlock, _swapchainRefreshFeed);
		_reconfigureSwapchain();
		//swapchainReadlock.unlock();
	}
	else
	{
		_reconfigureSwapchain();
	}
	// creating main pipeline object
	VkGraphicsPipelineCreateInfo pipelineCI = {VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
	pipelineCI.stageCount = shaderStageCIs.size();
	pipelineCI.pStages = shaderStageCIs.data();

	pipelineCI.pVertexInputState = &vertexISCI.getRef();
	pipelineCI.pInputAssemblyState = &inputAssStateCI;
	pipelineCI.pViewportState = &vpStateCI.getRef();
	pipelineCI.pRasterizationState = &resterStateCI;
	pipelineCI.pMultisampleState = &multisampleCI;
	pipelineCI.pDepthStencilState = &depthStencilStateCI;
	pipelineCI.pColorBlendState = &colorBSCI.getRef();
	pipelineCI.pDynamicState = nullptr;

	pipelineCI.layout = _pipelineLayout;
	pipelineCI.renderPass = _renderPass;
	pipelineCI.subpass = 0;

	pipelineCI.basePipelineHandle = VK_NULL_HANDLE;
	pipelineCI.basePipelineIndex = -1; // optional
	// development note : may be used to create multiple pipelines
	if(vkCreateGraphicsPipelines(hLogicalDevice,
		VK_NULL_HANDLE, 1, &pipelineCI, pAllocator, &_graphicsPipeline)
		!= VK_SUCCESS)
		throw graphicalObjectInitializationFailed("could not create main pipeline object");

	_createPrimCmdPoolAndBuffers();
	//
	// creating _descPool
	//
	size_t totaldescSetCount = _swapchainFramebuffers.size() * _descSetLayouts.size();
	std::vector<VkDescriptorPoolSize> poolSizes;
	poolSizes.reserve(_descSetLayouts.size() * 16);

	size_t maxDescsCount = 0;
	for( auto &ci : descSetLCIs )
	{
		for ( auto &descSetLB : ci._bindings )
		{
			VkDescriptorPoolSize poolSize;
			poolSize.descriptorCount = _swapchainFramebuffers.size();
			poolSize.type = descSetLB.descriptorType;
			poolSizes.emplace_back(poolSize);
		}
	}
	VkDescriptorPoolCreateInfo descPoolCI { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
	descPoolCI.poolSizeCount = poolSizes.size();
	descPoolCI.pPoolSizes = poolSizes.data();
	descPoolCI.maxSets = totaldescSetCount;
	//descPoolCI.flags

	//
	// creating descriptor sets
	//
	if(descPoolCI.poolSizeCount != 0 && descPoolCI.maxSets != 0) // according to the specification these values shouldn't be zero.
	{
		if( vkCreateDescriptorPool(hLogicalDevice, &descPoolCI, pAllocator, &_descPool) != VK_SUCCESS )
		{
			throw graphicalError("could not create VkDescriptorPool");
		}
		// using _descPool to allocate descriptorSets for each image of the swapchain
		VkDescriptorSetAllocateInfo descSetAI{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
		size_t descSetCount =  _descSetLayouts.size();
		descSetAI.descriptorSetCount = descSetCount;
		descSetAI.descriptorPool = _descPool;
		descSetAI.pSetLayouts = _descSetLayouts.data();
		// one internal vector for each swapchain image, n elements in each for n each descriptor set.
		_descSets.resize(_swapchainFramebuffers.size());
		for( std::vector<VkDescriptorSet> &descSetArr : _descSets )
		{
			descSetArr.resize(descSetCount);
			if(vkAllocateDescriptorSets(hLogicalDevice, &descSetAI, descSetArr.data())
				!= VK_SUCCESS)
				throw graphicalError("could not allocate VkDescriptorSet");
		}
	}
	//
	// creating descriptor buffers
	//
	_perPassBuffers.resize(_swapchainFramebuffers.size());
	for( size_t i = 0; i < _perPassBuffers.size(); ++i )
	{
		 _perPassBuffers[i] = std::make_shared<perPassBuffer_t>(_appInstance, _descSets[i]);
	}

	//
	// configuring descriptor sets
	//
	const rvkDescSameUpdater &descSetsConfigurator = settings_t::getDescSetsConfigurator();
	// sets each perPassBuffer_t as the buffer for descriptor set
	descSetsConfigurator.update(hLogicalDevice, _descSets, _perPassBuffers ); 
}


template<typename settingsT, typename appInstanceT>
inline void rvkGrPipeGroup<settingsT, appInstanceT>::_cleanupPipe()
{
	auto hLogicalDevice = _appInstance->getLogicalDevice();
	auto pAllocator = _appInstance->getpAllocator();

	vkDestroyDescriptorPool(hLogicalDevice, _descPool, pAllocator);

	vkDestroyPipeline(hLogicalDevice, _graphicsPipeline, pAllocator);
	_graphicsPipeline = VK_NULL_HANDLE;
	_destroyFramebuffers();

	for( VkDescriptorSetLayout dsl : _descSetLayouts )
	{
		vkDestroyDescriptorSetLayout(hLogicalDevice, dsl, pAllocator);
	}
	_descSetLayouts.clear();

	vkDestroyRenderPass(hLogicalDevice, _renderPass , pAllocator);
	_renderPass = VK_NULL_HANDLE;

	// note : if pipeline layout is not destroyed, validation layer says pipeline is not destroyed when destroying the device.
	vkDestroyPipelineLayout(hLogicalDevice, _pipelineLayout, pAllocator);
	_pipelineLayout = VK_NULL_HANDLE;


}



template<typename settingsT, typename appInstanceT>
inline void rvkGrPipeGroup<settingsT, appInstanceT>::_createPrimCmdPoolAndBuffers()
{
	// dev note : check of the wait op is needed.
	_appInstance->waitForGraphicsIdle();
	_defaultCmdPool = std::make_shared<rvkCmdPool>(_appInstance,
		_appInstance->getGraphicsQueueIndex(), 
		VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
	// swapchain image count is unlikely to change.
	_defaultCmdBuffers = std::make_shared<rvkCmdBuffers>(_appInstance, _defaultCmdPool, 
		VK_COMMAND_BUFFER_LEVEL_PRIMARY, _swapchainFramebuffers.size());
}

template<typename settingsT, typename appInstanceT>
inline void rvkGrPipeGroup<settingsT, appInstanceT>::_reconfigureSwapchain()
{
	// it is the caller's job to aquire and manage the locks.
	_destroyFramebuffers();
	auto hLogicalDevice =_appInstance->getLogicalDevice();
	auto pAllocator = _appInstance->getpAllocator();

	std::vector<VkFramebufferCreateInfo> fbuffersCI = 
		settings_t::framebuffersCI(_renderPass, 
		_appInstance->getLastSwapchainImageViews(), 
		_appInstance->getLastSwapchainOptions());
	size_t framebuffersCount = fbuffersCI.size();

	_swapchainFramebuffers.resize(framebuffersCount,VK_NULL_HANDLE);
	for( size_t i = 0; i < framebuffersCount; ++i)
	{
		if(vkCreateFramebuffer(hLogicalDevice,&fbuffersCI[i], pAllocator, 
			&_swapchainFramebuffers[i]) != VK_SUCCESS)
			throw graphicalObjectInitializationFailed("could not create swapchain framebuffer");
	}
}

template<typename settingsT, typename appInstanceT>
inline void rvkGrPipeGroup<settingsT, appInstanceT>::_destroyFramebuffers() noexcept
{
	auto hLogicalDevice =_appInstance->getLogicalDevice();
	auto pAllocator = _appInstance->getpAllocator();

	for( VkFramebuffer &fb : _swapchainFramebuffers)
	{
		vkDestroyFramebuffer(hLogicalDevice, fb, pAllocator);
		fb = VK_NULL_HANDLE;
	}
}

template<typename settingsT, typename appInstanceT>
inline void rvkGrPipeGroup<settingsT, appInstanceT>::_createPipelineLayout( const VkPipelineLayoutCreateInfo &pipeLayoutCI)
{
	auto hLogicalDevice = _appInstance->getLogicalDevice();
	auto pAllocator = _appInstance->getpAllocator();
	if(vkCreatePipelineLayout(hLogicalDevice, &pipeLayoutCI, pAllocator, &_pipelineLayout)
		!= VK_SUCCESS)
		throw graphicalObjectInitializationFailed("could not create VkPipelineLayout");
}

template<typename settingsT, typename appInstanceT>
inline void rvkGrPipeGroup<settingsT, appInstanceT>::_createRenderPass( const rvkRenderPassCI &renderPassCI)
{
	auto hLogicalDevice = _appInstance->getLogicalDevice();
	auto pAllocator = _appInstance->getpAllocator();
	if(vkCreateRenderPass(hLogicalDevice, &renderPassCI.getRef(), pAllocator, &_renderPass)
		!= VK_SUCCESS)
		throw graphicalObjectInitializationFailed("could not create RenderPass");
}
	

/* handles pulling in the graphics queue.
 */
template<typename appInstanceT, typename ...grPipeGroupTs>
inline mainGraphicsPuller<appInstanceT, grPipeGroupTs...>::mainGraphicsPuller(const std::shared_ptr<appInstance_t> &appInstance, 
	const std::shared_ptr<grPipeGroupTs> &... grPipeGroups)
	: _appInstance(appInstance), _grPipesTuple(grPipeGroups...)
{
	for(size_t i = 0; i < _prevCleanedUp.size(); ++i)
	{
		_prevCleanedUp[i] = true;
	}

	VkFenceCreateInfo fci{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };


	auto hLogicalDevice = _appInstance->getLogicalDevice();
	auto pAllocator = _appInstance->getpAllocator();
	for(int i = 0; i < reusableCount; ++i)
	{
		_reusableFences.emplace_back(std::make_shared<rvkFence>(_appInstance));
	}
	_reFillReservedSems();
}

template<typename appInstanceT, typename ...grPipeGroupTs>
inline mainGraphicsPuller<appInstanceT, grPipeGroupTs...>::~mainGraphicsPuller() noexcept
{
	auto hLogicalDevice = _appInstance->getLogicalDevice();
	auto pAllocator = _appInstance->getpAllocator();

	for( size_t i = 0; i < pipeGroupCount; ++i)
	{
		if(_preRenderCmds[i].size() || _postRenderCmds[i].size())
		{
			renderLessPull();
			break;
		}
	}
	// cleaning up semaphores
	_cleanupRenderQueue();
#ifndef NDEBUG
	if(_nextRenderWaitSem != VK_NULL_HANDLE)
		throw graphicalError("_nextRenderWaitSem should be freed helper functions");
#endif

	for( auto &sem : _reservedSems )
	{
		vkDestroySemaphore(hLogicalDevice, sem, pAllocator);
	}
	// cleaning up fences;
}

template<typename appInstanceT, typename ...grPipeGroupTs>
template<typename cmdCallableT>
inline void mainGraphicsPuller<appInstanceT, grPipeGroupTs...>::graphicsPull(cmdCallableT &drawPull)
{
	auto res = graphicsPull(drawPull, {}, {}, {},0);	
}

template<typename appInstanceT, typename ...grPipeGroupTs>
inline void mainGraphicsPuller<appInstanceT, grPipeGroupTs...>::renderLessPull()
{
	auto hLogicalDevice = _appInstance->getLogicalDevice();
	auto pAllocator = _appInstance->getpAllocator();

	std::vector<VkSemaphore> extraSems;
	std::unique_lock<std::mutex> rpQMlk(_renderPullQueueMutex);
	std::array<std::vector<VkPipelineStageFlags>,pipeGroupCount> postRenderWaitFlags(std::move(_postRenderWaitFlags));
	std::array<std::vector<VkPipelineStageFlags>,pipeGroupCount> preRenderWaitFlags(std::move(_preRenderWaitFlags));
	std::array<std::vector<VkCommandBuffer>,pipeGroupCount> preRenderCmds(std::move(_preRenderCmds));
	std::array<std::vector<VkCommandBuffer>,pipeGroupCount> postRenderCmds(std::move(_postRenderCmds));
	std::array<std::vector<VkSemaphore>,pipeGroupCount> preRenderWaitSems(std::move(_preRenderWaitSems));
	std::array<std::vector<VkSemaphore>,pipeGroupCount> postRenderWaitSems(std::move(_postRenderWaitSems));
	decltype(_renderPullcounter) renderIndex = _renderPullcounter++;
	_selectNextFence();
	rpQMlk.unlock();
	_renderPullQueueStartCV.notify_all();
	try
	{
		std::vector<VkSubmitInfo> allSubmitInfos;
		// dev note : due to limited size, allSubmitInfos can be changed to a static sized array.
		allSubmitInfos.reserve(2*pipeGroupCount);
		/* if the rendering pipeline has any pre or post Render requests.
		 */
		std::array<VkSemaphore,pipeGroupCount> preSignalSemaphores{};
		for(size_t i = 0; i < pipeGroupCount; ++i)
		{
			auto &preRCmds = preRenderCmds[i];
			auto &postRCmds = postRenderCmds[i];
			if(preRCmds.size() || postRCmds.size())
			{
				std::array<VkSubmitInfo,2> submitInfos {{{ VK_STRUCTURE_TYPE_SUBMIT_INFO },
				{ VK_STRUCTURE_TYPE_SUBMIT_INFO }}};
				VkSubmitInfo &preRenderSI = submitInfos[0];
				VkSubmitInfo &postRenderSI = submitInfos[1];

				size_t submitStagesCount = 0;
				VkSubmitInfo *pSubmitInfos = submitInfos.data() + 1;
				// adding postRender commands
				if(postRenderCmds.size())
				{
					++submitStagesCount;
					// adding preRender-postRender dependencies
					if(preRenderCmds.size())
					{
						preSignalSemaphores[i] = addSemaphore(postRenderWaitSems[i]);
						preRenderSI.signalSemaphoreCount = 1;
						preRenderSI.pSignalSemaphores = &preSignalSemaphores[i];
					}
					postRenderSI.waitSemaphoreCount = postRenderWaitSems[i].size();
					postRenderSI.pWaitSemaphores = postRenderWaitSems[i].data();
					postRenderSI.pWaitDstStageMask = postRenderWaitFlags[i].data();
					postRenderSI.commandBufferCount = postRenderCmds[i].size();
					postRenderSI.pCommandBuffers = postRenderCmds[i].data();
				}
				// adding preRender commands
				if(preRenderCmds.size())
				{
					++submitStagesCount;
					--pSubmitInfos;
					preRenderSI.waitSemaphoreCount = preRenderWaitSems[i].size();
					preRenderSI.pWaitSemaphores = preRenderWaitSems[i].data();
					preRenderSI.pWaitDstStageMask = preRenderWaitFlags[i].data();
					preRenderSI.commandBufferCount = preRenderCmds[i].size();
					preRenderSI.pCommandBuffers = preRenderCmds[i].data();
				}
				if(submitStagesCount)
				{
					allSubmitInfos.insert(allSubmitInfos.end(), pSubmitInfos, pSubmitInfos + submitStagesCount);
				}
			}
		}
		if(allSubmitInfos.size() &&
			_appInstance->submitToGraphics(allSubmitInfos.size(), allSubmitInfos.data(), _allRF()->getHandle())
			!= VK_SUCCESS)
		{
			throw rtrc::graphicalError("could not submit queue");
		}
		_allRF()->setIsSubmitted(renderIndex);

		//_cleanupFence((_reusableFenceSelection + 1)%reusableCount);
		_cleanupFence((_reusableFenceSelection + 2)%reusableCount);
		/* warning, _nextRenderWaitSem was freed in here, but the 
		 * validation layer did not recognize it's absence as resource 
		 * leak.
		 */
		if(_nextRenderWaitSem != VK_NULL_HANDLE)
		{
			vkDestroySemaphore(hLogicalDevice, _nextRenderWaitSem, pAllocator);
			_nextRenderWaitSem = VK_NULL_HANDLE;
		}
		_addCurrentFenceDump(std::move(preRenderWaitSems), {}, std::move(postRenderWaitSems), std::move(extraSems));
		_renderPullQueueEndCV.notify_all();
	}
	catch(...)
	{
		_appInstance->waitForGraphicsIdle();
		for( auto &sem : extraSems )
		{
			vkDestroySemaphore(hLogicalDevice, sem, pAllocator);
		}
		for(size_t i = 0; i < pipeGroupCount; ++i)
		{
			for( auto &sem : preRenderWaitSems[i] )
			{
				vkDestroySemaphore(hLogicalDevice, sem, pAllocator);
			}
			for( auto &sem : postRenderWaitSems[i] )
			{
				vkDestroySemaphore(hLogicalDevice, sem, pAllocator);
			}
		}
		_cleanupRenderQueue();
		if(_nextRenderWaitSem != VK_NULL_HANDLE)
		{
			vkDestroySemaphore(hLogicalDevice, _nextRenderWaitSem, pAllocator);
			_nextRenderWaitSem = VK_NULL_HANDLE;
		}
		std::rethrow_exception(std::current_exception());
	}
}



// the returned semaphores are experimental because there is no clear usage to them to make a proper design.
template<typename appInstanceT, typename ...grPipeGroupTs>
template<typename ...cmdCallableTs>
inline void mainGraphicsPuller<appInstanceT, grPipeGroupTs...>::graphicsPull(const std::tuple<cmdCallableTs...> &drawPulls,
std::array<std::vector<VkSemaphore>,pipeGroupCount> &&renderWaitSems, std::array<std::vector<VkPipelineStageFlags>,pipeGroupCount> &&renderWaitFlags,
const std::tuple<perPassWriteCmd_t<grPipeGroupTs>...> &perPassWrites) noexcept
{
	/* reusable Fences for local semaphores will be added to 
	 * _validFinishFences **as soon as possible** along with semaphore to 
	 * in _postRenderFinishedSemaphores or _postRenderLeftoverSemaphores or 
	 * one of the _prevXXX semaphore vectors.
	 * 
	 */
	//std::lock_guard lk(gm);
	/*auto startTime = std::chrono::high_resolution_clock::now();
	static size_t timeSumCPU(0);
	static constexpr size_t maxCount = 1000;
	static size_t counter = 0;
	counter = counter % maxCount;
	++counter;
	bool doPrint = counter == maxCount;*/
	VkSemaphoreCreateInfo semCI { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };

	auto hLogicalDevice = _appInstance->getLogicalDevice();
	auto pAllocator = _appInstance->getpAllocator();


	std::vector<VkSemaphore> extraSems;
	//
	// each throw resulting in invalidation of fences and semaphores must pop 
	// the fence from _postRenderFinishedSemaphores and destroy the semaphores
	//
	std::unique_lock<std::mutex> rpqLk(_renderPullQueueMutex);
	std::array<std::vector<VkPipelineStageFlags>,pipeGroupCount> postRenderWaitFlags(std::move(_postRenderWaitFlags));
	std::array<std::vector<VkPipelineStageFlags>,pipeGroupCount> preRenderWaitFlags(std::move(_preRenderWaitFlags));
	std::array<std::vector<VkCommandBuffer>,pipeGroupCount> preRenderCmds(std::move(_preRenderCmds));
	std::array<std::vector<VkCommandBuffer>,pipeGroupCount> postRenderCmds(std::move(_postRenderCmds));
	std::array<std::vector<VkSemaphore>,pipeGroupCount> preRenderWaitSems(std::move(_preRenderWaitSems));
	std::array<std::vector<VkSemaphore>,pipeGroupCount> postRenderWaitSems(std::move(_postRenderWaitSems));
#ifndef NDEBUG
	// testing move constructor
	for( auto &arr : _preRenderCmds)
	{
		if(arr.size())
		{
			mainLogger.logText("move constructor not working");
			exit(-1);
		}
	}
#endif
	// semaphores that should be at the end of the main render operation.
	std::array<std::vector<VkSemaphore>,pipeGroupCount> renderSignalSems;
	std::array<std::vector<VkSemaphore>,pipeGroupCount> postRenderSignalSems;
	std::array<std::vector<rvkPostRenderCopyRequest_t<grPipeGroupTs...>>,pipeGroupCount> 
		postRenderCopyRequests(std::move(_postRenderCopyRequests));
	//std::vector<std::unique_lock<std::mutex>> renderLocks;
	renderSignalSems[0].reserve(2);
	//td::unique_lock<std::mutex> pqlk(_renderPullQueueMutex);
	auto renderIndex = _renderPullcounter++;
	if(_nextRenderWaitSem != VK_NULL_HANDLE)
	{
		renderWaitSems[0].push_back(_nextRenderWaitSem);
		renderWaitFlags[0].push_back(VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
		_nextRenderWaitSem = VK_NULL_HANDLE;
	}
	//renderLocks
	_selectNextFence();
	rpqLk.unlock();
	try
	{
		{	
#ifndef NDEBUG
			for(size_t i = 0; i < pipeGroupCount; ++i)
			{
				if(!preRenderCmds[i].size() && preRenderWaitSems[i].size())
					throw graphicalError("implementation bug : preRenderCmds.size() && !preRenderWaitSems.size() is true");
				if(preRenderWaitSems[i].size() != preRenderWaitFlags[i].size())
					throw graphicalError("implementation bug : preRenderWaitSems.size() != preRenderWaitFlags.size() is true");
				if(!postRenderCmds[i].size() && postRenderWaitSems[i].size())
					throw graphicalError("implementation bug : postRenderCmds.size() && !postRenderWaitSems.size() is true");
				if(postRenderWaitSems[i].size() != postRenderWaitFlags[i].size())
					throw graphicalError("implementation bug : postRenderWaitSems.size() != postRenderWaitFlags.size() is true");

			}
#endif
		}

		_renderPullQueueStartCV.notify_all(); // next fence is free for queuing.

		renderWaitFlags[0].reserve(renderWaitFlags[0].size()+2);


		renderWaitFlags[0].push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT); // flag for next smeaphore
		VkSemaphore &imageAvailableSemaphore = addSemaphore(renderWaitSems[0]); // this has to be a reference because it may be recreated when passed as an argument.

		VkSemaphore presentWaitSem = rvkCreateSemaphore(_appInstance);
		VkSemaphore nextRenderWaitSem = rvkCreateSemaphore(_appInstance);
		renderSignalSems.back().emplace_back(presentWaitSem);
		renderSignalSems.back().emplace_back(nextRenderWaitSem);
		// creating semaphores for returning, destroy all in case of error
		// signalSemaphores : the final signal semaphores that are supposed to be returned in the end.

		auto flusher = [this](){
				//pqlk.lock();
				std::lock_guard<std::mutex> lk(this->_renderPullQueueMutex);
				this->_cleanupRenderQueue();};

		_rplCreator_t rplCreator(_grPipesTuple, imageAvailableSemaphore, VK_NULL_HANDLE, VK_SUBPASS_CONTENTS_INLINE,
		flusher);
		// this definitely needs a reference to the semaphore to recreate it if needed.
		renderPassLock_t rpL = partialUnpack<1, pipeGroupCount>(rplCreator, _grPipesTuple);

		//auto &perPassBuffer = rpL.getPerPassBuffer();
		// try block to ensure presentation to free swapchain image if something goes wrong.
		try
		{
		// default draw initial commands
		if(rpL._updateViewport) 
		{
			// experimental. never tried dynamic viewports before. i didn't need it yet.
			//std::vector<VkViewport> viewPorts = _grPipe->viewPorts();
			//vkCmdSetViewport(renderCmd,0,viewPorts.size(), viewPorts.data());
		}
		//rpL.addPostRenderCmds(postRenderCmds);//, _renderLocks);



		_submitInfosCreator_t sIC{ rpL, perPassWrites, renderIndex, _prevAllRF(), _allRF(),
			_reservedSems, 
			renderWaitFlags, renderWaitSems };

		std::array<std::vector<VkSubmitInfo>,pipeGroupCount> submitInfos(sIC(
			ct_size_series_type<0,pipeGroupCount>{}));
		// custom draw commands

		_drawPuller_t dP{rpL, drawPulls};
		dP(ct_size_series_type<0,pipeGroupCount>{});
		// end of draw commands
		
		std::array<std::vector<VkCommandBuffer>, pipeGroupCount> renderCmds;
		for(size_t i = 0; i < pipeGroupCount; ++i)
		{
			renderCmds[i].reserve(4);
			renderCmds[i].push_back(rpL.getCmdBuffer(i));
		}
		
		
		// post render copy requests
		for(size_t i = 0; i < pipeGroupCount; ++i)
		{
			for( auto &proc : postRenderCopyRequests[i] )
			{
				proc(renderCmds[i], postRenderCmds[i], rpL);
			}
		}
		rpL.endRecording();
		//
		// submitting 
		//
		VkSemaphore *prevRDSPtr{};
		std::array<VkSubmitInfo,pipeGroupCount> renderSIs{};
		std::vector<VkSubmitInfo> allSubmitInfos;
		allSubmitInfos.reserve(pipeGroupCount*(3+10));
		for(size_t i = 0; i < pipeGroupCount; ++i)
		{
			allSubmitInfos.insert(allSubmitInfos.end(), submitInfos[i].begin(), submitInfos[i].end());
			if(preRenderCmds[i].size())
			{
				if(i != 0)
				{
					preRenderWaitFlags[i].emplace_back(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
					preRenderWaitSems[i].emplace_back(*prevRDSPtr);
				}
				VkSubmitInfo preRenderSI{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
				renderWaitFlags[i].emplace_back(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT); // flag for next semaphore
				VkSemaphore &preRenderFinishedSemaphore = addSemaphore(renderWaitSems[i]);

				preRenderSI.waitSemaphoreCount = preRenderWaitSems[i].size();
				preRenderSI.pWaitSemaphores = preRenderWaitSems[i].data();
				preRenderSI.pWaitDstStageMask = preRenderWaitFlags[i].data();
				preRenderSI.signalSemaphoreCount = 1;
				preRenderSI.pSignalSemaphores = &preRenderFinishedSemaphore;
				preRenderSI.pCommandBuffers = preRenderCmds[i].data();
				preRenderSI.commandBufferCount = preRenderCmds[i].size();

				allSubmitInfos.push_back(preRenderSI);
			}
			else if(i != 0)
			{
				renderWaitFlags[i].emplace_back(VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
				renderWaitSems[i].emplace_back(*prevRDSPtr);
			}
			auto &renderSI = renderSIs[i];
			renderSI.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			renderSI.waitSemaphoreCount = renderWaitSems[i].size();
			renderSI.pWaitSemaphores = renderWaitSems[i].data();
			renderSI.pWaitDstStageMask = renderWaitFlags[i].data();
			renderSI.pCommandBuffers = renderCmds[i].data();//&rpL.getCmdBuffer(i);
			renderSI.commandBufferCount = renderCmds[i].size();//1;
			if(postRenderCmds[i].size())
			{
				if( i < pipeGroupCount - 1 )
				{
					// not the last rendering pipeline.
					prevRDSPtr = &addSemaphore(postRenderSignalSems[i]);
				}
				postRenderWaitFlags[i].emplace_back(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT); // flag for next semaphore
				renderSignalSems[i].push_back(addSemaphore(postRenderWaitSems[i]));

				VkSubmitInfo postRenderSI{ VK_STRUCTURE_TYPE_SUBMIT_INFO };

				renderSI.signalSemaphoreCount = renderSignalSems[i].size();
				renderSI.pSignalSemaphores = renderSignalSems[i].data();

				postRenderSI.waitSemaphoreCount = postRenderWaitSems[i].size();
				postRenderSI.pWaitSemaphores = postRenderWaitSems[i].data();
				postRenderSI.pWaitDstStageMask = postRenderWaitFlags[i].data();
				postRenderSI.signalSemaphoreCount = postRenderSignalSems[i].size();
				postRenderSI.pSignalSemaphores = postRenderSignalSems[i].data();
				postRenderSI.pCommandBuffers = postRenderCmds[i].data();
				postRenderSI.commandBufferCount = postRenderCmds[i].size();

				allSubmitInfos.push_back(renderSI);
				allSubmitInfos.push_back(postRenderSI);
			}
			else
			{
				if( i < pipeGroupCount - 1 )
				{
					// not the last rendering pipeline.
					prevRDSPtr = &addSemaphore(renderSignalSems[i]);
				}
				//signalSemaphores.push_back(presentWaitSem);
				renderSI.signalSemaphoreCount = renderSignalSems[i].size();
				renderSI.pSignalSemaphores = renderSignalSems[i].data();
				allSubmitInfos.push_back(renderSI);
			}
		}

		VkResult subres = _appInstance->submitToGraphics(allSubmitInfos.size(), allSubmitInfos.data(), _allRF()->getHandle());
		_allRF()->setIsSubmitted(renderIndex);
		{
		}
		if(subres != VK_SUCCESS)
		{
			throw rtrc::graphicalError("could not submit queue");
		}
		}
		catch(...)
		{
			//printf("error occurred when calling presenting");
			_appInstance->waitForGraphicsIdle();
			// image has to be returned to the swapchain.
			mainLogger.logPerformance("presenting image");
			_appInstance->singlePresent(rpL.getLock(), rpL.getImageIndex(), 1, &presentWaitSem);
			mainLogger.logPerformance("image presented");
			std::rethrow_exception(std::current_exception()); // rethrow skips the belw presentation
		}
		try
		{
			mainLogger.logPerformance("presenting image");
			_appInstance->singlePresent(rpL.getLock(), rpL.getImageIndex(), 1, &presentWaitSem);
			mainLogger.logPerformance("image presented");
		}
		catch(presentationError &err)
		{
			_appInstance->waitForGraphicsIdle();
			// do nothing
		}
		rpL.unlock();

		{
			/**************************************************************
			 * the current version is made to have only one thread running 
			 * graphicsPull so no mutex locking is required.
			 **************************************************************/
			_cleanupFence((_reusableFenceSelection + 2)%reusableCount);
			_reFillReservedSems();
			_addCurrentFenceDump(std::move(preRenderWaitSems), std::move(renderWaitSems),
				std::move(postRenderWaitSems), std::move(extraSems));
			_nextRenderWaitSem = nextRenderWaitSem;
			_renderPullQueueEndCV.notify_all();
		}
		return ;
	}
	// rendering failed. free all resources and rethrow the exception.
	catch(...)
	{
		_appInstance->waitForGraphicsIdle();
		for( auto &semArr : preRenderWaitSems )
		{
			for( auto &sem : semArr )
			{
				vkDestroySemaphore(hLogicalDevice, sem, pAllocator);
			}
		}
		for( auto &sem : extraSems )
		{
			vkDestroySemaphore(hLogicalDevice, sem, pAllocator);
		}
		for( auto &semArr : postRenderWaitSems)
		{
			for( auto &sem : semArr )
			{
				vkDestroySemaphore(hLogicalDevice, sem, pAllocator);
			}
		}
		//for(auto &sem : signalSemaphores)
		//{
		//	vkDestroySemaphore(hLogicalDevice, sem, pAllocator);
		//}
		for(auto &semArr : renderWaitSems)
		{
			for(auto &sem : semArr)
			{
				vkDestroySemaphore(hLogicalDevice, sem, pAllocator);
			}
		}
		_nextRenderWaitSem = VK_NULL_HANDLE; // not sure what to do about this.

		_cleanupRenderQueue();
		std::rethrow_exception(std::current_exception());
	}
}

/* queues pre render commands. pre render commands, run after the previous 
 * render or post render commands have completed their excetion
*/
template<typename appInstanceT, typename ...grPipeGroupTs>
inline void mainGraphicsPuller<appInstanceT, grPipeGroupTs...>::queuePreRenderCmd(size_t pipeIndex, const std::vector<VkCommandBuffer> &cmdBuff, std::vector<VkSemaphore> &&preRenderWaitSems,
	const std::vector<VkPipelineStageFlags> &preRenderWaitFlags)
{
	if(pipeIndex >= pipeGroupCount || preRenderWaitFlags.size() != preRenderWaitSems.size() || cmdBuff.size() == 0 ) throw graphicalError("invalid arguments in queuePreRenderCmd");
	std::lock_guard<std::mutex> lk(_renderPullQueueMutex);
	//_preRenderCmds.push_back(cmdBuff);
	_queuePreRenderCmd(pipeIndex, cmdBuff, std::move(preRenderWaitSems),preRenderWaitFlags);
}

/* queues post render commands. they are executed before the next pre 
 * render or render command.
 */
template<typename appInstanceT, typename ...grPipeGroupTs>
inline void mainGraphicsPuller<appInstanceT, grPipeGroupTs...>::queuePostRenderCmd(size_t pipeIndex, const std::vector<VkCommandBuffer> &cmdBuff, std::vector<VkSemaphore> &&postRenderWaitSems,
const std::vector<VkPipelineStageFlags> &postRenderWaitFlags)
{
	if(pipeIndex >= pipeGroupCount || postRenderWaitFlags.size() != postRenderWaitSems.size() || cmdBuff.size() == 0 ) throw graphicalError("invalid arguments in queuePostRenderCmd");
	std::lock_guard<std::mutex> lk(_renderPullQueueMutex);
	_queuePostRenderCmd(pipeIndex, cmdBuff,std::move(postRenderWaitSems),postRenderWaitFlags);
}

/* queues a copy request of the render results. in the current version the 
 * creator of the request is responsible to access the content of pipeIndex 
 * only. this is unsafe and will be fixed in future.
 * 
 * these copy requests can be inserted in the back or front of the post 
 * render commands depending on the callback.
 */
template<typename appInstanceT, typename ...grPipeGroupTs>
inline auto mainGraphicsPuller<appInstanceT, grPipeGroupTs...>::queueAndWaitPostRenderCopyRequest(size_t pipeGroupIndex, rvkPostRenderCopyRequest_t<grPipeGroupTs...> prcr)
{
	auto hLogicalDevice = _appInstance->getLogicalDevice();
	mainLogger.logMutexLock("locking _renderPullQueueMutex in queueAndWaitPostRenderCopyRequest call");
	std::unique_lock<std::mutex> lk(_renderPullQueueMutex);
	_postRenderCopyRequests[pipeGroupIndex].push_back(prcr);
	mainLogger.logMutexLock("waiting for _nextAllRF() in queueAndWaitPostRenderCopyRequest call");
	size_t index = _nextAllRF()->releaseAndWaitFor(lk, std::chrono::seconds(1));
	mainLogger.logMutexLock("waiting for _nextAllRF() in queueAndWaitPostRenderCopyRequest call ended");
	return index;
}

// untested function. waits for a render pull to start.
template<typename appInstanceT, typename ...grPipeGroupTs>
inline void mainGraphicsPuller<appInstanceT, grPipeGroupTs...>::waitForARenderPullStart() 
{
	std::unique_lock<std::mutex> lk(_renderPullQueueMutex);
	_renderPullQueueStartCV.wait(lk);
	// free to proceed.
}

// untested function. waits for a render pull to end.
template<typename appInstanceT, typename ...grPipeGroupTs>
inline void mainGraphicsPuller<appInstanceT, grPipeGroupTs...>::waitForARenderPullEnd()
{
	std::unique_lock<std::mutex> lk(_renderPullQueueMutex);
	_renderPullQueueEndCV.wait(lk);
}

// gets the current fence. and accesses it's info in a thread safe manner.
// note : don't keep the lock for a long time to not block rendering.
template<typename appInstanceT, typename ...grPipeGroupTs>
inline std::shared_ptr<const rvkFence> mainGraphicsPuller<appInstanceT, grPipeGroupTs...>::getCurrentFence(std::unique_lock<std::mutex> &outlk) const noexcept
{
	outlk = std::unique_lock<std::mutex>(_renderPullQueueMutex);
	return _allRF();
}

// getCurrentFence overload
template<typename appInstanceT, typename ...grPipeGroupTs>
inline void mainGraphicsPuller<appInstanceT, grPipeGroupTs...>::getCurrentFence(std::shared_ptr<const rvkFence> &ret, std::unique_lock<std::mutex> &outlk) const noexcept
{
	outlk = std::unique_lock<std::mutex>(_renderPullQueueMutex);
	ret = _allRF();
}

// getCurrentFence overload
template<typename appInstanceT, typename ...grPipeGroupTs>
inline const auto &mainGraphicsPuller<appInstanceT, grPipeGroupTs...>::getCurrentFenceRef(std::unique_lock<std::mutex> &outlk) const noexcept
{
	outlk = std::unique_lock<std::mutex>(_renderPullQueueMutex);
	return *_allRF().get();
}

// simple utility function 
template<typename appInstanceT, typename ...grPipeGroupTs>
inline void mainGraphicsPuller<appInstanceT, grPipeGroupTs...>::_queuePostRenderCmd(size_t pipeIndex, const std::vector<VkCommandBuffer> &cmdBuff, std::vector<VkSemaphore> &&postRenderWaitSems,
const std::vector<VkPipelineStageFlags> &postRenderWaitFlags)//, std::vector<std::unique_lock<std::mutex>> &&finishLocks = {})
{
	_postRenderCmds[pipeIndex].insert(_postRenderCmds[pipeIndex].end(), cmdBuff.begin(), cmdBuff.end());
	_postRenderWaitSems[pipeIndex].insert(_postRenderWaitSems[pipeIndex].end(), postRenderWaitSems.begin(), postRenderWaitSems.end());
	_postRenderWaitFlags[pipeIndex].insert(_postRenderWaitFlags[pipeIndex].end(), postRenderWaitFlags.begin(), postRenderWaitFlags.end());
	//_renderLocks.insert(_renderLocks.end(), finishLocks.begin(), finishLocks.end());
}

// simple utility function
template<typename appInstanceT, typename ...grPipeGroupTs>
inline void mainGraphicsPuller<appInstanceT, grPipeGroupTs...>::_queuePreRenderCmd(size_t pipeIndex, const std::vector<VkCommandBuffer> &cmdBuff, std::vector<VkSemaphore> &&preRenderWaitSems,
	const std::vector<VkPipelineStageFlags> &preRenderWaitFlags)
{
	//_preRenderCmds.push_back(cmdBuff);
	_preRenderCmds[pipeIndex].insert(_preRenderCmds[pipeIndex].end(), cmdBuff.begin(), cmdBuff.end());
	_preRenderWaitSems[pipeIndex].insert(_preRenderWaitSems[pipeIndex].end(), preRenderWaitSems.begin(), preRenderWaitSems.end());
	_preRenderWaitFlags[pipeIndex].insert(_preRenderWaitFlags[pipeIndex].end(), preRenderWaitFlags.begin(), preRenderWaitFlags.end());
}

// adds a local semaphore to a semaphore vector. maybe replaced with better
// alternatives in future.
template<typename appInstanceT, typename ...grPipeGroupTs>
inline VkSemaphore &mainGraphicsPuller<appInstanceT, grPipeGroupTs...>::addSemaphore(std::vector<VkSemaphore> &vec)
{
	auto hLogicalDevice = _appInstance->getLogicalDevice();
	auto pAllocator = _appInstance->getpAllocator();

	VkSemaphoreCreateInfo semCI { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO}; 

	VkSemaphore tmpSem = VK_NULL_HANDLE;
	if(vkCreateSemaphore(hLogicalDevice, &semCI, pAllocator, &tmpSem)
		!= VK_SUCCESS)
	{
		// there is no need to destroy, nothing was created.
		throw rtrc::graphicalError("could not create semaphore");
	}
	//vec.push_back(tmpSem);
	return vec.emplace_back(tmpSem);//vec[vec.size() - 1];
}

/* mutex lock needed : _renderPullQueueMutex
 * after calling this, the new fence may be selected with _allRF()
 * it should be noted that after submitting, _allRF()->setIsSubmitted must 
 * be called. 
 */ 
template<typename appInstanceT, typename ...grPipeGroupTs>
inline void mainGraphicsPuller<appInstanceT, grPipeGroupTs...>::_selectNextFence()
#ifdef NDEBUG
noexcept
#endif
{
	_reusableFenceSelection = ++_reusableFenceSelection%reusableCount;
#ifndef NDEBUG
	if(_reusableDumpSelection[_reusableFenceSelection].has_value())
		throw graphicalError("trying to use a fence that it's queue is not cleaned up\n");
#endif
}

// relates semaphores to a specific fence
// call only once for each fence
// mutex lock needed : _renderPullQueueMutex(if the render pulls are not in a single thread)
template<typename appInstanceT, typename ...grPipeGroupTs>
inline void mainGraphicsPuller<appInstanceT, grPipeGroupTs...>::_addCurrentFenceDump( std::array<std::vector<VkSemaphore>,pipeGroupCount> &&prevPreRenderWaitSems ,
std::array<std::vector<VkSemaphore>,pipeGroupCount> &&prevRenderWaitSems,
std::array<std::vector<VkSemaphore>,pipeGroupCount> &&prevPostRenderWaitSems,
std::vector<VkSemaphore> &&prevExtraSems )
#ifdef NDEBUG
noexcept
#endif
{
	// find a dump index.
	size_t i = 0;
	for(; i < prevDumpCount; ++i)
	{
		if(_prevCleanedUp[i]) break;
	}
	_prevCleanedUp[i] = false;
#ifndef NDEBUG
	if(i > prevDumpCount)
		throw graphicalError("could not find dump index");
	if(_reusableDumpSelection[_reusableFenceSelection].has_value())
		throw graphicalError("_addCurrentFenceDump called on fence that's not cleaned up");
#endif
	_reusableDumpSelection[_reusableFenceSelection] = i;
	for(size_t j = 0; j < pipeGroupCount; ++j)
	{
		// i is dump selection index and j is pipe index.
		_prevPreRenderWaitSems[i].insert(_prevPreRenderWaitSems[i].begin(), 
			prevPreRenderWaitSems[j].begin(), prevPreRenderWaitSems[j].end());
		prevPreRenderWaitSems[j].clear();
		_prevRenderWaitSems[i].insert(_prevRenderWaitSems[i].begin(),
			prevRenderWaitSems[j].begin(), prevRenderWaitSems[j].end());
		prevRenderWaitSems[j].clear();
		_prevPostRenderWaitSems[i].insert(_prevPostRenderWaitSems[i].begin(),
			prevPostRenderWaitSems[j].begin(), prevPostRenderWaitSems[j].end());
		prevPostRenderWaitSems[j].clear();
	}
	_prevExtraSems[i] = std::move(prevExtraSems);
}

// cleans up a fence
// mutex lock needed : _renderPullQueueMutex(if the render pulls are not in a single thread)
template<typename appInstanceT, typename ...grPipeGroupTs>
inline void mainGraphicsPuller<appInstanceT, grPipeGroupTs...>::_cleanupFence(size_t selection)
{
	auto hLogicalDevice = _appInstance->getLogicalDevice();
	auto pAllocator = _appInstance->getpAllocator();
	if(_reusableDumpSelection[selection].has_value())
	{
		// threads are free to start waiting for the fence submission, no 
		// lock needed. other values are locked with _renderPullQueueMutex
		//_nextAllRF().reset(); 
		_reusableFences[selection]->reset();
		// fence is reset(and is waited for before reset if needed)
		auto dumpS = _reusableDumpSelection[selection].value();
		for(auto &sem : _prevPreRenderWaitSems[dumpS])
		{
			vkDestroySemaphore(hLogicalDevice, sem, pAllocator);
		}
		_prevPreRenderWaitSems[dumpS].clear();
		for(auto &sem : _prevRenderWaitSems[dumpS])
		{
			vkDestroySemaphore(hLogicalDevice, sem, pAllocator);
		}
		_prevRenderWaitSems[dumpS].clear();
		for(auto &sem : _prevPostRenderWaitSems[dumpS])
		{
			vkDestroySemaphore(hLogicalDevice, sem, pAllocator);
		}
		_prevPostRenderWaitSems[dumpS].clear();
		for(auto &sem : _prevExtraSems[dumpS] )
		{
			vkDestroySemaphore(hLogicalDevice, sem, pAllocator);
		}
		_prevExtraSems[dumpS].clear();		
		_prevCleanedUp[dumpS] = true;
		_reusableDumpSelection[selection].reset();
	}
}

// cleans up the entire render queue and some other resources. commonly used for swapchain reset to 
//make sure resources are not in use by command buffers.
// mutex lock needed : _renderPullQueueMutex(if the render pulls are not in a single thread)
template<typename appInstanceT, typename ...grPipeGroupTs>
inline void mainGraphicsPuller<appInstanceT, grPipeGroupTs...>::_cleanupRenderQueue() noexcept 
{
	const auto &hLogicalDevice = _appInstance->getLogicalDevice();
	const auto &pAllocator = _appInstance->getpAllocator();
	for(size_t i = 0; i < reusableCount; ++i)
	{
		_cleanupFence(i);
	}

	if(_nextRenderWaitSem != VK_NULL_HANDLE)
	{
		vkDestroySemaphore(hLogicalDevice, _nextRenderWaitSem, pAllocator);
		_nextRenderWaitSem = VK_NULL_HANDLE;
	}
}

template<typename appInstanceT, typename ...grPipeGroupTs>
inline auto mainGraphicsPuller<appInstanceT, grPipeGroupTs...>::_prevFenceSelection() const noexcept { return (_reusableFenceSelection + reusableCount - 1)%reusableCount; }
template<typename appInstanceT, typename ...grPipeGroupTs>
inline auto mainGraphicsPuller<appInstanceT, grPipeGroupTs...>::_nextFenceSelection() const noexcept { return (_reusableFenceSelection + 1)%reusableCount; }
template<typename appInstanceT, typename ...grPipeGroupTs>
inline auto &mainGraphicsPuller<appInstanceT, grPipeGroupTs...>::_allRF() noexcept { return _reusableFences[_reusableFenceSelection]; }
template<typename appInstanceT, typename ...grPipeGroupTs>
inline const auto &mainGraphicsPuller<appInstanceT, grPipeGroupTs...>::_allRF() const noexcept { return _reusableFences[_reusableFenceSelection]; }
template<typename appInstanceT, typename ...grPipeGroupTs>
inline auto &mainGraphicsPuller<appInstanceT, grPipeGroupTs...>::_prevAllRF() noexcept { return _reusableFences[_prevFenceSelection()]; }
template<typename appInstanceT, typename ...grPipeGroupTs>
inline auto &mainGraphicsPuller<appInstanceT, grPipeGroupTs...>::_nextAllRF() noexcept { return _reusableFences[_nextFenceSelection()]; }

template<typename appInstanceT, typename ...grPipeGroupTs>
template<size_t increaseSizeIfEmpty>
inline void mainGraphicsPuller<appInstanceT, grPipeGroupTs...>::_reFillReservedSems()
{
	auto hLogicalDevice = _appInstance->getLogicalDevice();
	auto pAllocator = _appInstance->getpAllocator();
	VkSemaphoreCreateInfo semCI{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
	if(increaseSizeIfEmpty)
	{
		if(_reservedSems.size() == 0)
		{
			_reservedSemsStorageSize += increaseSizeIfEmpty;
		}
	}
	try
	{
		size_t refillStartIndex = _reservedSems.size();
		_reservedSems.resize(_reservedSemsStorageSize, VK_NULL_HANDLE);
		for( size_t i = refillStartIndex; i < _reservedSemsStorageSize; ++i)
		{
			if( vkCreateSemaphore(hLogicalDevice, &semCI, pAllocator, &_reservedSems[i]) != VK_SUCCESS)
				throw graphicalError(" could not create semaphore for reservation");
		}
	}
	catch(...)
	{
		for( auto &sem : _reservedSems )
		{
			vkDestroySemaphore(hLogicalDevice, sem, pAllocator);
		}
		_reservedSems.clear();
		std::rethrow_exception(std::current_exception());
	}
}
	
	


inline rvkStagedImage::rvkStagedImage(const std::shared_ptr<vulkanAppInstance> &appInstance,
const size_t &width, const size_t &height, const size_t &planeCount, const VkFormat &format, VkImageTiling tiling,
VkSharingMode sharingMode, VkImageUsageFlags usageFlags)
	: rvkImage(appInstance, width, height, planeCount, format, tiling, sharingMode,
	usageFlags|VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
	_coherent_buff(appInstance->getLogicalDevice(), appInstance->getpAllocator(), _byteSize,
	appInstance->getDeviceMemoryProps(), sharingMode, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
{
	const auto &hLogicalDevice = _appInstance->getLogicalDevice();
	const auto &pAllocator = _appInstance->getpAllocator();
	VkEventCreateInfo eCI{ VK_STRUCTURE_TYPE_EVENT_CREATE_INFO};
	_coherent_buff.map();
	try
	{
		if( vkCreateEvent(hLogicalDevice, &eCI, pAllocator, &_coherentCopyIsDone)
			!= VK_SUCCESS)
			throw graphicalError("could not create VkEvent");
		//vkSetEvent(hLogicalDevice, _coherentCopyIsDone);
	}
	catch(...)
	{
		vkDestroyEvent(hLogicalDevice, _coherentCopyIsDone, pAllocator);
	}
}

inline rvkStagedImage::rvkStagedImage(const std::shared_ptr<vulkanAppInstance> &appInstance,
const rvkColorPlaneLayout_t &cpl, const rvkFormatInterface &rvkFI, VkImageTiling tiling,
VkSharingMode sharingMode, VkImageUsageFlags usageFlags)
	: rvkStagedImage(appInstance, cpl._width, cpl._height, cpl._planeCount, rvkFI._vkformat,
	tiling, sharingMode, usageFlags)
{

}

inline rvkStagedImage::rvkStagedImage( rvkStagedImage &&other)
	: rvkImage(std::move(other)), _coherent_buff(std::move(other._coherent_buff)),
	_coherentCopyIsDone(other._coherentCopyIsDone), _deviceLoadNotCalled(other._deviceLoadNotCalled)
{
	other._coherentCopyIsDone = VK_NULL_HANDLE;
}

inline rvkStagedImage::~rvkStagedImage()
{
	const auto &hLogicalDevice = _appInstance->getLogicalDevice();
	const auto &pAllocator = _appInstance->getpAllocator();
	vkDestroyEvent(hLogicalDevice, _coherentCopyIsDone, pAllocator);
}

/* loads the image from the CPU side into a host_coherent and host_visible memory.
 * 
 * synchornization : 
 * may be called from any thread.
 * image buffer shouldn't be in use(device load command should've completed 
 * execution). or there will be extra delay.
 */
template<typename loaderCallableT> // a callable with a void * argument.
inline void rvkStagedImage::loadStaged_host(const loaderCallableT &loaderCallable)
{
#ifndef NDEBUG
	if(!_coherentCopyIsDone)
		throw graphicalError("operation on a moved object");
#endif
	const auto &hLogicalDevice = this->_appInstance->getLogicalDevice();

	std::unique_lock<std::mutex> lk(this->_gm);
	if(!_deviceLoadNotCalled) // a loadStaged_device has been called on this data.
	{
		/* note : for the first call, the event is constructed in a rest 
		 * state and _deviceLoadNotCalled is false
		 */
		// this will be inefficient if it gets blocked.
		while(vkGetEventStatus(hLogicalDevice, this->_coherentCopyIsDone)
			!= VK_EVENT_RESET)
		{
			lk.unlock();
			std::this_thread::sleep_for(std::chrono::microseconds(500));
			lk.lock();
		}
		loaderCallable(this->_coherent_buff._pData);
		vkSetEvent(hLogicalDevice, _coherentCopyIsDone);
		_deviceLoadNotCalled = true;
		//throw rNotYetException("attempting to overwrite unloaded data.");
	}
	else // safely overriding the existing data.
	{
		// event is not going to be reset by the gpu
		loaderCallable(this->_coherent_buff._pData);
		// event is already set.
		// _transferRecordIsNeeeded is already true
	}
	_initialHostLoad = false; // there was no failure.
}

/* loads the image from the CPU side into a host_coherent and host_visible memory.
 * fails if called before any call to loadStaged_host after construction.
 * tries to safely override with data if it wasn't referenced in the device.
 * synchornization : 
 * may be called from any thread.
 * return : true of attempt was successful, false if it wasn't.
 */
template<typename loaderCallableT> // a callable with a void * argument.
inline bool rvkStagedImage::try_overwrite_loadStaged_host(const loaderCallableT &loaderCallable)
{
#ifndef NDEBUG
	if(!_coherentCopyIsDone)
		throw graphicalError("operation on a moved object");
#endif
	const auto &hLogicalDevice = this->_appInstance->getLogicalDevice();

	std::lock_guard<std::mutex> lk(this->_gm);
	if(_deviceLoadNotCalled) // a loadStaged_device has not been called on this data.
	{
		// safely overriding the existing data.
		// event is not going to be reset by the gpu
		loaderCallable(this->_coherent_buff._pData);
		// event is already set.
		// _transferRecordIsNeeeded is already true
	}
#ifndef NDEBUG
	else
	{
		if(_initialHostLoad)
			throw invalidCallState("rvkStagedImage::try_overwrite_loadStaged_host called before any call to rvkStagedImage::loadStaged_host");
	}
#endif
	return _deviceLoadNotCalled;
}


/* records a device load command into the command buffer if it has not been
 * recorded before. a faster alternative is to use 
 * preRecord_loadStaged_device once and use informDeviceLoad() instead.
 * return : true if a recording was done on the command buffer.
 * dev note : do not include internal command buffers as access to them
 * cannot be synchronized in a flexible way.
 */
inline void rvkStagedImage::loadStaged_device_cmd(const VkCommandBuffer &cmdBuffer, const rvkBufferImageCopyInfo &bici, 
VkImageLayout srcImageLayout,
	VkPipelineStageFlags srcStage,
	VkImageLayout dstImageLayout,
	VkPipelineStageFlags dstStage)
{
	std::lock_guard<std::mutex> lk(_gm);
#ifndef NDEBUG
	if(!_coherentCopyIsDone)
		throw graphicalError("operation on a moved object");
#endif
	if(_deviceLoadNotCalled)
	{
		_recordStagingCmd(cmdBuffer, bici, srcImageLayout, srcStage, dstImageLayout, dstStage);
		_deviceLoadNotCalled = false;
	}
}

// use with care (experimental).
// pre precords loadStaged_device_cmd command, use in junction with informDeviceLoad().
inline void rvkStagedImage::preRecord_loadStaged_device(const VkCommandBuffer &cmdBuffer, const rvkBufferImageCopyInfo &bici, 
VkImageLayout srcImageLayout,
	VkPipelineStageFlags srcStage,
	VkImageLayout dstImageLayout,
	VkPipelineStageFlags dstStage)
{
#ifndef NDEBUG
	if(!_coherentCopyIsDone)
		throw graphicalError("operation on a moved object");
#endif
	_recordStagingCmd(cmdBuffer, bici, srcImageLayout, srcStage, dstImageLayout, dstStage);
}

/* use with care(experimental).
 * informs of attempt of the device to use the image.
 * return true if the caller must submit a transfer command. 
 */
inline bool rvkStagedImage::informDeviceLoad() noexcept
{
	std::lock_guard<std::mutex> lk(_gm);
#ifndef NDEBUG
	if(!_coherentCopyIsDone)
		throw graphicalError("operation on a moved object");
#endif
	bool ret = _deviceLoadNotCalled;
	_deviceLoadNotCalled = false;
	return ret;
}

// for updating samplers.
inline const auto &rvkStagedImage::getImgHandle() { return _hImage; }

inline const rvkImage &rvkStagedImage::getRvkImage() { return *this; }

inline void rvkStagedImage::_init()
{
}

inline void rvkStagedImage::_recordStagingCmd(const VkCommandBuffer &cmdBuffer, const rvkBufferImageCopyInfo &bici, 
VkImageLayout srcImageLayout,
	VkPipelineStageFlags srcStage,
	VkImageLayout dstImageLayout,
	VkPipelineStageFlags dstStage)
{
	VkImageMemoryBarrier imb { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
	imb.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	imb.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imb.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	std::vector<VkImageMemoryBarrier> imbs; imbs.reserve(3);
	imb.oldLayout = srcImageLayout;
	imb.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	rvkImage::addBarriers_allPlanes(imbs, imb);
	vkCmdWaitEvents(cmdBuffer, 1, &_coherentCopyIsDone, //srcStage|
		/*VK_PIPELINE_STAGE_TRANSFER_BIT/*for transfer without render*///|
		VK_PIPELINE_STAGE_HOST_BIT/*for host event manipulation*/, 
		VK_PIPELINE_STAGE_TRANSFER_BIT, 0, nullptr, 0, nullptr, 0, nullptr);
	vkCmdPipelineBarrier(cmdBuffer, srcStage |
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 
		VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr,
		imbs.size(), imbs.data());

	rvkCmdCopyBufferToImage(cmdBuffer, bici, _coherent_buff._hBuffer, 
		_hImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	if(dstImageLayout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		imbs.clear();
		imb.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imb.newLayout = dstImageLayout;
		imb.srcAccessMask = imb.dstAccessMask;
		imb.dstAccessMask = 0;
		rvkImage::addBarriers_allPlanes(imbs, imb);
		vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
			dstStage,
			0, 0, nullptr, 0, nullptr, imbs.size(), imbs.data());
	}
	// vkCmdResetEvent : stageMask must not include VK_PIPELINE_STAGE_HOST_BIT
	vkCmdResetEvent(cmdBuffer, _coherentCopyIsDone, VK_PIPELINE_STAGE_TRANSFER_BIT/*dstStage*/);

}

// currently unused.	
/*struct rvkDrawIndirectCommand {
uint32_t    vertexCount;
uint32_t    instanceCount;
uint32_t    firstVertex;
uint32_t    firstInstance;
};*/
}
}