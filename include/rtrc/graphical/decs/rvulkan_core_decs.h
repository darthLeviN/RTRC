
#pragma once
#include <vulkan/vulkan.hpp>
#include <condition_variable>
#include <tuple>
#include "../rimage_core.h"
#include <functional>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <set>
#include <chrono>


namespace rtrc { namespace vkl {

struct rvkColorPlaneLayout_t
{
	// the indexes, are vulkan plane indexes.
	//size_t _bufferOffsets[3];
	uint8_t *_rvkPlaneData[3];
	size_t _planeCount;
	size_t _linesizes[3], _planeHeights[3], _planeWidths[3];
	size_t _height, _width;
	
	void copyTo(void *dst);
};
	

typedef rvkColorPlaneLayout_t (*getVkPlaneProc_t)(const rimagel::rImage &);

struct rvkFormatInterface
{
	VkFormat _vkformat;
	rimagel::rImagePixFmt _rImgPixFormat;
	getVkPlaneProc_t _getPlaneProc;
};


struct rvkBufferImageCopyInfo
{
	size_t _rgCount;
	VkBufferImageCopy _regions[3];
};


inline uint32_t rvkFindMemoryProperties(uint32_t memoryTypeBits, VkMemoryPropertyFlags reqPropFlags,
	 const VkPhysicalDeviceMemoryProperties &deviceMemProps);

struct VkPhysicalDeviceReqProperties {
    uint32_t                            apiVersion;
    uint32_t                            driverVersion;
    VkPhysicalDeviceType                deviceType;
    //char                                deviceName[VK_MAX_PHYSICAL_DEVICE_NAME_SIZE];
    //uint8_t                             pipelineCacheUUID[VK_UUID_SIZE];
    VkPhysicalDeviceLimits              limits;
    VkPhysicalDeviceSparseProperties    sparseProperties;
};

struct ApplicationInfo : VkApplicationInfo
{
	ApplicationInfo() noexcept;
};

struct DebugUtilsMessengerCreateInfoEXT : VkDebugUtilsMessengerCreateInfoEXT
{
	DebugUtilsMessengerCreateInfoEXT() noexcept;
};

struct DeviceQueueCreateInfo : VkDeviceQueueCreateInfo
{
	DeviceQueueCreateInfo() noexcept;
};

struct SwapchainCreateInfoKHR : VkSwapchainCreateInfoKHR
{
	SwapchainCreateInfoKHR() noexcept;
};

/* collects swapchain details of the VkSurfaceKHR and searches inside them.
 * can be refreshed with functions like reInitialize and resetCapabilities.
 * this struct is useful for creating a swapchain from a khrSurface.
 */ 
struct swapChainSupportDetails {
	// bug : has the permission to change khrSurface
	swapChainSupportDetails() = default; // uninitialized.
	
	swapChainSupportDetails(VkPhysicalDevice physicalDevice, VkSurfaceKHR khrSurface) noexcept;
	
	void reInitialize(VkPhysicalDevice physicalDevice, VkSurfaceKHR khrSurface) noexcept;
	
	// efficiency in these functions is unimportant
	bool hasFormat(const VkSurfaceFormatKHR &reqFormat) const noexcept;
	
	bool hasPresentationMode(VkPresentModeKHR reqpm) const noexcept;
	
	// idk if this is needed or not.
	void resetCapabilities(VkPhysicalDevice physicalDevice, VkSurfaceKHR khrSurface) noexcept;
	
	// changes reqExtent if needed
	void getSwapExtent(VkExtent2D &reqExtent);
	
	
	const auto &getCapabilities() const noexcept;
	const auto &getFormats() const noexcept;
	const auto &getPresentModes() const noexcept;
	
private:
    VkSurfaceCapabilitiesKHR _capabilities;
    std::vector<VkSurfaceFormatKHR> _formats;
    std::vector<VkPresentModeKHR> _presentModes;
};

// just a simple holder for swapchain properties. mostly will be passed to callers.
struct swapchainProperties
{
	VkSurfaceFormatKHR _surfaceFormat;
	VkPresentModeKHR _presentMode;
	VkExtent2D _extent;
	size_t _imageLayerCount = 1;
	size_t _imageCount = 0; // set by swapchain creator
	VkFormat _depthAndStencilImageFormat = VK_FORMAT_UNDEFINED;
	VkImageTiling _depthAndStencilImageTiling = VK_IMAGE_TILING_OPTIMAL;
};



struct pipeVertexInputStateCI
{
	pipeVertexInputStateCI(const pipeVertexInputStateCI &) = delete;
	pipeVertexInputStateCI &operator=(const pipeVertexInputStateCI &) = delete;
	
	pipeVertexInputStateCI(pipeVertexInputStateCI &&moved);
	
	
	pipeVertexInputStateCI( std::vector<VkVertexInputBindingDescription> &&inputBindingDescs,
	 std::vector<VkVertexInputAttributeDescription> &&inputAttrDescs);
	
		
	const auto &getRef() const noexcept;
	
	VkPipelineVertexInputStateCreateInfo _ci{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
	std::vector<VkVertexInputBindingDescription> _inputBindingDescs;
	std::vector<VkVertexInputAttributeDescription> _inputAttrDescs;
	
};

struct pipeViewportStateCI
{
	pipeViewportStateCI(const pipeViewportStateCI &pci);
	pipeViewportStateCI(pipeViewportStateCI  &&pci);
	
	pipeViewportStateCI &operator=(const pipeViewportStateCI &) = delete;
	
	pipeViewportStateCI(std::vector<VkViewport> &&viewports, std::vector<VkRect2D> &&scissors );
		
	const auto &getRef() const noexcept;
	

	std::vector<VkViewport> _viewports;
	std::vector<VkRect2D> _scissors;
	VkPipelineViewportStateCreateInfo _ci;
	
};

struct colorBlendAttachmentState : VkPipelineColorBlendAttachmentState
{
	colorBlendAttachmentState();
};

struct colorBlendStateCI
{
	colorBlendStateCI(const colorBlendStateCI &ci) = delete;
	
	colorBlendStateCI(colorBlendStateCI &&ci);
	
	colorBlendStateCI &operator=(const colorBlendStateCI &) = delete;
	
	colorBlendStateCI(std::vector<colorBlendAttachmentState> &&attStates);
	
	const auto &getRef() const noexcept;
	
	std::vector<colorBlendAttachmentState> _attStates;
	VkPipelineColorBlendStateCreateInfo _ci;
	
};

struct descSetLayoutCI
{
	descSetLayoutCI(const descSetLayoutCI &) = delete;
	descSetLayoutCI &operator=(const descSetLayoutCI &) = delete;
	
	descSetLayoutCI(descSetLayoutCI &&moved);
	
	descSetLayoutCI(std::vector<VkDescriptorSetLayoutBinding> &&bindings);
	
	const auto &getRef() const noexcept;
	
	std::vector<VkDescriptorSetLayoutBinding> _bindings;
	VkDescriptorSetLayoutCreateInfo _ci;
};

/* this is not very understandable struct due to nested vectors. may change in future.
 * defines the description of array of array of VkAttachmentReference. meaning
 * an array of groups of fragment's output indexes.
 */

struct rvkAttachmentReferences
{
	std::vector<VkAttachmentReference> colorAttRefs;
	std::optional<VkAttachmentReference> depthAndStencilAttachment;
};

struct rvkSubpassDescs : VkSubpassDescription
{
	rvkSubpassDescs(const rvkSubpassDescs &) = delete;
	rvkSubpassDescs &operator=(const rvkSubpassDescs &) = delete;
	
	rvkSubpassDescs(rvkSubpassDescs &&subpass);
	
	rvkSubpassDescs(std::vector<rvkAttachmentReferences> &&attRefsArr);
	
	const VkSubpassDescription *getPtr() const noexcept;
	auto getSize() const noexcept;
	
	std::vector<rvkAttachmentReferences> _attRefsArr;
	std::vector<VkSubpassDescription> _subpassDescs;
};

struct rvkRenderPassCI
{
	rvkRenderPassCI(const rvkRenderPassCI &) = delete;
	rvkRenderPassCI &operator=(const rvkRenderPassCI &) = delete;
	rvkRenderPassCI(rvkRenderPassCI &&ci);
	
	rvkRenderPassCI(std::vector<VkAttachmentDescription> &&colorAttDescs, std::vector<VkSubpassDependency> &&dependencies,
	rvkSubpassDescs &&subpassDescs);
	
	const VkRenderPassCreateInfo &getRef() const noexcept;
	
	std::vector<VkAttachmentDescription> _colorAttDescs;
	std::vector<VkSubpassDependency> _dependencies;
	rvkSubpassDescs _subpassDescs;
	VkRenderPassCreateInfo _ci;
};

struct rvkRenderPassBeginInfo
{
	rvkRenderPassBeginInfo(const rvkRenderPassBeginInfo &) = delete;
	rvkRenderPassBeginInfo &operator=(const rvkRenderPassBeginInfo &) = delete;
	
	rvkRenderPassBeginInfo(rvkRenderPassBeginInfo &&moved);
	
	rvkRenderPassBeginInfo(std::vector<VkClearValue> &&clearColors);
	
	const VkRenderPassBeginInfo &getRef();
	
	VkRenderPassBeginInfo _beginInfo;
	std::vector<VkClearValue> _clearColors;
};

struct rvkWriteDescriptorSet
{
	
	rvkWriteDescriptorSet &operator=(const rvkWriteDescriptorSet &) = delete;
	
	rvkWriteDescriptorSet( const rvkWriteDescriptorSet &copied );
	
	rvkWriteDescriptorSet( rvkWriteDescriptorSet &&moved);
	rvkWriteDescriptorSet(std::vector<VkDescriptorImageInfo> &&imgInfos,
	std::vector<VkDescriptorBufferInfo> &&bufferInfos,
	std::vector<VkBufferView> &&texelViews);
	
	rvkWriteDescriptorSet(std::vector<VkDescriptorImageInfo> &&imgInfos);
		
	rvkWriteDescriptorSet(std::vector<VkDescriptorBufferInfo> &&bufferInfos);
		
	rvkWriteDescriptorSet(std::vector<VkBufferView> &&texelViews);
		
	VkWriteDescriptorSet &getRef();
	
	std::vector<VkDescriptorImageInfo> _imgInfos;
	std::vector<VkDescriptorBufferInfo> _bufferInfos;
	std::vector<VkBufferView> _texelViews;
	VkWriteDescriptorSet _winfo;
};

/* a vector of VkWriteDescriptorSet passed as a option and mapped to the desired descriptorSets
 * the VkWriteDescriptorSet::dstSet should be equal to the index to the desired descriptorSet array/vector
 * use _imgInfos, _bufferInfos and _texelViews to send pointer related data to rvkDescSameUpdater
 */
struct rvkDescSameUpdater
{
	rvkDescSameUpdater( const rvkDescSameUpdater & ) = delete;
	rvkDescSameUpdater &operator=(const rvkDescSameUpdater &) = delete;
	
	rvkDescSameUpdater(rvkDescSameUpdater &&moved);
	
	rvkDescSameUpdater(std::vector<rvkWriteDescriptorSet> &&writes);
	
	// changes custom values in descSets to real values with the help of a perPassBuffer
	template<typename perPassBuffer_t>
	void mapTo(const std::vector<VkDescriptorSet> &descSets, std::vector<rvkWriteDescriptorSet> &descWrites, const perPassBuffer_t &perIBuff) const;
	
	// updates all of the descriptor set arrays with the same options.
	// all internal arrays should have the same size. the first std::vector<VkDEscriptorSet> in descSetsArr will be used as a size reference.
	template<typename perPassBuffer_t>
	void update(VkDevice device, 
	const std::vector<std::vector<VkDescriptorSet>> &descSetsArr, 
	const std::vector<std::shared_ptr<perPassBuffer_t>> &perIBuffs ) const;

	std::vector<rvkWriteDescriptorSet> _opts;
};
	
enum rvkAppWarningLevels
{
	rvkWarnings_overriddenSettings
};

inline bool checkVkDeviceFeatures( const VkPhysicalDeviceFeatures &avFs, 
	const VkPhysicalDeviceFeatures &reqFs) noexcept;

struct rvkPhysicalDeviceFeatures
{
	rvkPhysicalDeviceFeatures(const rvkPhysicalDeviceFeatures &copy);
	
	rvkPhysicalDeviceFeatures( rvkPhysicalDeviceFeatures &&) = delete;
	
	rvkPhysicalDeviceFeatures &operator=(const rvkPhysicalDeviceFeatures &other);
	
	rvkPhysicalDeviceFeatures();
	
	bool checkFeatures(VkPhysicalDevice phyDev) const noexcept;
	
	const auto &getFeatures2() const noexcept;
	
	
	
private:
	VkPhysicalDeviceSamplerYcbcrConversionFeatures _samplerYcbcrConversionStruct{
		VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES};
	VkPhysicalDeviceFeatures2 _devFeatures2{
		VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
	void *&_pNext;
public:
	
	VkPhysicalDeviceFeatures &_devFeatures;
	VkBool32 &_samplerYcbcrConversion;
};


struct vulkanAppInstance {
		
	typedef std::function<void(rvkAppWarningLevels, const char *)> warningCallback_t;
	
	vulkanAppInstance(const vulkanAppInstance &) = delete;
	vulkanAppInstance(vulkanAppInstance &&) = delete;
	vulkanAppInstance &operator=(const vulkanAppInstance &) = delete;
	vulkanAppInstance(std::vector<const char *> &vkReqExtensions, 
	const char **validationLayers, uint32_t validationLayerCount,
	VkAllocationCallbacks *pAllocator = nullptr, PFN_vkDebugUtilsMessengerCallbackEXT debugCallback = nullptr);
	virtual ~vulkanAppInstance() noexcept;
	
	void generateWarning(rvkAppWarningLevels wl, const char *msg) const noexcept;
	
	/* getXX functions are only thread safe if createLogicalDevice is called in
	 * the constructor of the derived class.
	 */
	const auto &getpAllocator() const noexcept;
	const auto &getLogicalDevice() const noexcept;
	const auto &getDeviceFeatures() const noexcept;
	const auto &getDeviceMemoryProps() const noexcept;
	void getFormatProperties(VkFormatProperties &props, VkFormat format) const noexcept;
	VkFormatProperties getFormatProperties(VkFormat format) const noexcept;
	const auto &getDeviceProperties() const noexcept;
	
protected:
	
	
	
	// should be called once only
	void createLogicalDevice(const VkPhysicalDevice &physicalDevice, const VkDeviceCreateInfo &dvciArg, 
		const rvkPhysicalDeviceFeatures &features, const VkPhysicalDeviceMemoryProperties &phyDevMemProps);
	
	const auto &getInstance() const noexcept;
	const auto &getDebugMessenger() const noexcept;
	auto getPhysicalDevice() const noexcept;
	
	
private:
	

	
private:
	VkAllocationCallbacks *_pAllocator = nullptr;
	VkInstance _instance = VK_NULL_HANDLE;
	rvkPhysicalDeviceFeatures _deviceFeatures;
	VkPhysicalDeviceMemoryProperties _deviceMemoryProps{};
	VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
	VkPhysicalDeviceProperties _deviceProperties{};
	//VkFormatProperties
#ifndef NDEBUG
	VkDebugUtilsMessengerEXT _debugMessenger = VK_NULL_HANDLE;
	PFN_vkDestroyDebugUtilsMessengerEXT _debugMessengerDestroyer = nullptr;
#endif
	VkDevice _logicalDevice = VK_NULL_HANDLE;

	warningCallback_t _warningCallback = [](rvkAppWarningLevels wl,const char *msg)
	{
		mainLogger.logText("calling _warningCallback \n");
		printf("warning level %llu: %s\n", wl, msg);
		mainLogger.logText("_warningCallback called \n");
	};

	
};

inline bool checkVkDeviceExtensions(const VkPhysicalDevice &physicalDevice, 
	const std::vector<const char *> &devReqExts) noexcept;



inline bool checkVkDeviceProperties( const VkPhysicalDeviceProperties &foundProps, 
	const VkPhysicalDeviceProperties &reqProps) noexcept;



template<typename appInstanceT>
inline VkSemaphore rvkCreateSemaphore(const std::shared_ptr<appInstanceT> &appInstance);


struct rvkCmdPool
{
	typedef std::shared_ptr<vulkanAppInstance> appInstance_t;
	
	rvkCmdPool(const rvkCmdPool &) = delete;
	rvkCmdPool(const rvkCmdPool &&) = delete;
	rvkCmdPool &operator=(const rvkCmdPool &) = delete;
	
	
	template<typename inputSharedAppInstanceT>
	rvkCmdPool(const inputSharedAppInstanceT &appInstance, uint32_t qfamilyIndex,
		VkCommandPoolCreateFlags flags = 0);
	
	void recreate();
	
	
	
	~rvkCmdPool();
	
	VkCommandPool getHandle();
private:
	VkCommandPoolCreateInfo _cmdPoolCI{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
	appInstance_t _appInstance;
	VkCommandPool _cmdPool = VK_NULL_HANDLE;
};


struct rvkCmdBuffers
{
	/* developer plan : add a mutex array to prevent aquiring more renderPassLock s
	 */
	typedef std::shared_ptr<vulkanAppInstance> appInstance_t;
	typedef std::shared_ptr<rvkCmdPool> cmdPool_t;
	
	rvkCmdBuffers(const rvkCmdBuffers &) = delete;
	rvkCmdBuffers(rvkCmdBuffers &&) = delete;
	rvkCmdBuffers &operator=(const rvkCmdBuffers &) = delete;
	
	template<typename inputSharedAppInstanceT>
	rvkCmdBuffers(const inputSharedAppInstanceT &appInstance, const cmdPool_t &cmdPool, 
	VkCommandBufferLevel level, uint32_t count);
	
	template<bool destroyPrevious = true>
	void recreate(const cmdPool_t &cmdPool, uint32_t newCount);
	
	
	template<bool destroyPrevious = true>
	void recreate(const cmdPool_t &cmdPool);
	
	
	// begin recording in an index
	void beginI(size_t index, VkCommandBufferUsageFlags flags);
	
	
	
	// end recording in one.
	void endI(size_t index);
	
	~rvkCmdBuffers();
	
	
	std::vector<VkCommandBuffer> _cmdBuffers;
private:
	VkCommandBufferAllocateInfo _allocInfo { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
	appInstance_t _appInstance;
	cmdPool_t _cmdPool;
};

struct _rvkImage_core
{
	_rvkImage_core(const _rvkImage_core &) = delete;
	_rvkImage_core &operator=(const _rvkImage_core &) = delete;
	
	_rvkImage_core(_rvkImage_core &&other);
	
	_rvkImage_core(const VkDevice &logicalDevice, VkAllocationCallbacks * const &pAllocator,
	const VkPhysicalDeviceMemoryProperties &deviceMemProps,
	const size_t &width, const size_t &height, const size_t &planeCount, const VkFormat &format, 
	const VkFormatProperties &formatProps, const VkImageTiling &tiling,
	const VkSharingMode &sharingMode, const VkImageUsageFlags &usageFlags, const VkMemoryPropertyFlags &memProps);
	
	void unmap();
	
	void  map(VkDeviceSize offset, VkDeviceSize size);
	
	void map();
	
	// adds barriers for all image planes, uses a sample for most parameters.
	void addBarriers_allPlanes(std::vector<VkImageMemoryBarrier> &barriers, const VkImageMemoryBarrier &sample,
		uint32_t baseMipLevel = 0, uint32_t levelCount = VK_REMAINING_MIP_LEVELS,
		uint32_t baseArrayLayer = 0, uint32_t layerCount = VK_REMAINING_ARRAY_LAYERS) const noexcept;
	
	void addBarriers_depthAndStencilAspect(std::vector<VkImageMemoryBarrier> &barriers, const VkImageMemoryBarrier &sample,
		uint32_t baseMipLevel = 0, uint32_t levelCount = VK_REMAINING_MIP_LEVELS,
		uint32_t baseArrayLayer = 0, uint32_t layerCount = VK_REMAINING_ARRAY_LAYERS) const;
	
	void addBarriers_depthAspect(std::vector<VkImageMemoryBarrier> &barriers, const VkImageMemoryBarrier &sample,
		const uint32_t &baseMipLevel = 0, const uint32_t &levelCount = VK_REMAINING_MIP_LEVELS,
		const uint32_t &baseArrayLayer = 0, const uint32_t &layerCount = VK_REMAINING_ARRAY_LAYERS) const;
	void addBarriers_stencilAspect(std::vector<VkImageMemoryBarrier> &barriers, const VkImageMemoryBarrier &sample,
		const uint32_t &baseMipLevel = 0, const uint32_t &levelCount = VK_REMAINING_MIP_LEVELS,
		const uint32_t &baseArrayLayer = 0, const uint32_t &layerCount = VK_REMAINING_ARRAY_LAYERS) const;
	
	
	void cmdCopyDepthToBuffer(const VkCommandBuffer &cmdBuff, const VkImageLayout &layout, const VkBuffer &dstBuff) const;
	
	void cmdCopyDepthToBuffer(const VkCommandBuffer &cmdBuff, const VkImageLayout &layout, const VkBuffer &dstBuff,
	const int32_t &x, const int32_t &y) const;
	
	auto const &getFormatFeatures() const noexcept;
	
	auto const &getFormat() const noexcept;
	auto getWidth() const noexcept;
	auto getHeight() const noexcept;
	
	VkDeviceSize _byteSize = 0;
	VkImage _hImage{};
	bool mapped = false;
	void *_pData = nullptr;
	
protected:
	void cleanup() noexcept;
	
private:
	void _cleanup() noexcept;
	
	VkDevice _hLogicalDevice;
	VkAllocationCallbacks *_pAllocator;
	
	VkDeviceMemory _imgMem{};
	VkFormatFeatureFlags _formatFeatures = 0;
	VkFormat _format;
	uint32_t _height = 0, _width = 0;
	size_t _planeCount;
	bool _moved = false;
};

template<bool doCleanup = true>
struct rvkImage_basic : _rvkImage_core
{
	rvkImage_basic(rvkImage_basic<doCleanup> &&other);
	rvkImage_basic(VkDevice logicalDevice, VkAllocationCallbacks *pAllocator,
	const VkPhysicalDeviceMemoryProperties &deviceMemProps,
	size_t width, size_t height, size_t planeCount, VkFormat format, 
	VkFormatProperties formatProps, VkImageTiling tiling,
	VkSharingMode sharingMode, VkImageUsageFlags usageFlags, VkMemoryPropertyFlags memProps);
	
	~rvkImage_basic();
};



struct _rvkBuffer_core
{
	_rvkBuffer_core(const _rvkBuffer_core &) = delete;
	_rvkBuffer_core &operator=(const _rvkBuffer_core &) = delete;
	
	_rvkBuffer_core( _rvkBuffer_core &&other );
	
	
	_rvkBuffer_core( VkDevice hLogicalDevice, VkAllocationCallbacks *pAllocator, VkDeviceSize byteSize, 
	const VkPhysicalDeviceMemoryProperties &deviceMemProps,
	VkSharingMode sharingMode, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags reqPropFlags);
	
	void unmap();
	
	void  map(VkDeviceSize offset, VkDeviceSize size);
	
	void map();
	
	// frequent calls not recommended, try to use a single vkFlushMappedMemoryRanges instead
	void flush(VkDeviceSize offset, VkDeviceSize size);
	
	void flushAll();
	
	void cmdBind(const VkCommandBuffer &cmdbuff, 
	uint32_t bindingIndex /*must be less than VkPhysicalDeviceLimits::maxVertexInputBindings*/) const noexcept;
	
	VkBuffer _hBuffer; // if this is set to VK_NULL_HANDLE, it could mean that the object has been moved.
	void *_pData = nullptr;
	VkDeviceSize _byteSize;
	VkDeviceMemory _mem;
	
protected:
	void cleanup() noexcept;
private:
	
	VkDevice _hLogicalDevice{};
	VkAllocationCallbacks *_pAllocator;
	
	void _cleanup() noexcept;
	bool _mapped;
};

template<bool doCleanup = true>
struct rvkBuffer_basic : _rvkBuffer_core
{
	rvkBuffer_basic( rvkBuffer_basic<doCleanup> &&other);
	
	
	rvkBuffer_basic( VkDevice hLogicalDevice, VkAllocationCallbacks *pAllocator, VkDeviceSize byteSize, 
	const VkPhysicalDeviceMemoryProperties &deviceMemProps,
	VkSharingMode sharingMode, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags reqPropFlags);
	
	~rvkBuffer_basic();
};

struct rvkBuffer : rvkBuffer_basic<false>
{
	
	rvkBuffer(const rvkBuffer &) = delete;
	//rvkBuffer( rvkBuffer && ) = delete;
	rvkBuffer &operator=(const rvkBuffer &) = delete;
	
	rvkBuffer( rvkBuffer &&other);
	
	template<typename appInstanceT>
	rvkBuffer( const std::shared_ptr<appInstanceT> &appInstance, VkDeviceSize byteSize, 
	VkSharingMode sharingMode, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags reqPropFlags);
	
	
	~rvkBuffer();
	
	
private:
	
	std::shared_ptr<vulkanAppInstance> _appInstance{};
};

struct rvkImage : rvkImage_basic<false>
{
	
	rvkImage(const rvkImage &) = delete;
	rvkImage &operator=(const rvkImage &) = delete;
	
	rvkImage(rvkImage &&other);
	
	rvkImage(const std::shared_ptr<vulkanAppInstance> &appInstance,
	const rvkColorPlaneLayout_t &cpl, const rvkFormatInterface &rvkFI, VkImageTiling tiling,
	VkSharingMode sharingMode, VkImageUsageFlags usageFlags, VkMemoryPropertyFlags memProps);
	
	rvkImage(const std::shared_ptr<vulkanAppInstance> &appInstance,
	size_t width, size_t height, size_t planeCount, VkFormat format, VkImageTiling tiling,
	VkSharingMode sharingMode, VkImageUsageFlags usageFlags, VkMemoryPropertyFlags memProps);
	
	~rvkImage();
	
	auto const &getAppInstance() const noexcept;
protected:
	std::shared_ptr<vulkanAppInstance> _appInstance{};
};

/* dev note :
 * submitIndex overflow is not considered.
 */
struct rvkFence_core
{
	rvkFence_core() = delete;
	rvkFence_core(const rvkFence_core &) = delete;
	rvkFence_core &operator=(const rvkFence_core &) = delete;
	
	typedef std::tuple<std::unique_lock<std::mutex>,std::unique_lock<std::mutex>> retLk_t;
	
	rvkFence_core(rvkFence_core &&other);
	
	rvkFence_core(VkDevice hLogicalDevice, VkAllocationCallbacks *pAllocator, bool signaled = false);
	/*the actual timeout duration may be twice as long.
	 */
	template<typename RepT, typename PeriodT>
	void waitFor(const std::chrono::duration<RepT,PeriodT> &timeoutDuration) const;
	
	
	
	/* unlocks the input lock after a success for lock of the internal lock. 
	 *	ensures lock is not released until fence into wait mode.
	 * returns the _submitIndex
	 * _submitIndex should be incremental(not necessarily increased by one) or 
	 *	some functions may not work properly.
	 * 
	 * the actual timeout duration may be twice as long.
	 */
	template<typename lockT, typename RepT, typename PeriodT>
	auto releaseAndWaitFor(lockT &ilk, const std::chrono::duration<RepT,PeriodT> &timeoutDuration) const;
	
	std::tuple<std::unique_lock<std::mutex>,std::unique_lock<std::mutex>> reset();
	
	/*
	 * _submitIndex should be incremental(not necessarily increased by one) or 
	 *	some functions may not work properly.
	 */
	void setIsSubmitted(uint64_t submitIndex);
	
	/* high performance version to check if a renderIndex for the retrieved fence
	 * has finished it's execution.
	 * when using circular resource buffers, it can be used to know if the 
	 * oldest resource can be assigned as free to use or not. although it should
	 * be considered that if the resource is used in a single render only, or in 
	 * all renders till the preparation of the new resource. 
	 */
	bool isFinished(uint64_t submitIndex) const;
	
	/* the actual timeout duration may be twice as long.
	 */
	template<typename RepT, typename PeriodT>
	void waitForFinish(uint64_t submitIndex, const std::chrono::duration<RepT, PeriodT> &timeoutDuration) const;
	
	VkFence getHandle() const noexcept;
	
	auto getLastSubmitIndex() const noexcept;
	
protected:
	
	void _cleanup();
	
private:
	
	// may unlock locks. returns the _submitIndex
	template<typename RepT, typename durationT>
	uint64_t _waitFor(std::unique_lock<std::mutex> &iwlk, std::unique_lock<std::mutex> &islk,
		const std::chrono::duration<RepT, durationT> &timeoutDuration) const;
	
	// keeps the locks locked.
	void _reset(std::unique_lock<std::mutex> &iwlk, std::unique_lock<std::mutex> &islk );
	
	/* locking order
	 * 1. _isWaitingM;
	 * 2. _isSubmittedM;
	 * 
	 */
	
	VkDevice _hLogicalDevice{};
	VkAllocationCallbacks *_pAllocator{};
	VkFence _handle{};
	mutable std::mutex _isWaitingM; // protects _isWaiting
	mutable std::condition_variable _isWaitingCV;
	mutable bool _isWaiting = false;
	mutable std::mutex _isSubmittedM; // protects _isSubmitted and _submitIndex
	mutable std::condition_variable _isSubmittedCV;
	bool _isSubmitted = false;
	std::optional<uint64_t> _submitIndex;
};

template<bool doCleanup = true>
struct rvkFence_basic : rvkFence_core
{
	rvkFence_basic() = delete;
	rvkFence_basic(const rvkFence_basic &) = delete;
	rvkFence_basic &operator=(const rvkFence_basic &) = delete;
	rvkFence_basic(rvkFence_basic &&other);
	rvkFence_basic(VkDevice hLogicalDevice, VkAllocationCallbacks *pAllocator, bool signaled = false);
	~rvkFence_basic();
};

struct rvkFence : rvkFence_basic<false>
{
	rvkFence(const rvkFence &) = delete;
	rvkFence &operator=(const rvkFence &) = delete;
	rvkFence(rvkFence &&other);
	template<typename appInstanceT>
	rvkFence(const std::shared_ptr<appInstanceT> &appInstance, bool signaled = false);
	
	~rvkFence() noexcept;
	
private:
	std::shared_ptr<vulkanAppInstance> _appInstance;
	
};


}
}
