
#pragma once
#include "decs/rvulkan_core_decs.h"
#include "../rtrc.h"
#include "../rstring.h"

namespace rtrc { namespace vkl {
	


static constexpr VkImageAspectFlags rvkPlaneFlags[3] = { 
		VK_IMAGE_ASPECT_PLANE_0_BIT, VK_IMAGE_ASPECT_PLANE_1_BIT, VK_IMAGE_ASPECT_PLANE_2_BIT
	};



inline void rvkColorPlaneLayout_t::copyTo(void *dst)
{
	size_t offset = 0;
	for(size_t i = 0; i < _planeCount; ++i)
	{
		size_t copySize = _linesizes[i]*_planeHeights[i];
		mainLogger.logTraffic_generic(Rsnprintf<char,100>("cpl loading %llu data", copySize));
		memcpy((uint8_t *)dst + offset, _rvkPlaneData[i], copySize);
		offset += copySize;
	}
}

inline uint32_t rvkFindMemoryProperties(uint32_t memoryTypeBits, VkMemoryPropertyFlags reqPropFlags,
	 const VkPhysicalDeviceMemoryProperties &deviceMemProps)
{
	for(uint32_t i = 0; i < deviceMemProps.memoryTypeCount; ++i)
	{
		// memoryTypeBits will be tagged with the supported indexes, with the smallest index being the best option.
		if((memoryTypeBits & (1 << i)) && (deviceMemProps.memoryTypes[i].propertyFlags & reqPropFlags) == reqPropFlags)
		{
			return i;
		}
	}
	throw graphicalError("requested device memory properties not found");
}


inline ApplicationInfo::ApplicationInfo() noexcept : VkApplicationInfo{VK_STRUCTURE_TYPE_APPLICATION_INFO} {}

inline DebugUtilsMessengerCreateInfoEXT::DebugUtilsMessengerCreateInfoEXT() noexcept 
	: 
	VkDebugUtilsMessengerCreateInfoEXT{VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT} 
{
}

inline DeviceQueueCreateInfo::DeviceQueueCreateInfo() noexcept : VkDeviceQueueCreateInfo{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO}
{
}


inline SwapchainCreateInfoKHR::SwapchainCreateInfoKHR() noexcept : VkSwapchainCreateInfoKHR{VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR} {}


inline swapChainSupportDetails::swapChainSupportDetails(VkPhysicalDevice physicalDevice, VkSurfaceKHR khrSurface) noexcept
{
	reInitialize(physicalDevice, khrSurface);
}
	
inline void swapChainSupportDetails::reInitialize(VkPhysicalDevice physicalDevice, VkSurfaceKHR khrSurface) noexcept
{
	resetCapabilities(physicalDevice, khrSurface);
	{
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, khrSurface, &formatCount, nullptr);
		if(formatCount != 0)
		{
			_formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, khrSurface, &formatCount, _formats.data());
		}
	}
	{
		uint32_t presentModesCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, khrSurface, &presentModesCount, nullptr);
		if(presentModesCount != 0)
		{
			_presentModes.resize(presentModesCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, khrSurface, &presentModesCount, _presentModes.data());

		}
	}
}
	
// efficiency in these functions is unimportant
inline bool swapChainSupportDetails::hasFormat(const VkSurfaceFormatKHR &reqFormat) const noexcept
{
	for(const auto &f : _formats)
	{
		if (f.colorSpace == reqFormat.colorSpace && f.format == reqFormat.format) return true;
	}
	return false;
}
	
inline bool swapChainSupportDetails::hasPresentationMode(VkPresentModeKHR reqpm) const noexcept
{
	for(const auto &pm : _presentModes)
	{
		if( pm == reqpm) return true;
	}
	return false;
}
	
// idk if this is needed or not.
inline void swapChainSupportDetails::resetCapabilities(VkPhysicalDevice physicalDevice, VkSurfaceKHR khrSurface) noexcept
{
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, khrSurface, &_capabilities);
}
	
// changes reqExtent if needed
inline void swapChainSupportDetails::getSwapExtent(VkExtent2D &reqExtent)
{
	if(_capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		reqExtent = _capabilities.currentExtent;
	}
	else
	{
		reqExtent.height = std::clamp(reqExtent.height, 
			_capabilities.minImageExtent.height, _capabilities.maxImageExtent.height);
		reqExtent.width = std::clamp(reqExtent.width, 
			_capabilities.minImageExtent.width, _capabilities.maxImageExtent.width);
	}
}
	
	
inline const auto &swapChainSupportDetails::getCapabilities() const noexcept { return _capabilities; }
inline const auto &swapChainSupportDetails::getFormats() const noexcept { return _formats; }
inline const auto &swapChainSupportDetails::getPresentModes() const noexcept { return _presentModes; }
inline pipeVertexInputStateCI::pipeVertexInputStateCI(pipeVertexInputStateCI &&moved)
	: _ci(moved._ci), _inputBindingDescs(std::move(moved._inputBindingDescs)), 
	_inputAttrDescs(std::move(moved._inputAttrDescs))
{

}
inline pipeVertexInputStateCI::pipeVertexInputStateCI( std::vector<VkVertexInputBindingDescription> &&inputBindingDescs,
 std::vector<VkVertexInputAttributeDescription> &&inputAttrDescs)
	: _ci{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO}, _inputBindingDescs(std::move(inputBindingDescs)) , _inputAttrDescs(std::move(inputAttrDescs))
{
	_ci.vertexBindingDescriptionCount = _inputBindingDescs.size();
	_ci.pVertexBindingDescriptions = _inputBindingDescs.data();

	_ci.vertexAttributeDescriptionCount = _inputAttrDescs.size();
	_ci.pVertexAttributeDescriptions = _inputAttrDescs.data();
}

inline const auto &pipeVertexInputStateCI::getRef() const noexcept { return _ci; }


inline pipeViewportStateCI::pipeViewportStateCI(const pipeViewportStateCI &pci)
	: _viewports(pci._viewports), _scissors(pci._scissors), _ci(pci._ci)
{
	_ci.pViewports = _viewports.data();
	_ci.pScissors = _scissors.data();
}
inline pipeViewportStateCI::pipeViewportStateCI(pipeViewportStateCI  &&pci)
	: _viewports(std::move(pci._viewports)), _scissors(std::move(pci._scissors)), _ci(pci._ci)
{
	//_ci.pViewports = _viewports.data();
	//_ci.pScissors = _scissors.data();
}

inline pipeViewportStateCI::pipeViewportStateCI(std::vector<VkViewport> &&viewports, std::vector<VkRect2D> &&scissors )
	: _viewports(viewports), _scissors(scissors), _ci{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO}
{
	_ci.pViewports = _viewports.data();
	_ci.viewportCount = _viewports.size();
	_ci.pScissors = _scissors.data();
	_ci.scissorCount = _scissors.size();
}

inline const auto &pipeViewportStateCI::getRef() const noexcept { return _ci; }


inline colorBlendAttachmentState::colorBlendAttachmentState() : VkPipelineColorBlendAttachmentState{} {}



inline colorBlendStateCI::colorBlendStateCI(colorBlendStateCI &&ci)
	: _ci(ci._ci), _attStates(std::move(ci._attStates))
{
	//_ci.olors = _attStates.data();
}



inline  colorBlendStateCI::colorBlendStateCI(std::vector<colorBlendAttachmentState> &&attStates)
	: _ci{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO}, _attStates(std::move(attStates))
{
	_ci.pAttachments = _attStates.data();
	_ci.attachmentCount = _attStates.size();
}

inline const auto &colorBlendStateCI::getRef() const noexcept { return _ci; }


inline descSetLayoutCI::descSetLayoutCI(descSetLayoutCI &&moved)
	: _ci(moved._ci), _bindings(std::move(moved._bindings))
{
}

inline descSetLayoutCI::descSetLayoutCI(std::vector<VkDescriptorSetLayoutBinding> &&bindings)
	: _bindings(std::move(bindings)), _ci { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO} 
{
	_ci.bindingCount = _bindings.size();
	_ci.pBindings = _bindings.data();
}

inline const auto &descSetLayoutCI::getRef() const noexcept { return _ci; }

inline rvkSubpassDescs::rvkSubpassDescs(rvkSubpassDescs &&subpass)
	: _attRefsArr(std::move(subpass._attRefsArr)), _subpassDescs(std::move(subpass._subpassDescs))
{
}

inline rvkSubpassDescs::rvkSubpassDescs(std::vector<rvkAttachmentReferences> &&attRefsArr)
	: _attRefsArr(std::move(attRefsArr)), _subpassDescs(_attRefsArr.size())
{
	size_t size = _attRefsArr.size();
	for(size_t i = 0; i < size; ++i )
	{
		// a subpass description for each group of framgnet's output
		_subpassDescs[i].colorAttachmentCount = _attRefsArr[i].colorAttRefs.size();
		_subpassDescs[i].pColorAttachments = _attRefsArr[i].colorAttRefs.data();

		if(_attRefsArr[i].depthAndStencilAttachment.has_value())
			_subpassDescs[i].pDepthStencilAttachment = &_attRefsArr[i].depthAndStencilAttachment.value();
	}
}

inline const VkSubpassDescription *rvkSubpassDescs::getPtr() const noexcept { return _subpassDescs.data(); }
inline auto rvkSubpassDescs::getSize() const noexcept { return _subpassDescs.size(); }


inline rvkRenderPassCI::rvkRenderPassCI(rvkRenderPassCI &&ci)
	: _colorAttDescs(std::move(ci._colorAttDescs)), _subpassDescs(std::move(ci._subpassDescs)), _ci(ci._ci)
{

}

inline rvkRenderPassCI::rvkRenderPassCI(std::vector<VkAttachmentDescription> &&colorAttDescs, std::vector<VkSubpassDependency> &&dependencies,
rvkSubpassDescs &&subpassDescs)
	: _colorAttDescs(std::move(colorAttDescs)), _dependencies(std::move(dependencies)),
	_subpassDescs(std::move(subpassDescs)), _ci{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO}
{
	// must be compatible with swapchain.
	_ci.attachmentCount = _colorAttDescs.size();
	_ci.pAttachments = _colorAttDescs.data();

	_ci.dependencyCount = _dependencies.size();
	_ci.pDependencies = _dependencies.data();

	_ci.subpassCount = _subpassDescs.getSize();
	_ci.pSubpasses = _subpassDescs.getPtr();
}

inline const VkRenderPassCreateInfo &rvkRenderPassCI::getRef() const noexcept { return _ci; }

inline rvkRenderPassBeginInfo::rvkRenderPassBeginInfo(rvkRenderPassBeginInfo &&moved)
	: _beginInfo(moved._beginInfo), _clearColors(std::move(moved._clearColors))
{
}

inline rvkRenderPassBeginInfo::rvkRenderPassBeginInfo(std::vector<VkClearValue> &&clearColors)
	: _beginInfo{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO}, _clearColors(std::move(clearColors))
{
	_beginInfo.clearValueCount = _clearColors.size();
	_beginInfo.pClearValues = _clearColors.data();
}

inline const VkRenderPassBeginInfo &rvkRenderPassBeginInfo::getRef(){ return _beginInfo;}


inline rvkWriteDescriptorSet::rvkWriteDescriptorSet( const rvkWriteDescriptorSet &copied )
	: _imgInfos(copied._imgInfos), _bufferInfos(copied._bufferInfos), _texelViews(copied._texelViews),
	_winfo(copied._winfo)
{
	_winfo.pImageInfo = _imgInfos.data();
	_winfo.pBufferInfo = _bufferInfos.data();
	_winfo.pTexelBufferView = _texelViews.data();
}

inline rvkWriteDescriptorSet::rvkWriteDescriptorSet( rvkWriteDescriptorSet &&moved)
	: _imgInfos(std::move(moved._imgInfos)), _bufferInfos(std::move(moved._bufferInfos)),
	_texelViews(std::move(moved._texelViews)), _winfo(moved._winfo)
{

}
inline rvkWriteDescriptorSet::rvkWriteDescriptorSet(std::vector<VkDescriptorImageInfo> &&imgInfos,
std::vector<VkDescriptorBufferInfo> &&bufferInfos,
std::vector<VkBufferView> &&texelViews)
	: _imgInfos(std::move(imgInfos)), _bufferInfos(std::move(bufferInfos)), 
	_texelViews(std::move(texelViews)), _winfo { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET }
{
	_winfo.pImageInfo = _imgInfos.data();
	_winfo.pBufferInfo = _bufferInfos.data();
	_winfo.pTexelBufferView = _texelViews.data();
}

inline rvkWriteDescriptorSet::rvkWriteDescriptorSet(std::vector<VkDescriptorImageInfo> &&imgInfos)
	: rvkWriteDescriptorSet(std::move(imgInfos),{}, {})
{

}

inline rvkWriteDescriptorSet::rvkWriteDescriptorSet(std::vector<VkDescriptorBufferInfo> &&bufferInfos)
	: rvkWriteDescriptorSet({},std::move(bufferInfos),{})
{
}

inline rvkWriteDescriptorSet::rvkWriteDescriptorSet(std::vector<VkBufferView> &&texelViews)
	: rvkWriteDescriptorSet({},{},std::move(texelViews))
{
}

inline VkWriteDescriptorSet &rvkWriteDescriptorSet::getRef() { return _winfo; }

inline rvkDescSameUpdater::rvkDescSameUpdater(rvkDescSameUpdater &&moved)
	: _opts(std::move(moved._opts))
{
}

inline rvkDescSameUpdater::rvkDescSameUpdater(std::vector<rvkWriteDescriptorSet> &&writes)
	: _opts(std::move(writes))
{
}

// changes custom values in descSets to real values with the help of a perPassBuffer
template<typename perPassBuffer_t>
inline void rvkDescSameUpdater::mapTo(const std::vector<VkDescriptorSet> &descSets, std::vector<rvkWriteDescriptorSet> &descWrites, const perPassBuffer_t &perIBuff) const
{
	//memcpy(pDescWrites, _opts.data(), _opts.size()*sizeof(VkWriteDescriptorSet));
	//size_t writeCount = _opts.size();

	for( rvkWriteDescriptorSet &write : descWrites )
	{
		perIBuff.mapDescSetWrite(write);
		write._winfo.dstSet = descSets[(size_t)write._winfo.dstSet];
	}
}

// updates all of the descriptor set arrays with the same options.
// all internal arrays should have the same size. the first std::vector<VkDEscriptorSet> in descSetsArr will be used as a size reference.
template<typename perPassBuffer_t>
inline void rvkDescSameUpdater::update(VkDevice device, 
const std::vector<std::vector<VkDescriptorSet>> &descSetsArr, 
const std::vector<std::shared_ptr<perPassBuffer_t>> &perIBuffs ) const
{
	// count of swapchain images
	size_t arrSize = descSetsArr.size(); // should be equal to perIBuffs.size();
#ifndef NDEBUG
	if(descSetsArr.size() != perIBuffs.size())
		throw graphicalError("rvkDescSameUpdater::update called with wrong arguments");
#endif

	if( descSetsArr.size() )
	{
		size_t descWriteCount = _opts.size();
		// writes array contains the input for vkUpdateDescriptorSets
		std::vector<VkWriteDescriptorSet> writes(descWriteCount*arrSize);
		// arrSize copies of _opts(the write data) 
		std::vector<std::vector<rvkWriteDescriptorSet>> rvkWrites(arrSize, _opts);
		// loop to iterate over rvkWrites, translate it's custom values to 
		// real ones representing the descSetsArr and perPassBuffer_t group and then iterate over 
		for(size_t i = 0; i < arrSize; ++i)
		{
			//mapTo(descSetsArr[i], &writes[i*descWriteCount], *perIBuffs[i].get());
			// updating to real values in rvkWrites[i], the descSetArr[i] and perIBuffs[i] are used as a reference
			mapTo(descSetsArr[i], rvkWrites[i], *perIBuffs[i].get());
			// inserting write data into vkUpdateDescriptorSets's input structure.
			for(size_t j = 0; j < descWriteCount; ++j)
			{
				writes[i*descWriteCount+j] = rvkWrites[i][j]._winfo;
			}
		}
		// developer note : if the software crashes on vkUpdateDescriptorSets, things like dstArrayElement may be wrong.
		vkUpdateDescriptorSets( device, writes.size(), writes.data(), 0, nullptr);
	}
}

inline bool checkVkDeviceFeatures( const VkPhysicalDeviceFeatures &avFs, 
	const VkPhysicalDeviceFeatures &reqFs) noexcept
{
	//
	// current implementation is incomplete.
	//
	if(reqFs.geometryShader)
	{
		if(!avFs.geometryShader)
			return false;
	}
	return true;
}


inline rvkPhysicalDeviceFeatures::rvkPhysicalDeviceFeatures(const rvkPhysicalDeviceFeatures &copy)
	: rvkPhysicalDeviceFeatures()
{
	_devFeatures = copy._devFeatures;
	_samplerYcbcrConversion = copy._samplerYcbcrConversion;
}



inline rvkPhysicalDeviceFeatures &rvkPhysicalDeviceFeatures::operator=(const rvkPhysicalDeviceFeatures &other)
{
	_devFeatures = other._devFeatures;
	_samplerYcbcrConversion = other._samplerYcbcrConversion;

	return *this;
}

inline rvkPhysicalDeviceFeatures::rvkPhysicalDeviceFeatures()
	: _devFeatures(_devFeatures2.features), 
	_samplerYcbcrConversion(_samplerYcbcrConversionStruct.samplerYcbcrConversion),
	_pNext(_devFeatures2.pNext)
{
	_devFeatures2.pNext = &_samplerYcbcrConversionStruct;
}

inline bool rvkPhysicalDeviceFeatures::checkFeatures(VkPhysicalDevice phyDev) const noexcept
{
	VkPhysicalDeviceSamplerYcbcrConversionFeatures 
	samplerYcbcrConversionStruct{
		VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES};
	VkPhysicalDeviceFeatures2 devFeatures2{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
	devFeatures2.pNext = &samplerYcbcrConversionStruct;

	vkGetPhysicalDeviceFeatures2( phyDev, &devFeatures2);
	if(!checkVkDeviceFeatures(devFeatures2.features, _devFeatures))
		return false;
	if(_samplerYcbcrConversion)
	{
		mainLogger.logText("checking for ycbcr support\n");
		if(samplerYcbcrConversionStruct.samplerYcbcrConversion != VK_TRUE)
			return false;
		mainLogger.logText("ycbcr supported\n");
	}
	return true;
}

inline const auto &rvkPhysicalDeviceFeatures::getFeatures2() const noexcept { return _devFeatures2; }


inline vulkanAppInstance::vulkanAppInstance(std::vector<const char *> &vkReqExtensions, 
const char **debugLayers, uint32_t debugLayerCount,
VkAllocationCallbacks *pAllocator, PFN_vkDebugUtilsMessengerCallbackEXT debugCallback)
	: _pAllocator(pAllocator)
{
	//
	// main instance configuration
	//
	ApplicationInfo appInfo;
	appInfo.pApplicationName = "default Vk App name";
	appInfo.applicationVersion = VK_MAKE_VERSION(1,1,120);
	appInfo.pEngineName = "no Vk engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1,1,120);
	appInfo.apiVersion = VK_API_VERSION_1_1;
	VkInstanceCreateInfo icinfo{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
	icinfo.pApplicationInfo = &appInfo;
	icinfo.enabledExtensionCount = vkReqExtensions.size();
	icinfo.ppEnabledExtensionNames = vkReqExtensions.data();
	icinfo.enabledLayerCount = 0;
#ifndef NDEBUG
	//
	// validation layers configurations
	//
	{
		uint32_t availableLayerCount;
		vkEnumerateInstanceLayerProperties(&availableLayerCount, nullptr);
		std::vector<VkLayerProperties> availableLayers(availableLayerCount);
		vkEnumerateInstanceLayerProperties(&availableLayerCount, availableLayers.data());
		for(uint32_t i = 0; i < debugLayerCount; ++i)
		{
			bool layerFound = false;
			for(const auto &availableLayer : availableLayers)
			{
				if(strcmp(availableLayer.layerName, debugLayers[i]) == 0)
				{
					layerFound = true;
					break;
				}
			}
			if(!layerFound)
			{
				throw badEnvironmentRequirement(Rsnprintf<char,1000>("vulkan Debug layer:\"%s\" not found",debugLayers[i]).data);
			}
		}
		// All requested validation layers are found
		icinfo.enabledLayerCount = debugLayerCount;
		icinfo.ppEnabledLayerNames = debugLayers;

		// Add debugging extensions
		// Low performance quick way to do it.
		vkReqExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		icinfo.ppEnabledExtensionNames = vkReqExtensions.data();
		icinfo.enabledExtensionCount += 1;
	}
#endif
	VkResult res = vkCreateInstance(&icinfo, _pAllocator, &_instance);
	if(res != VK_SUCCESS)
	{
		throw mainAppObjInitializationFailure("vulkan instance initialization failed");
	}
#ifndef NDEBUG
	//
	// Initializing messenger object after instance has been initialized.
	//
	if(debugCallback)
	{
		/*bool validationIsEnabled = false;
		if(!std::find_if(validationLayers, validationLayers+validationLayerCount,
				[](const char *str)
				{
					if(strcmp(validationLayers[i], "VK_LAYER_KHRONOS_validation") == 0)
					{
						return true;
					}
				}))
			throw invalidFunctionCall("debugCallback is set but VK_LAYER_KHRONOS_validation is not enabled in the arguments");*/
		auto createdmsgfunc = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(_instance,
			"vkCreateDebugUtilsMessengerEXT");
		if(!createdmsgfunc)
		{
			throw badEnvironmentRequirement("vulkan debug messanger creation not supported");
		}
		auto destroydmsgfunc = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(_instance,
			"vkDestroyDebugUtilsMessengerEXT");
		if(!destroydmsgfunc)
		{
			throw badEnvironmentRequirement("vulkan debug messanger destruction not supported");
		}
		_debugMessengerDestroyer = destroydmsgfunc;

		DebugUtilsMessengerCreateInfoEXT dmsgcinfo;
		dmsgcinfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | 
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		dmsgcinfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		dmsgcinfo.pfnUserCallback = debugCallback;
		dmsgcinfo.pUserData = nullptr; // Change it to some value from some arguments if needed.
		createdmsgfunc(_instance, &dmsgcinfo, _pAllocator, &_debugMessenger);
	}
	//if(apiDumpCallback)
	{
		/*if(!std::find_if(validationLayers, validationLayers+validationLayerCount,
				[](const char *str)
				{
					if(strcmp(validationLayers[i], "VK_LAYER_LUNARG_api_dump") == 0)
					{
						return true;
					}
				}))
			throw invalidFunctionCall("apiDumpCallback is set but VK_LAYER_LUNARG_api_dump is not enabled in the arguments");*/
		//auto createadmsgfunc = (PFN_vkCreateD)
	}
#endif
	
}
inline vulkanAppInstance::~vulkanAppInstance() noexcept
{
	//
	// to do
	//

	vkDestroyDevice(_logicalDevice, _pAllocator);

	// final actions
#ifndef NDEBUG
	if(_debugMessengerDestroyer) // DebugCallback was not nullptr
	{
		_debugMessengerDestroyer(_instance, _debugMessenger, _pAllocator);
	}
#endif
	vkDestroyInstance(_instance, _pAllocator);
}

inline void vulkanAppInstance::generateWarning(rvkAppWarningLevels wl, const char *msg) const noexcept
{
	if(_warningCallback)
		_warningCallback(wl,msg);
}

inline const auto &vulkanAppInstance::getpAllocator() const noexcept { return _pAllocator; }
inline const auto &vulkanAppInstance::getLogicalDevice() const noexcept { return _logicalDevice; }
inline const auto &vulkanAppInstance::getDeviceFeatures() const noexcept { return _deviceFeatures; }
inline const auto &vulkanAppInstance::getDeviceMemoryProps() const noexcept { return _deviceMemoryProps; }
inline void vulkanAppInstance::getFormatProperties(VkFormatProperties &props, VkFormat format) const noexcept
{
	vkGetPhysicalDeviceFormatProperties(_physicalDevice, format, &props);
}
inline VkFormatProperties vulkanAppInstance::getFormatProperties(VkFormat format) const noexcept
{
	VkFormatProperties props;
	getFormatProperties(props, format);
	return props;
}
inline const auto &vulkanAppInstance::getDeviceProperties() const noexcept { return _deviceProperties; }





inline void vulkanAppInstance::createLogicalDevice(const VkPhysicalDevice &physicalDevice, const VkDeviceCreateInfo &dvciArg, 
	const rvkPhysicalDeviceFeatures &features, const VkPhysicalDeviceMemoryProperties &phyDevMemProps)
{
	// fix this(add logging)
	VkPhysicalDeviceProperties devProps{};
	vkGetPhysicalDeviceProperties(physicalDevice, &devProps);	
	printf("creating vulkan logical device on \"%s\"\n", _deviceProperties.deviceName);
#ifndef NDEBUG
	if(_logicalDevice)
		throw mainAppObjInitializationFailure("should not call createLogicalDevice more than once");
#endif
	VkDeviceCreateInfo dvci(dvciArg);

	//VkPhysicalDeviceFeatures2 features2 = features.getFeatures2();
	dvci.pNext = &(features.getFeatures2());
	//dvciArg.pNext = rvkPhysicalDeviceFeatures.
	// some values are ignored in new versions
	// https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#extendingvulkan-layers-devicelayerdeprecation
	
	if(vkCreateDevice(physicalDevice, &dvci, _pAllocator, &_logicalDevice) != VK_SUCCESS)
	{
		throw mainAppObjInitializationFailure("failed to create vulkan logical device");
	}
	_physicalDevice = physicalDevice;
	//vkGetPhysicalDeviceProperties(_physicalDevice, &_deviceProperties);
	_deviceProperties = devProps;
	_deviceFeatures = features;
	_deviceMemoryProps = phyDevMemProps;
	// fix this(add logging)
	printf("created vulkan logical device on \"%s\"\n", _deviceProperties.deviceName);
}


inline const auto &vulkanAppInstance::getInstance() const noexcept { return _instance; }
#ifndef NDEBUG
inline const auto &vulkanAppInstance::getDebugMessenger() const noexcept { return _debugMessenger; }
#endif
inline auto vulkanAppInstance::getPhysicalDevice() const noexcept { return _physicalDevice; }


inline bool checkVkDeviceExtensions(const VkPhysicalDevice &physicalDevice, 
	const std::vector<const char *> &devReqExts) noexcept
{
	std::set<std::string_view> devReqExtsSet(devReqExts.begin(), devReqExts.end());
	//
	// checking device extensions
	//
	uint32_t extCount;
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extCount, nullptr);
	std::vector<VkExtensionProperties> avDevExts(extCount);
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extCount, avDevExts.data());
	for(const auto &avExt : avDevExts)
	{
		devReqExtsSet.erase(avExt.extensionName);
	}
	return devReqExtsSet.empty();
}



inline bool checkVkDeviceProperties( const VkPhysicalDeviceProperties &foundProps, 
	const VkPhysicalDeviceProperties &reqProps) noexcept
{
	//
	// current implementation is incomplete.
	//
	if(reqProps.deviceType/*0 is "other" device type*/)
	{
		if(reqProps.deviceType != foundProps.deviceType)
			return false;
	}
	return true;
}



template<typename appInstanceT>
inline VkSemaphore rvkCreateSemaphore(const std::shared_ptr<appInstanceT> &appInstance)
{
	VkSemaphore retSem;
	constexpr VkSemaphoreCreateInfo sCI { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
	if( vkCreateSemaphore(appInstance->getLogicalDevice(), &sCI, appInstance->getpAllocator(), &retSem) != VK_SUCCESS)
		throw graphicalError("could not create Semaphore");
	
	return retSem;
}



template<typename inputSharedAppInstanceT>
inline rvkCmdPool::rvkCmdPool(const inputSharedAppInstanceT &appInstance, uint32_t qfamilyIndex,
	VkCommandPoolCreateFlags flags)
	: _appInstance(appInstance)
{
	auto hLogicalDevice = _appInstance->getLogicalDevice();
	auto pAllocator = _appInstance->getpAllocator();

	_cmdPoolCI.queueFamilyIndex = qfamilyIndex;
	_cmdPoolCI.flags = flags;

	if(vkCreateCommandPool(hLogicalDevice, &_cmdPoolCI, 
		pAllocator, &_cmdPool) != VK_SUCCESS)
		throw graphicalObjectInitializationFailed(
			"could not create command pool");
}

inline void rvkCmdPool::recreate()
{
	auto hLogicalDevice = _appInstance->getLogicalDevice();
	auto pAllocator = _appInstance->getpAllocator();

	vkDestroyCommandPool(hLogicalDevice, _cmdPool, pAllocator);
	if(vkCreateCommandPool(hLogicalDevice, &_cmdPoolCI, 
		pAllocator, &_cmdPool) != VK_SUCCESS)
		throw graphicalObjectInitializationFailed(
			"could not create command pool");
}



inline rvkCmdPool::~rvkCmdPool()
{
	auto hLogicalDevice = _appInstance->getLogicalDevice();
	auto pAllocator = _appInstance->getpAllocator();

	vkDestroyCommandPool(hLogicalDevice, _cmdPool, pAllocator);
}

inline VkCommandPool rvkCmdPool::getHandle() { return _cmdPool; }

template<typename inputSharedAppInstanceT>
inline rvkCmdBuffers::rvkCmdBuffers(const inputSharedAppInstanceT &appInstance, const cmdPool_t &cmdPool, 
VkCommandBufferLevel level, uint32_t count)
	: _appInstance(appInstance), _cmdPool(cmdPool), _cmdBuffers(count,VK_NULL_HANDLE)
{
	auto hLogicalDevice = _appInstance->getLogicalDevice();

	_allocInfo.commandPool = _cmdPool->getHandle();
	_allocInfo.level = level;
	_allocInfo.commandBufferCount = count;

	if(_allocInfo.commandBufferCount != 0)
	{
		try
		{
			if(vkAllocateCommandBuffers(hLogicalDevice, &_allocInfo, _cmdBuffers.data()) 
			!= VK_SUCCESS)
			throw graphicalObjectInitializationFailed("could not allocate command buffers");
		}
		catch(...)
		{
			vkFreeCommandBuffers(hLogicalDevice,
			_cmdPool->getHandle(), _cmdBuffers.size(), _cmdBuffers.data());
			std::rethrow_exception(std::current_exception());
		}

	}
}

template<bool destroyPrevious>
inline void rvkCmdBuffers::recreate(const cmdPool_t &cmdPool, uint32_t newCount)
{
	_allocInfo.commandBufferCount = newCount;
	recreate<destroyPrevious>(cmdPool);
}


template<bool destroyPrevious>
inline void rvkCmdBuffers::recreate(const cmdPool_t &cmdPool)
{
	auto hLogicalDevice = _appInstance->getLogicalDevice();
	if(destroyPrevious)
	{
		auto hLogicalDevice = _appInstance->getLogicalDevice();
		vkFreeCommandBuffers(hLogicalDevice,
			_cmdPool->getHandle(), _cmdBuffers.size(), _cmdBuffers.data());
	}
	_cmdPool = cmdPool;
	_allocInfo.commandPool = _cmdPool->getHandle();
	_cmdBuffers.resize(_allocInfo.commandBufferCount);
	memset(_cmdBuffers.data(), 0, 
		sizeof(VkCommandBuffer)*_allocInfo.commandBufferCount);
	try
	{
		if(vkAllocateCommandBuffers(hLogicalDevice, &_allocInfo, _cmdBuffers.data()) 
		!= VK_SUCCESS)
		throw graphicalObjectInitializationFailed("could not allocate command buffers");
	}
	catch(...)
	{
		vkFreeCommandBuffers(hLogicalDevice,
		_cmdPool->getHandle(), _cmdBuffers.size(), _cmdBuffers.data());
		std::rethrow_exception(std::current_exception());
	}
}


// begin recording in an index
inline void rvkCmdBuffers::beginI(size_t index, VkCommandBufferUsageFlags flags)
{
	VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	beginInfo.flags = flags;
	beginInfo.pInheritanceInfo = nullptr; // op
	if( vkBeginCommandBuffer( _cmdBuffers[index], &beginInfo) != VK_SUCCESS)
			throw graphicalError( "vkBeginCommandBuffer failed");
}



// end recording in one.
inline void rvkCmdBuffers::endI(size_t index)
{
	if(vkEndCommandBuffer(_cmdBuffers[index]) != VK_SUCCESS)
			throw graphicalError( "vkEndCommandBuffer failed");
}

inline rvkCmdBuffers::~rvkCmdBuffers()
{
	auto hLogicalDevice = _appInstance->getLogicalDevice();
	vkFreeCommandBuffers(hLogicalDevice,
		_cmdPool->getHandle(), _cmdBuffers.size(), _cmdBuffers.data());
}




inline _rvkImage_core::_rvkImage_core(_rvkImage_core &&other)
{
	memcpy(this, &other, sizeof(_rvkImage_core));
	other._moved = true;
}

inline _rvkImage_core::_rvkImage_core(const VkDevice &logicalDevice, VkAllocationCallbacks * const &pAllocator,
const VkPhysicalDeviceMemoryProperties &deviceMemProps,
const size_t &width, const size_t &height, const size_t &planeCount, const VkFormat &format, 
const VkFormatProperties &formatProps, const VkImageTiling &tiling,
const VkSharingMode &sharingMode, const VkImageUsageFlags &usageFlags, const VkMemoryPropertyFlags &memProps)
	: _hLogicalDevice(logicalDevice), _pAllocator(pAllocator), 
	_planeCount(planeCount), _format(format), _width(width), _height(height)
{
	//const auto &deviceMemProps = _appInstance->getDeviceMemoryProps();
	try
	{
		/* If the combination of parameters to vkGetPhysicalDeviceImageFormatProperties2 
		 * is not supported by the implementation for use in vkCreateImage, 
		 * then all members of imageFormatProperties will be filled with zero.
		 * in that case, current implementation of this class, does not 
		 * generate any warnings.
		 */
		//if(tiling == VK_IMAGE_TILING_LINEAR)
		//{
		//	_formatProps2.imageFormatProperties.
		//}


		VkImageCreateInfo imageCI { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
		//imageCI.pNext = &_formatProps2;
		imageCI.imageType = VK_IMAGE_TYPE_2D;
		imageCI.extent.width = width;//_cpl._width;
		imageCI.extent.height = height;//_cpl._height;
		imageCI.extent.depth = 1;
		imageCI.mipLevels = 1;
		imageCI.arrayLayers = 1;
		imageCI.format = _format;
		_format = imageCI.format;
		imageCI.tiling = tiling;
		imageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageCI.usage = usageFlags;
		imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCI.sharingMode = sharingMode;
                //imageCI.flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT; // for multi planar images.

		if(vkCreateImage(_hLogicalDevice, &imageCI, _pAllocator, &_hImage) != VK_SUCCESS)
			throw graphicalError("could not create vkImage");

		VkMemoryRequirements reqPropsImg;
		vkGetImageMemoryRequirements(_hLogicalDevice, _hImage, &reqPropsImg);
		_byteSize = reqPropsImg.size;
		VkMemoryAllocateInfo allocInfo{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
		allocInfo.allocationSize = reqPropsImg.size;
		allocInfo.memoryTypeIndex = rvkFindMemoryProperties(reqPropsImg.memoryTypeBits, 
			memProps, deviceMemProps);

		if( vkAllocateMemory(_hLogicalDevice, &allocInfo, _pAllocator, &_imgMem)
			!= VK_SUCCESS)
			throw graphicalError("could not allocate memory for image");

		if( vkBindImageMemory(_hLogicalDevice, _hImage, _imgMem, 0) != VK_SUCCESS)
			throw outOfGraphicsMemory("could not bind image memory");

		switch(tiling)
		{
			case VK_IMAGE_TILING_OPTIMAL:
				_formatFeatures = formatProps.optimalTilingFeatures;
				break;
			case VK_IMAGE_TILING_LINEAR:
				_formatFeatures = formatProps.linearTilingFeatures;
				break;
			default:
				break;
		}


	}
	catch(...)
	{
		_cleanup();
		std::rethrow_exception(std::current_exception());
	}
}

inline void _rvkImage_core::unmap()
{
	// can add throw with if(_moved) in here
	vkUnmapMemory(_hLogicalDevice, _imgMem);
	mapped = false;
}

inline void  _rvkImage_core::map(VkDeviceSize offset, VkDeviceSize size)
{
	// can add throw with if(_moved) in here
	if(mapped) unmap();
	VkResult ret = vkMapMemory(_hLogicalDevice,
		_imgMem, offset, size, 0, &_pData);
	if( ret != VK_SUCCESS)
	{
		if( ret == VK_ERROR_MEMORY_MAP_FAILED)
		{
			throw graphicalError("vertex buffer memory mapping failed");
		}
		throw outOfGraphicsMemory("vertex buffer memory mapping failed");
	}
	mapped = true;
}

inline void _rvkImage_core::map()
{
	map(0, _byteSize);
}

// adds barriers for all image planes, uses a sample for most parameters.
inline void _rvkImage_core::addBarriers_allPlanes(std::vector<VkImageMemoryBarrier> &barriers, const VkImageMemoryBarrier &sample,
	uint32_t baseMipLevel, uint32_t levelCount,
	uint32_t baseArrayLayer, uint32_t layerCount) const noexcept
{
	// can add throw with if(_moved) in here
	VkImageSubresourceRange isr{};
	isr.baseMipLevel = baseMipLevel;
	isr.levelCount = levelCount;
	isr.baseArrayLayer = baseArrayLayer;
	isr.layerCount = layerCount;
	size_t oldSize = barriers.size();
	barriers.resize(oldSize +_planeCount, sample);
	for(size_t i = 0; i < _planeCount; ++i)
	{
		size_t cur = oldSize+i;
		barriers[cur].image = _hImage;
		isr.aspectMask = rvkPlaneFlags[i];
		//
		// experimental
		//
		if(_planeCount == 1)
		{
			isr.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}	
		barriers[cur].subresourceRange = isr;
	}

}

inline void _rvkImage_core::addBarriers_depthAndStencilAspect(std::vector<VkImageMemoryBarrier> &barriers, const VkImageMemoryBarrier &sample,
	uint32_t baseMipLevel, uint32_t levelCount,
	uint32_t baseArrayLayer, uint32_t layerCount) const
{
	//
	// experimental
	//
	if(_planeCount != 1) throw graphicalError("invalid plane Count for accessing depth aspect of image");
	// can add throw with if(_moved) in here
	VkImageSubresourceRange isr{};
	isr.baseMipLevel = baseMipLevel;
	isr.levelCount = levelCount;
	isr.baseArrayLayer = baseArrayLayer;
	isr.layerCount = layerCount;
	size_t oldSize = barriers.size();
	barriers.resize(oldSize + 1, sample);
	barriers[oldSize].image = _hImage;
	isr.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	barriers[oldSize].subresourceRange = isr;
}

inline void _rvkImage_core::addBarriers_depthAspect(std::vector<VkImageMemoryBarrier> &barriers, const VkImageMemoryBarrier &sample,
	const uint32_t &baseMipLevel, const uint32_t &levelCount,
	const uint32_t &baseArrayLayer, const uint32_t &layerCount) const
{
	//
	// experimental
	//
	if(_planeCount != 1) throw graphicalError("invalid plane Count for accessing depth aspect of image");
	// can add throw with if(_moved) in here
	VkImageSubresourceRange isr{};
	isr.baseMipLevel = baseMipLevel;
	isr.levelCount = levelCount;
	isr.baseArrayLayer = baseArrayLayer;
	isr.layerCount = layerCount;
	size_t oldSize = barriers.size();
	barriers.resize(oldSize + 1, sample);
	barriers[oldSize].image = _hImage;
	isr.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	barriers[oldSize].subresourceRange = isr;
}

inline void _rvkImage_core::addBarriers_stencilAspect(std::vector<VkImageMemoryBarrier> &barriers, const VkImageMemoryBarrier &sample,
	const uint32_t &baseMipLevel, const uint32_t &levelCount,
	const uint32_t &baseArrayLayer, const uint32_t &layerCount) const
{
	//
	// experimental
	//
	if(_planeCount != 1) throw graphicalError("invalid plane Count for accessing depth aspect of image");
	// can add throw with if(_moved) in here
	VkImageSubresourceRange isr{};
	isr.baseMipLevel = baseMipLevel;
	isr.levelCount = levelCount;
	isr.baseArrayLayer = baseArrayLayer;
	isr.layerCount = layerCount;
	size_t oldSize = barriers.size();
	barriers.resize(oldSize + 1, sample);
	barriers[oldSize].image = _hImage;
	isr.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
	barriers[oldSize].subresourceRange = isr;
}


inline void _rvkImage_core::cmdCopyDepthToBuffer(const VkCommandBuffer &cmdBuff, const VkImageLayout &layout, const VkBuffer &dstBuff) const 
{
	std::array<VkBufferImageCopy,1> regions{};
	regions[0].bufferImageHeight = _height;
	regions[0].bufferRowLength = _width;
	regions[0].bufferOffset = 0;
	regions[0].imageExtent.depth = 1;
	regions[0].imageExtent.width = _width;
	regions[0].imageExtent.height = _height;
	regions[0].imageSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	regions[0].imageSubresource.layerCount = 1;

	vkCmdCopyImageToBuffer(cmdBuff, _hImage, layout, dstBuff, 1, regions.data());
}

inline void _rvkImage_core::cmdCopyDepthToBuffer(const VkCommandBuffer &cmdBuff, const VkImageLayout &layout, const VkBuffer &dstBuff,
const int32_t &x, const int32_t &y) const 
{
	std::array<VkBufferImageCopy,1> regions{};
	regions[0].bufferImageHeight = _height;
	regions[0].bufferRowLength = _width;
	regions[0].bufferOffset = 0;
	regions[0].imageExtent.depth = 1;
	regions[0].imageExtent.width = 1;
	regions[0].imageExtent.height = 1;
	regions[0].imageOffset.x = (x >= 0 && x <= _width - 1 ) ? x : 0;
	regions[0].imageOffset.y = (y >= 0 && y <= _height - 1 ) ? y : 0;
	regions[0].imageSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	regions[0].imageSubresource.layerCount = 1;

	vkCmdCopyImageToBuffer(cmdBuff, _hImage, layout, dstBuff, 1, regions.data());
}

inline auto const &_rvkImage_core::getFormatFeatures() const noexcept { return _formatFeatures; }

inline auto const &_rvkImage_core::getFormat() const noexcept { return _format; }
inline auto _rvkImage_core::getWidth() const noexcept { return _width; }
inline auto _rvkImage_core::getHeight() const noexcept { return _height; }

inline void _rvkImage_core::cleanup() noexcept
{
	if(!_moved)
	{
		_cleanup();
	}
}

inline void _rvkImage_core::_cleanup() noexcept
{	
	vkDestroyImage(_hLogicalDevice, _hImage, _pAllocator);
	vkFreeMemory(_hLogicalDevice, _imgMem, _pAllocator);
}


template<bool doCleanup>
inline rvkImage_basic<doCleanup>::rvkImage_basic(rvkImage_basic<doCleanup> &&other)
	: _rvkImage_core(std::move(other))
{

}
template<bool doCleanup>
inline rvkImage_basic<doCleanup>::rvkImage_basic(VkDevice logicalDevice, VkAllocationCallbacks *pAllocator,
const VkPhysicalDeviceMemoryProperties &deviceMemProps,
size_t width, size_t height, size_t planeCount, VkFormat format, 
VkFormatProperties formatProps, VkImageTiling tiling,
VkSharingMode sharingMode, VkImageUsageFlags usageFlags, VkMemoryPropertyFlags memProps)
	: _rvkImage_core(logicalDevice, pAllocator, deviceMemProps, width, height, planeCount,
	format, formatProps, tiling, sharingMode, usageFlags, memProps)
{

}
template<bool doCleanup>
inline rvkImage_basic<doCleanup>::~rvkImage_basic()
{
	if(doCleanup)cleanup();
}

inline _rvkBuffer_core::_rvkBuffer_core( _rvkBuffer_core &&other )
{
	memcpy(this, &other, sizeof(_rvkBuffer_core));
	other._hBuffer = VK_NULL_HANDLE;
}


inline _rvkBuffer_core::_rvkBuffer_core( VkDevice hLogicalDevice, VkAllocationCallbacks *pAllocator, VkDeviceSize byteSize, 
const VkPhysicalDeviceMemoryProperties &deviceMemProps,
VkSharingMode sharingMode, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags reqPropFlags)
	: _hLogicalDevice(hLogicalDevice), _pAllocator(pAllocator), _byteSize(byteSize), _mapped(false)
{
	try
	{
		VkBufferCreateInfo bufferCI{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		bufferCI.size = byteSize;
		bufferCI.usage = usageFlags;
		bufferCI.sharingMode = sharingMode;

		if(vkCreateBuffer(_hLogicalDevice, &bufferCI, _pAllocator, &_hBuffer)
			!= VK_SUCCESS)
			throw graphicalError("could not create vertex buffer");

		VkMemoryRequirements reqPropsVer;
		vkGetBufferMemoryRequirements(_hLogicalDevice, _hBuffer, &reqPropsVer);
		VkMemoryAllocateInfo allocInfo{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
		allocInfo.allocationSize = reqPropsVer.size;
		allocInfo.memoryTypeIndex = rvkFindMemoryProperties(reqPropsVer.memoryTypeBits, 
			reqPropFlags, deviceMemProps);

		if( vkAllocateMemory(_hLogicalDevice, &allocInfo, _pAllocator, &_mem)
			!= VK_SUCCESS)
			throw graphicalError("could not allocate memory for buffer object of");

		if(vkBindBufferMemory(_hLogicalDevice, _hBuffer, _mem, 0)
			!= VK_SUCCESS)
		{
			throw outOfGraphicsMemory("could not bind allocated memory for buffer object");
		}
	}
	catch(...)
	{
		_cleanup();
		std::rethrow_exception(std::current_exception());
	}
}

inline void _rvkBuffer_core::unmap()
{
	vkUnmapMemory(_hLogicalDevice, _mem);
	_mapped = false;
}

inline void  _rvkBuffer_core::map(VkDeviceSize offset, VkDeviceSize size)
{
	if(_mapped) unmap();
	VkResult ret = vkMapMemory(_hLogicalDevice,
		_mem, offset, size, 0, &_pData);
	if( ret != VK_SUCCESS)
	{
		if( ret == VK_ERROR_MEMORY_MAP_FAILED)
		{
			throw graphicalError("buffer memory mapping failed");
		}
		throw outOfGraphicsMemory("buffer memory mapping failed");
	}
	_mapped = true;
}

inline void _rvkBuffer_core::map()
{
	map(0, _byteSize);
}

// frequent calls not recommended, try to use a single vkFlushMappedMemoryRanges instead
inline void _rvkBuffer_core::flush(VkDeviceSize offset, VkDeviceSize size)
{
	VkMappedMemoryRange range{ VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE };
	range.memory = _mem;
	range.size = size;
	range.offset = offset;
	if(vkFlushMappedMemoryRanges(_hLogicalDevice, 1, &range) != VK_SUCCESS)
		throw outOfGraphicsMemory(" could not flush buffer memory");
}

inline void _rvkBuffer_core::flushAll()
{
	flush(0,_byteSize);
}

inline void _rvkBuffer_core::cmdBind(const VkCommandBuffer &cmdbuff, 
uint32_t bindingIndex /*must be less than VkPhysicalDeviceLimits::maxVertexInputBindings*/) const noexcept
{
	VkDeviceSize offset = 0;
	vkCmdBindVertexBuffers(cmdbuff, bindingIndex, 1, &_hBuffer, &offset);
}


inline void _rvkBuffer_core::cleanup() noexcept
{
	if(_hBuffer != VK_NULL_HANDLE) // object is not moved
		_cleanup();
}

inline void _rvkBuffer_core::_cleanup() noexcept
{	
	vkDestroyBuffer(_hLogicalDevice, _hBuffer, _pAllocator);
	vkFreeMemory(_hLogicalDevice, _mem, _pAllocator);
}


template<bool doCleanup>
inline rvkBuffer_basic<doCleanup>::rvkBuffer_basic( rvkBuffer_basic<doCleanup> &&other)
	: _rvkBuffer_core(std::move(other))
{

}

template<bool doCleanup>
inline rvkBuffer_basic<doCleanup>::rvkBuffer_basic( VkDevice hLogicalDevice, VkAllocationCallbacks *pAllocator, VkDeviceSize byteSize, 
const VkPhysicalDeviceMemoryProperties &deviceMemProps,
VkSharingMode sharingMode, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags reqPropFlags)
	: _rvkBuffer_core(hLogicalDevice, pAllocator, byteSize, deviceMemProps,
	sharingMode, usageFlags, reqPropFlags)
{

}
template<bool doCleanup>
inline rvkBuffer_basic<doCleanup>::~rvkBuffer_basic()
{
	if(doCleanup)
		cleanup();
}

inline rvkBuffer::rvkBuffer( rvkBuffer &&other)
	: rvkBuffer_basic<false>(std::move(other)), _appInstance(std::move(other._appInstance))
{

}

template<typename appInstanceT>
inline rvkBuffer::rvkBuffer( const std::shared_ptr<appInstanceT> &appInstance, VkDeviceSize byteSize, 
VkSharingMode sharingMode, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags reqPropFlags)
	: rvkBuffer_basic<false>(appInstance->getLogicalDevice(), appInstance->getpAllocator(),
	byteSize, appInstance->getDeviceMemoryProps(),
	sharingMode, usageFlags, reqPropFlags), _appInstance(appInstance)
{

}


inline rvkBuffer::~rvkBuffer()
{
	cleanup();
}

inline rvkImage::rvkImage(rvkImage &&other)
	: rvkImage_basic<false>(std::move(other)),_appInstance(other._appInstance)
{
}

inline rvkImage::rvkImage(const std::shared_ptr<vulkanAppInstance> &appInstance,
const rvkColorPlaneLayout_t &cpl, const rvkFormatInterface &rvkFI, VkImageTiling tiling,
VkSharingMode sharingMode, VkImageUsageFlags usageFlags, VkMemoryPropertyFlags memProps)
	: rvkImage(appInstance, cpl._width, cpl._height, cpl._planeCount, rvkFI._vkformat,
	tiling, sharingMode, usageFlags, memProps)
{

}

inline rvkImage::rvkImage(const std::shared_ptr<vulkanAppInstance> &appInstance,
size_t width, size_t height, size_t planeCount, VkFormat format, VkImageTiling tiling,
VkSharingMode sharingMode, VkImageUsageFlags usageFlags, VkMemoryPropertyFlags memProps)
	: rvkImage_basic<false>(appInstance->getLogicalDevice(), appInstance->getpAllocator(),
	appInstance->getDeviceMemoryProps(), width, height, planeCount, format, 
	appInstance->getFormatProperties(format), tiling, sharingMode, usageFlags, memProps),
	_appInstance(appInstance)
{

}

inline rvkImage::~rvkImage()
{
	cleanup();
}

inline auto const &rvkImage::getAppInstance() const noexcept { return _appInstance; }

	
	
inline rvkFence_core::rvkFence_core(rvkFence_core &&other)
	: _hLogicalDevice(other._hLogicalDevice), _pAllocator(other._pAllocator),
	_handle(other._handle), _isSubmitted(other._isSubmitted)
{
	std::unique_lock<std::mutex> iwlk(other._isWaitingM);
	std::unique_lock<std::mutex> islk(other._isSubmittedM);
	if(_isWaiting && _isSubmitted)
	{
		other._waitFor(iwlk,islk,std::chrono::seconds(1)); // if throws, "other" remains unchanged.
	}
	other._handle = VK_NULL_HANDLE;
}

inline rvkFence_core::rvkFence_core(VkDevice hLogicalDevice, VkAllocationCallbacks *pAllocator, bool signaled)
	: _hLogicalDevice(hLogicalDevice), _pAllocator(pAllocator)
{
	VkFenceCreateInfo ci{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
	if(signaled) ci.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	if(vkCreateFence(_hLogicalDevice, &ci, _pAllocator, &_handle)!= VK_SUCCESS)
		throw graphicalError("could not create Fence");
}

/*the actual timeout duration may be twice as long.
 */
template<typename RepT, typename PeriodT>
inline void rvkFence_core::waitFor(const std::chrono::duration<RepT,PeriodT> &timeoutDuration) const
{
	std::unique_lock<std::mutex> iwlk(_isWaitingM);
	std::unique_lock<std::mutex> islk(_isSubmittedM);
	_waitFor(iwlk, islk, timeoutDuration);
}



/* unlocks the input lock after a success for lock of the internal lock. 
 *	ensures lock is not released until fence into wait mode.
 * returns the _submitIndex
 * _submitIndex should be incremental(not necessarily increased by one) or 
 *	some functions may not work properly.
 * 
 * the actual timeout duration may be twice as long.
 */
template<typename lockT, typename RepT, typename PeriodT>
inline auto rvkFence_core::releaseAndWaitFor(lockT &ilk, const std::chrono::duration<RepT,PeriodT> &timeoutDuration) const
{
	std::unique_lock<std::mutex> iwlk(_isWaitingM);
	std::unique_lock<std::mutex> islk(_isSubmittedM);
	ilk.unlock();
	return _waitFor(iwlk,islk, timeoutDuration);
}

inline std::tuple<std::unique_lock<std::mutex>,std::unique_lock<std::mutex>> rvkFence_core::reset()
{
	std::unique_lock<std::mutex> iwlk(_isWaitingM);
	std::unique_lock<std::mutex> islk(_isSubmittedM);
	_reset(iwlk,islk);
	return{std::move(iwlk),std::move(islk)};
}

/*
 * _submitIndex should be incremental(not necessarily increased by one) or 
 *	some functions may not work properly.
 */
inline void rvkFence_core::setIsSubmitted(uint64_t submitIndex)
{
	std::lock_guard<std::mutex> islk(_isSubmittedM);
	_isSubmitted = true;
	_submitIndex = submitIndex;
	_isSubmittedCV.notify_all();
}

/* high performance version to check if a renderIndex for the retrieved fence
 * has finished it's execution.
 * when using circular resource buffers, it can be used to know if the 
 * oldest resource can be assigned as free to use or not. although it should
 * be considered that if the resource is used in a single render only, or in 
 * all renders till the preparation of the new resource. 
 */
inline bool rvkFence_core::isFinished(uint64_t submitIndex) const
{
	std::lock_guard<std::mutex> islk(_isSubmittedM);
#ifndef NDEBUG
	if(_submitIndex.has_value() && submitIndex > _submitIndex.value())
		throw invalidFunctionCall("invalid submitIndex requested");
#endif
	if(_submitIndex.has_value() && submitIndex < _submitIndex.value())
	{
		return true;
	}
	//else
	if(!_isSubmitted)
	{
		return true;
	}
	return false;
}

/* the actual timeout duration may be twice as long.
 */
template<typename RepT, typename PeriodT>
inline void rvkFence_core::waitForFinish(uint64_t submitIndex, const std::chrono::duration<RepT, PeriodT> &timeoutDuration) const 
{
	std::unique_lock<std::mutex> iwlk(_isWaitingM);
	std::unique_lock<std::mutex> islk(_isSubmittedM);
#ifndef NDEBUG
	if(_submitIndex.has_value() && submitIndex > _submitIndex.value())
		throw invalidFunctionCall("invalid submitIndex requested");
#endif
	if(_submitIndex.has_value() && submitIndex < _submitIndex.value())
	{
		return;
	}
	else if(!_isSubmitted)
	{
		return;
	}
	// else
	_waitFor(iwlk, islk, timeoutDuration);
}

inline VkFence rvkFence_core::getHandle() const noexcept { 
	return _handle; 
}

inline auto rvkFence_core::getLastSubmitIndex() const noexcept
{
	std::lock_guard<std::mutex> islk(_isSubmittedM);
	return _submitIndex; // remember, it has a optional type
}

inline void rvkFence_core::_cleanup()
{
	std::lock_guard<std::mutex> islk(_isSubmittedM);
	if(_isSubmitted)
		throw graphicalError("attempted to destroy a fence that is submitted. this is probably a cleanup bug");
	vkDestroyFence(_hLogicalDevice, _handle, _pAllocator);
}

// may unlock locks. returns the _submitIndex
template<typename RepT, typename durationT>
inline uint64_t rvkFence_core::_waitFor(std::unique_lock<std::mutex> &iwlk, std::unique_lock<std::mutex> &islk,
	const std::chrono::duration<RepT, durationT> &timeoutDuration) const
{
	//std::unique_lock<std::mutex> lk(_isWaitingM);
	if(!_handle)
		throw graphicalError("trying to wait for invalid fence handle");
	// locking order allows this.
	if(_isWaiting)
	{
#ifndef NDEBUG
		if(!_submitIndex.has_value())
			throw graphicalError("logical problem with _waitFor, _submitIndex has no value");
#endif
		auto si = _submitIndex.value();
		islk.unlock();
		if(_isWaitingCV.wait_for(iwlk, timeoutDuration)
			== std::cv_status::timeout)
			throw rTimeoutException("fence _waitFor timeout");
		return si;
		//islk.lock();
	}
	else
	{
		// note. locking order allows this.
		_isWaiting = true;
		iwlk.unlock();
		if(!_isSubmitted)
		{
			if(!_isSubmittedCV.wait_for(islk, timeoutDuration, [this](){ return this->_isSubmitted; }))
			{
				islk.unlock();
				iwlk.lock();
				_isWaiting = false;
				throw rTimeoutException("fence _waitFor timeout"
					" while waiting for submit");
			}
		}
#ifndef NDEBUG
		if(!_submitIndex.has_value())
			throw graphicalError("logical problem with _waitFor, _submitIndex has no value");
#endif
		auto si = _submitIndex.value();
		auto vkTimeout = std::chrono::duration_cast<std::chrono::nanoseconds>(timeoutDuration).count();
		if(vkTimeout < 0)
			throw RbasicException("invalid timeout variable");
		VkResult res = vkWaitForFences(_hLogicalDevice, 1, &_handle, VK_TRUE, 
			(uint64_t) vkTimeout);
		islk.unlock();
		iwlk.lock();
		_isWaiting = false;
		iwlk.unlock();
		if(res == VK_SUCCESS) 
		{
			_isWaitingCV.notify_all();
			return si;
		}
		else if(res == VK_TIMEOUT)
		{
			throw rTimeoutException("fence _waitFor timeout while "
				"waiting for Fence with vkWaitForFences");
		}
		else
			throw graphicalError("vkWaitForFence failed");
	}
}

// keeps the locks locked.
inline void rvkFence_core::_reset(std::unique_lock<std::mutex> &iwlk, std::unique_lock<std::mutex> &islk )
{
	if(!_handle)
		throw graphicalError("trying to reset invalid fence handle");
	if(_isWaiting)
	{
		if(!_isSubmitted)
		{
			mainLogger.logText("trying to rest an unsubmitted fence, a response to this behaviour is not implemented");
			exit(-1);
		}
		islk.unlock();
		_isWaitingCV.wait(iwlk, [this](){return !this->_isWaiting; });
		islk.lock();
		vkWaitForFences(_hLogicalDevice, 1, &_handle, VK_TRUE, UINT64_MAX);
		vkResetFences(_hLogicalDevice, 1, &_handle);
		_isSubmitted = false;
	}
	else if(_isSubmitted)
	{
		VkResult res = vkWaitForFences(_hLogicalDevice, 1, &_handle, VK_TRUE, UINT64_MAX);
		if(res == VK_SUCCESS) 
		{
			vkResetFences(_hLogicalDevice, 1, &_handle);
			_isSubmitted = false;
			return;
		}
		else
			throw graphicalError("vkWaitForFence failed");
	}
	else
	{
		vkResetFences(_hLogicalDevice, 1, &_handle);
	}
	//_isSubmittedCV.notify_all(); not required.
}


template<bool doCleanup>
rvkFence_basic<doCleanup>::rvkFence_basic(rvkFence_basic &&other)
	: rvkFence_core(std::move(other))
{
}
template<bool doCleanup>
rvkFence_basic<doCleanup>::rvkFence_basic(VkDevice hLogicalDevice, VkAllocationCallbacks *pAllocator, bool signaled)
	: rvkFence_core(hLogicalDevice, pAllocator, signaled)
{

}
template<bool doCleanup>
rvkFence_basic<doCleanup>::~rvkFence_basic()
{
	if(doCleanup == true)
		_cleanup();
}

inline rvkFence::rvkFence(rvkFence &&other)
	: rvkFence_basic<false>(std::move(other))
{

}
template<typename appInstanceT>
inline rvkFence::rvkFence(const std::shared_ptr<appInstanceT> &appInstance, bool signaled)
	: rvkFence_basic<false>(appInstance->getLogicalDevice(), appInstance->getpAllocator(), signaled),
	_appInstance(appInstance)
{

}

inline rvkFence::~rvkFence() noexcept
{
	_cleanup();
}
}}
