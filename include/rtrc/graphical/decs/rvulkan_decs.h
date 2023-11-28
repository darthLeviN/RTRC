

#pragma once


#include <vulkan/vulkan.hpp>
#include "../../rexception.h"
#include <cstring>
#include <optional>
#include <set>
#include "../../rmemory.h"
#include <limits>
#include <algorithm>
#include <memory>
#include <type_traits>
#include <shared_mutex>
#include "../../rmutex.h"
#include "../../rtrc.h"
#include <cstdint>
#include <thread>
#include <condition_variable>
#include <chrono>
#include <functional>
#include "../rvulkan_core.h"
#include "../rimage_core.h"
#include "../rvulkan_image.h"
#include "../rvkBits.h"
#include "../../dcsynch.h"
#include <queue>
#include <boost/circular_buffer.hpp>
#include <glm//glm.hpp>
#include "../../compiletime.h"

using namespace rtrc::memoryl;
using namespace rtrc::rimagel;
namespace rtrc { namespace vkl {
	
/* Bugs :
 * Some functions that output in their vector arguments, have the permission to 
 * resize these vectors, this is not an ideal situation. design an alternative
 * dynamic container that does not allow resizing.
 */
template<typename firstGrPipeT, typename ...grPipeGroupTs>
struct renderPassLock
{
	/* bug : a mechanism is needed to prevent creating two locks on the same command buffer.
	 * validation layers may help with this, but it is recommended to create a 
	 * mutex array for rkvCmdBuffers
	 */
	template<typename pT>
	using perPassBuffer_t = typename pT::perPassBuffer_t;

	
	static constexpr size_t pipeGroupCount = 1 + sizeof...(grPipeGroupTs);
	
	typedef std::function<void(VkCommandBuffer)> postRenderProc_t;
	
	
	renderPassLock(const renderPassLock &) = delete;
	renderPassLock &operator=(const renderPassLock &) = delete;
	
	// move constructor
	renderPassLock(renderPassLock &&moved);
	
	// main constructor
	renderPassLock(const uint32_t &imageIndex, const bool &updateViewport, 
		const std::shared_ptr<rvkCmdBuffers> &cmdBuffers,
	std::shared_lock<std::shared_mutex> &&pL, const std::shared_ptr<char> &scFeed,
	const std::shared_ptr<perPassBuffer_t<firstGrPipeT>> &fperPassBuffer,
	const std::shared_ptr<grPipeGroupTs> &...otherGrPipeGroups);
	
	
	~renderPassLock();
	
	// gets a command buffer of a pipe index as a template argument.
	template<size_t pipeIndex>
	const VkCommandBuffer &getCmdBuffer() const;
	
	// gets a command buffer of a pipe index as a function argument.
	const VkCommandBuffer &getCmdBuffer(size_t pipeIndex) const;
	
	// gets a perPassBuffer of a pipe index as a template argument.
	// note pipe index cannot be a function argument because that would mean
	// having different return types.
	template<size_t pipeIndex>
	const auto &getPerPassBuffer();
	
	// unlocks the main lock. call once after no member function needs to be 
	// accessed anymore.
	void unlock();
	
	// call before unlock to issue the end of the recording of all commands 
	// buffers.
	void endRecording();
	
	
	bool isLocked() noexcept;
	
	uint32_t getImageIndex() noexcept;
	auto getScFeed() noexcept;
	std::shared_lock<std::shared_mutex> &getLock() noexcept;
	
	const bool _updateViewport = false;
private:

	uint32_t _imageIndex;
	/* developer note : shared pointers of _cmdBuffers and _perPassBuffer can be grouped together as a single one
	 * try merging them if better performance is required. doing so will require modifying the pipeline object
	 */
	std::array<std::shared_ptr<rvkCmdBuffers>, pipeGroupCount> _cmdBuffersArr;
	std::tuple<std::shared_ptr<perPassBuffer_t<firstGrPipeT>>, std::shared_ptr<perPassBuffer_t<grPipeGroupTs>>...> _perPassBuffers;
	std::shared_lock<std::shared_mutex> _pL;
	//std::shared_lock<std::shared_mutex> _pipeRecreationLock;
	bool _jobEnded = false;
	
	std::shared_ptr<char> _scFeed;
};

template<typename ...grPipeGroupTs>
using rvkPostRenderCopyRequest_t = std::function<void(std::vector<VkCommandBuffer> & /*renderCmds*/, 
	std::vector<VkCommandBuffer> &/*postRcmds*/, renderPassLock<grPipeGroupTs...> &rPL )>;

struct swapchainImgVGroup
{
	VkImageView colorV = VK_NULL_HANDLE;
	VkImageView depthV = VK_NULL_HANDLE;
};

/* a wsi vulkan instance.
 * more details will be added in future.
 */
template<typename windowT>
struct rvkWSIAppInstance : vulkanAppInstance
{
	typedef windowT window_t;
	//typedef std::unique_ptr<windowT> window_t;
	typedef typename window_t::resizeCallback_t resizeCallback_t;
	typedef typename window_t::setResizeCallback_t setResizeCallback_t;
	
	// surfaceCreator must be callable with (VkInstance, VkSurfaceKHR *, VkAllocationCallbacks *) and return VkResult
	//template<typename surfaceCreator_t>
	rvkWSIAppInstance(std::vector<const char *> &vkReqExtensions,
	const char **validationLayers, uint32_t validationLayerCount,
	VkAllocationCallbacks *pAllocator, const rvkPhysicalDeviceFeatures &reqDevFeatures,
	PFN_vkDebugUtilsMessengerCallbackEXT debugCallback, //const windowDestroyer_t &windowDestroyer,
	std::unique_ptr<windowT> &&window, //const setResizeCallback_t &setResizeCallback,
	//surfaceCreator_t surfaceCreator,
	const swapchainProperties &swapChainOptions);
	
	virtual ~rvkWSIAppInstance() noexcept;
	
	/* updates swapchain and notifies waiting functions.
	 * call to this is thread safe.
	 * blocks until the swapchain is recreated.
	 */
	void updateSwapchain(VkExtent2D &reqExtent);
	
	// a compile time programmed basic function to rate physical devices.
	static constexpr uint32_t ratePhysicalDevice(const VkPhysicalDeviceProperties &deviceProps) noexcept;
	
	
private:
	
	// can also be called after locking _statesMutex
	// creates/recreates swapChain. but does nothing notify anything else.
	void _createSwapchain( VkExtent2D &reqExtent);
	
	void _cleanupSwapchain() noexcept;
	
	
	bool checkswapChainSupportDetails(const swapChainSupportDetails &swcsd, 
	VkPhysicalDevice physicalDevice) const noexcept;
	
	// creates the logical device, call once in constructor.
	void _createlogicalDeviceWSI(const rvkPhysicalDeviceFeatures &reqFeatures = {}, 
	const VkPhysicalDeviceProperties &reqProps = {});
	
	/* recreates the swapchain depth images. must call _cleanUpSwapchainImages()
	 * after use in destructor and before reuse. 
	 */
	void _recreateSwapchainDepthImages(size_t imageCount);
	
	// Clean up created image views after use in destructor and before reuse. 
	// or modify this for automatic cleanup.
	void _recreateSwapchainImgsAndImgVs();
	
	// returns the swapchain color images
	std::vector<VkImage> _getSwapchainColorImages() noexcept;
	
	// cleans up manually allocated swapchain images
	void _cleanUpSwapchainImages() noexcept;
	
	// cleans up swapchain image views
	void _cleanUpSwapchainImgvs() noexcept;
	
	// Cleans up presentation semaphores for ALL of the swapchain images.
	void _cleanUpSwapchainPresentSemaphores() noexcept;
	
	/* changes the swapchain presentation semaphores with new ones.
	 * old semaphores for the same image index, can be freed safely.
	*/
	void _changeSwapchainPresentSemaphores(uint32_t imageIndex, uint32_t semCount, VkSemaphore *sems);
	
	// Configures the swapchain image settings in _swapchainOptions.
	bool _findSwapchainDepthFormatAndTiling(VkPhysicalDevice phyDev);
	
public:
	
	// if swapchain needs to be reconfigured, returns true. in that case, the readerLock gets unlocked(invalidated)
	bool acquireNextImageKHR(std::shared_lock<std::shared_mutex> &readerLock, 
		uint32_t &imageIndex, VkSemaphore &semaphore, VkFence fence, uint64_t timeout =  UINT64_MAX);
	
	// the lock guarantees that from the time that the image was aquired, there was no swapchain reconstruction.
	void singlePresent(std::shared_lock<std::shared_mutex> &readerLock,
	uint32_t imageIndex, uint32_t semaphoresCount, VkSemaphore *pSemaphores);
	
	VkResult submitToGraphics(uint32_t submitCount, const VkSubmitInfo *pSubmits, VkFence fence);
	
	void waitForGraphicsIdle();
	
	void waitForPresentIdle();
	
	
	/* getLastSwapchainOptions and getLastSwapchainXXXXImageViews (commonly used by the pipeline managing struct)
	 * these two functions are not thread safe alone, they should be used along with _swapchainUrMutex lock.
	 * proper calling mechanism of these functions in a safe way has not been decided yet.
	 */
	const auto &getLastSwapchainOptions() const noexcept;
	const auto &getLastSwapchainImageViews() const noexcept;
	const auto &getLastSwapchainDepthImgs() const noexcept;
	
	// don't use queue data directly for operations that require synchronization with the queue in the vulkan specifications.
	const auto &getGraphicsQueueIndex() const noexcept;
	auto getGraphicsQueue() const noexcept;
	const auto &getPresentQueueIndex() const noexcept;
	auto getPresentQueue() const noexcept;
	
	auto &getWindow() noexcept;
	
	// acquires a reader lock, waits and blocks if the swapchain is outdated to prevent repeating errors.
	bool getSwapchainReaderLock(std::shared_lock<std::shared_mutex> &readerLock, 
	updatableResourceMutex::feed_t &feed);
	// keep reader locks ununlockd till all store operations have been completed.
private:
	
	/* mutex lock order : 
	 * 1._statesMutex lock
	 * 2. writer _swapchainUrMutex lock
	 * 3. any order unlock.
	 * 
	 * 1. reader _swapchainUrMutex input unlocking (future development doesn't allow relocking)
	 * 2._statesMutex lock
	 * note : this means readers are not allowed
	 * 
	 * 1._statesMutex lock
	 * 2. reader _swapchainUrMutex lock output.
	 */
	
	mutable std::mutex _statesMutex;
	bool _swapchainUptoDate = false;
	bool _swapchainOutOfDate = true;
	std::condition_variable _swapchainUpdateCV;
	
	mutable updatableResourceMutex _swapchainUrMutex;
	swapchainProperties _swapchainOptions;
	VkSwapchainKHR _lastSwapchain = VK_NULL_HANDLE;
	std::vector<rvkImage_basic<>> _swapchainDepthImgs;
	
	//std::vector<VkImageView> _swapchainColorImgvs;
	//std::vector<VkImageView> _swapchainDepthImgvs;
	std::vector<swapchainImgVGroup> _swapchainImgVs;
	std::vector<std::vector<VkSemaphore>> _swapchainPresentSemaphores;
	
	
	swapChainSupportDetails _swapchainSupDetails; // uninitialized.
	VkSurfaceKHR _khrSurface = VK_NULL_HANDLE;
	uint32_t _graphicsQueueIndex = 0;
	std::mutex _graphicsQueueM; // synchronizes the access to _graphicsQueue
	VkQueue _graphicsQueue = VK_NULL_HANDLE;
	uint32_t _presentQueueIndex = 0;
	std::mutex _presentQueueM; // synchronizes the access to _presentQueue
	VkQueue _presentQueue = VK_NULL_HANDLE;
	
	
	//windowDestroyer_t _windowDestroyer;
	std::unique_ptr<window_t> _window;
	//setResizeCallback_t _setResizeCallback;
	resizeCallback_t _defaultResizeCallback = [this](int width, int height)
	{
		VkExtent2D extent;
		extent.width = width;
		extent.height = height;
		this->//createSwapchain(extent);
			updateSwapchain(extent);
	};
};



/* basic shader module, the argument ensures correct placement, because the 
 * placement is required to be known at compile time.
 * 
 * a rvkShaderModule instance is meant to be immutable. so it can be safely shared
 */
template<VkShaderStageFlagBits stageBits>
struct rvkShaderModule
{
	static constexpr auto _stageFlag = stageBits;
	
	rvkShaderModule(const rvkShaderModule &) = delete;
	rvkShaderModule( rvkShaderModule &&) = delete;
	rvkShaderModule &operator=(rvkShaderModule &) = delete;
	template<typename appInstanceT>
	rvkShaderModule(const std::shared_ptr<appInstanceT> &appInstance, const std::vector<char> &code, 
	const char *pName = "main");
	
	~rvkShaderModule();
	/*VkShaderModule createShaderModule(const std::vector<char> &code)
	{
		
	}*/
	auto &getShaderModule();
	auto &getAppInstance();
	const char *getpName();
	VkPipelineShaderStageCreateInfo getShaderStageCI() const noexcept;
	
private:
	VkShaderModule _shaderModule;
	std::shared_ptr<vulkanAppInstance> _appInstance; // cheap but effective way for managing reasources.
	const char * _pName = "main";
};

struct rvkPerPassVertexBindInfo
{
	std::vector<VkBuffer> buffers;
	std::vector<VkDeviceSize> offsets;
};

/* perPassBufferT struct must be defined by the user, and have the following members :
 * void mapDescSetWrite(VkWriteDescriptorSet &) const : modifies custom user 
 * values to real values.
 * 
 * template<appInstanceT>
 * perPassBufferT(const std::shared_ptr<appInstanceT> &, 
 *	const std::vector<VkDescriptorSet> &descSets) : default constructor, creates the related buffers to a render pass.
 * 
 * ~perPassBufferT() noexcept : destructor that frees the buffers and command buffers.
 * 
 * 
 * writeCmd_t : specifies the type of the write commands that cmdWrite(VkCommandBuffer, const writeCmd_t &) takes
 * 
 * std::vector<VkSubmitInfo> cmdWrite(const writeCmd_t &, uint64_t renderIndex,
 *	const std::shared_ptr<rvkFence> &prevFence, // previous submitted render pull
 *	const std::shared_ptr<rvkFence> &curFence, // current unsubmitted render pull
 *	std::vector<VkSemaphore> &reservedSems, 
 *	std::vector<VkPipelineStageFlags> &renderWaitFlags, 
 *	std::vector<VkSemaphore> &renderWaitSems) : 
 * creates a VkSubmitInfo structs, edits the vectors to take reserved semaphores 
 * and add wait semaphores and flags for each VkSubmitInfo generated.
 * if there is not enough reservedSemaphores, the function has to create it's own,
 * if there is an error while creating any, it should take care of putting the 
 * semaphores in renderWaitSems so they can be freed or put them back in reervedSems(recommended)
 * also this function should reserve at least three extra VkSubmitInfos in the 
 * returned vector for better performance.
 * 
 * rvkPerPassVertexBindInfo getVertexBuffers() const :
 * binds vertex data which are used for instanced rendering.
 * 
 * it is also recommended to assign all automatic generated constructors as deleted.
 * 
 * responsibilities : 
 * 1. render puller is responsible for synchronization of buffer usage.
 * 2. render puller must take care of the submission order of the returned 
 *	vector when using their output semaphores.
 * 3. render puller is responsible to destroy the output semaphores after use.
 * 4. pipeline object is responsible for creating an array of this type for each 
 *	pass(swapchain image) and choosing which one to bind before passing a lock 
 *	to the render puller.
 * 5. the settings struct type of the pipeline must have the type accessible as 
 *	::perPassBuffer_t
 * 6. render puller must lock the swapchain in read mode while calling cmdWrite.
 */

/* commonVkPipeSettings.
 * must have a virtual destructor if it has any.
 * also, it should manage resources related to settings like samplers and buffers
 * and images (if the images are not managed by the samplers themselves)
 */
template<typename perPassBufferT>
struct commonVkPipeSettings
{
	typedef perPassBufferT perPassBuffer_t;
	
	// i am unsure wether to unify samplers, buffers and extraResources or not.
	commonVkPipeSettings(std::vector<VkVertexInputBindingDescription> &&bindingDescs, 
	std::vector<VkVertexInputAttributeDescription> &&attrDescs, std::vector<std::vector<VkDescriptorSetLayoutBinding>> &&descSetLBsArr,
		rvkDescSameUpdater &&descSetsConfigurator,
		std::vector<std::shared_ptr<void>> &&extraResources = {});
	
	
	
	bool dynamicViewport();
	// describes vertex attributes
	pipeVertexInputStateCI vertexInputStateCI();
	
	// describes the drawing method.
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCI();
	
	// cheap temporary function
	std::vector<VkViewport> viewPorts(const swapchainProperties &scopts);
	
	
	// defines viewport clip.
	pipeViewportStateCI viewportStateCI(const swapchainProperties &scopts);
	
	// defines rasterizer.
	VkPipelineRasterizationStateCreateInfo restrizationStateCI();
	
	// defines multisampling
	VkPipelineMultisampleStateCreateInfo multisamplerStateCI();
	
	// defines color blend settings
	colorBlendStateCI colorBlendAttState();
	
	// descriptor sets create info.
	std::vector<descSetLayoutCI> descSetLayout();
	
	VkPipelineDepthStencilStateCreateInfo depthStencilStateCI();
	
	// defines 
	rvkRenderPassCI renderPassCI(const swapchainProperties &scopts);
	
	// this will get more arguments in the future.
	std::vector<VkFramebufferCreateInfo> framebuffersCI(VkRenderPass &renderPass, 
	const std::vector<swapchainImgVGroup> &swapchainImageViews, const swapchainProperties &scopts);
	
	rvkRenderPassBeginInfo renderPassBeginInfo(VkRenderPass renderPass, 
	VkFramebuffer framebuffer, const swapchainProperties &scopts);
	
	const rvkDescSameUpdater &getDescSetsConfigurator();
private:
	
	/* describes the data in attached images.
	 * An attachment description describes the properties of an attachment 
	 * including its format, sample count, and how its contents are treated 
	 * at the beginning and end of each render pass instance.
	 * 
	 * indexes in the attachmentDescription array, are referred by VkAttachmentReference structs.
	 */
	std::vector<VkAttachmentDescription> attachmentDescriptions(const swapchainProperties &scopts);
	
	/* VkSubPassDependecies handle synchronization between pipeline Stages
	 */
	std::vector<VkSubpassDependency> subpassDependencies();
	
	/* indexes in subpass's pColorAttachments desc are directly referenced in 
	 * fragment shader's output location.
	 * A subpass represents a phase of rendering that reads and writes a subset 
	 * of the attachments in a render pass.
	 * 
	 * a subpass description specifies the index of it's attachments and their layout.
	 */
	rvkSubpassDescs subpassDescs();
	
	std::vector<VkVertexInputBindingDescription> _inputBindingDescs;
	std::vector<VkVertexInputAttributeDescription> _inputAttrDescs;
	std::vector<std::vector<VkDescriptorSetLayoutBinding>> _descSetLBsArr;
	rvkDescSameUpdater _descSetsConfigurator;
	std::vector<std::shared_ptr<void>> _extraResources;
};








template<typename settingsT, typename appInstanceT>
struct rvkGrPipeGroup : settingsT
{
	/* the functionality of shared pointers is required to make it possible to
	 * use avoid duplication.
	 * 
	 * multiple pipelines are meant to be used by a single puller, pipelines are
	 * not meant to be shared among pullers.
	 */ 
	using type = rvkGrPipeGroup<settingsT, appInstanceT>;
	typedef std::shared_ptr<appInstanceT> appInstance_t;
	typedef std::shared_ptr<rvkShaderModule<VK_SHADER_STAGE_VERTEX_BIT>> vrShader_t;
	typedef std::shared_ptr<rvkShaderModule<VK_SHADER_STAGE_FRAGMENT_BIT>> frShader_t;
	typedef std::shared_ptr<rvkShaderModule<VK_SHADER_STAGE_GEOMETRY_BIT>> geoShader_t;
	typedef settingsT settings_t;
	typedef typename settings_t::perPassBuffer_t perPassBuffer_t;
	typedef std::function<void()> recordCallerFlusher_t; // used when pipeline recreation upon requestis needed so locks from previous pulls can be freed.
	
	
	
	static constexpr size_t circularPostRenderCopyCmdArrayCount = 3;
	static constexpr size_t maximumOnFlyRendering = 1; // if puller uses a higher number, then it is a problem.
	
	rvkGrPipeGroup(const rvkGrPipeGroup &) = delete;
	rvkGrPipeGroup(rvkGrPipeGroup &&) = delete;
	rvkGrPipeGroup &operator=(const rvkGrPipeGroup &) = delete;
	
	
	rvkGrPipeGroup(const appInstance_t &appInstance, settings_t &&settings,const vrShader_t &vrShader, const frShader_t &frShader,
		const geoShader_t &geoShader = {});
	
	
	// it's unlikely that it happens but having a writeLock to the swapchain will result in a trap when calling this.
	template<typename ...Args> // Args are shared pointer types of the next graphics pipelines
	auto beginRenderRecording(VkSemaphore &khrSemaphore, VkFence khrFence, VkSubpassContents contents,
	recordCallerFlusher_t flusher = {}, const std::shared_ptr<Args> &... otherGrPipeGroups);
	
	/*void grCmdDraw(renderPassLock<1> &rpL, uint32_t vertexCount, uint32_t instanceCount,
		uint32_t firstVertex, uint32_t firstInstance)
	{
		vkCmdDraw(rpL.getCmdBuffer(), vertexCount, instanceCount,
			firstVertex, firstInstance);
	}*/
	std::vector<VkViewport> viewPorts();
	
	virtual ~rvkGrPipeGroup();
	
	const auto &getPerPassBuffer(const size_t &imageIndex);
	
	const auto &getDefaultCmdBuffers();
	
	//auto &getSettingsManager() { return _settings; }
private:
	void grbindCmdBuffer(VkCommandBuffer cmdBuffer);
	
	/* use a external flusher function before use to make sure pipeline isn't 
	 * in use anymore.
	 */
	void _reCreate();
	
	template<bool lockSwapchain = false>
	void _create();
	
	void _cleanupPipe();
	
	void _createPrimCmdPoolAndBuffers();
	
	void _reconfigureSwapchain();
	
	void _destroyFramebuffers() noexcept;
	
	void _createPipelineLayout( const VkPipelineLayoutCreateInfo &pipeLayoutCI);
	
	void _createRenderPass( const rvkRenderPassCI &renderPassCI);
	
	updatableResourceMutex::feed_t _swapchainRefreshFeed;
	appInstance_t _appInstance;
	//settings_t _settings;
	vrShader_t _vrShader;
	frShader_t _frShader;
	geoShader_t _geoShader;
	
	/* descriptor sets management has been kept separate from the perPassBuffer,
	 * but they will be referenced by perPassBuffer. the management is kept 
	 * separate to simply usage of redinition of pipe settings and prePassBuffer.
	 */
	VkDescriptorPool _descPool = VK_NULL_HANDLE;
	std::vector<VkDescriptorSetLayout> _descSetLayouts;
	std::vector<std::vector<VkDescriptorSet>> _descSets; // a std::vector<VkDEscriptorSet> for each swapchain image.
	VkPipelineLayout _pipelineLayout = VK_NULL_HANDLE;
	VkRenderPass _renderPass = VK_NULL_HANDLE;
	std::vector<VkFramebuffer> _swapchainFramebuffers;
	VkPipeline _graphicsPipeline = VK_NULL_HANDLE;
	std::shared_ptr<rvkCmdPool> _defaultCmdPool;
	std::shared_ptr<rvkCmdBuffers> _defaultCmdBuffers;
	std::vector<std::shared_ptr<perPassBuffer_t>> _perPassBuffers;
	

};

/* handles pulling in the graphics queue.
 */

template<typename appInstanceT, typename ...grPipeGroupTs>
struct mainGraphicsPuller
{
	typedef appInstanceT appInstance_t;
	//template<size_t i>
	//using grPipe_t = std::tuple_element<i,std::tuple<grPipeGroupTs...>>;
	template<typename pipeT>
	using pipeSettings_t = typename pipeT::settings_t;
	//template<size_t i>
	//using pipeSettings_t = pipeSettings_t<grPipe_t<i>>;
	//template<size_t i>
	//using perPassBuffer_t = typename pipeSettings_t<i>::perPassBuffer_t;
	//template<size_t i>
	//using perPassWriteCmd_t = typename perPassBuffer_t<i>::writeCmd_t;
	template<typename pipeT>
	using perPassBuffer_t = typename pipeSettings_t<pipeT>::perPassBuffer_t;
	template<typename pipeT>
	using perPassWriteCmd_t = typename perPassBuffer_t<pipeT>::writeCmd_t;
	
	using renderPassLock_t = renderPassLock<grPipeGroupTs...>;
	static constexpr size_t pipeGroupCount = sizeof...(grPipeGroupTs);
	
	using grPipesTuple_t = std::tuple<const std::shared_ptr<grPipeGroupTs> ...>;
	
	static constexpr size_t reusableCount = 3;
	static constexpr size_t prevDumpCount = reusableCount - 1;
	
	mainGraphicsPuller(const mainGraphicsPuller &) = delete;
	mainGraphicsPuller(mainGraphicsPuller &&) = delete;
	mainGraphicsPuller &operator=(const mainGraphicsPuller &) = delete;
	
	mainGraphicsPuller(const std::shared_ptr<appInstance_t> &appInstance, 
		const std::shared_ptr<grPipeGroupTs> &... grPipeGroups);
	
	~mainGraphicsPuller() noexcept;
	
	template<typename cmdCallableT>
	void graphicsPull(cmdCallableT &drawPull);
	
	// the returned semaphores are experimental because there is no clear usage to them to make a proper design.
	// the current version is made to have only one thread running a graphicsPull
	template<typename ...cmdCallableTs>
	void graphicsPull(const std::tuple<cmdCallableTs...> &drawPulls,
	std::array<std::vector<VkSemaphore>,pipeGroupCount> &&renderWaitSems, std::array<std::vector<VkPipelineStageFlags>,pipeGroupCount> &&renderWaitFlags,
	const std::tuple<perPassWriteCmd_t<grPipeGroupTs>...> &perPassWrites) noexcept;
	
	void renderLessPull();
	
	/* queues a copy request of the render results. in the current version the 
	 * creator of the request is responsible to access the content of pipeIndex 
	 * only. this is unsafe and will be fixed in future.
	 * 
	 * these copy requests can be inserted in the back or front of the post 
	 * render commands depending on the callback.
	 */
	auto queueAndWaitPostRenderCopyRequest(size_t pipeGroupIndex, rvkPostRenderCopyRequest_t<grPipeGroupTs...> prcr);
	
	/* queues pre render commands. pre render commands, run after the previous 
	 * render or post render commands have completed their excetion
	*/
	void queuePreRenderCmd(size_t pipeIndex, const std::vector<VkCommandBuffer> &cmdBuff, std::vector<VkSemaphore> &&preRenderWaitSems,
		const std::vector<VkPipelineStageFlags> &preRenderWaitFlags);
	
	/* queues post render commands. they are executed before the next pre 
	 * render or render command.
	 */
	void queuePostRenderCmd(size_t pipeIndex, const std::vector<VkCommandBuffer> &cmdBuff, std::vector<VkSemaphore> &&postRenderWaitSems,
	const std::vector<VkPipelineStageFlags> &postRenderWaitFlags);
	
	
	// untested function. waits for a render pull to start. 
	void waitForARenderPullStart();
	
	// untested function. waits for a render pull to end.
	void waitForARenderPullEnd();
	
	// gets the current fence. and accesses it's info in a thread safe manner.
	// note : don't keep the lock for a long time to not block rendering.
	std::shared_ptr<const rvkFence> getCurrentFence(std::unique_lock<std::mutex> &outlk) const noexcept;
	
	// getCurrentFence overload
	void getCurrentFence(std::shared_ptr<const rvkFence> &ret, std::unique_lock<std::mutex> &outlk) const noexcept;
	
	// getCurrentFence overload
	const auto &getCurrentFenceRef(std::unique_lock<std::mutex> &outlk) const noexcept;
	

private:
	
	// simple utility function 
	void _queuePostRenderCmd(size_t pipeIndex, const std::vector<VkCommandBuffer> &cmdBuff, std::vector<VkSemaphore> &&postRenderWaitSems,
	const std::vector<VkPipelineStageFlags> &postRenderWaitFlags);//, std::vector<std::unique_lock<std::mutex>> &&finishLocks = {})
	
	
	// simple utility function
	void _queuePreRenderCmd(size_t pipeIndex, const std::vector<VkCommandBuffer> &cmdBuff, std::vector<VkSemaphore> &&preRenderWaitSems,
		const std::vector<VkPipelineStageFlags> &preRenderWaitFlags);
	
	// adds a local semaphore to a semaphore vector. maybe replaced with better
	// alternatives in future.
	VkSemaphore &addSemaphore(std::vector<VkSemaphore> &vec);
	
	/* mutex lock needed : _renderPullQueueMutex
	 * after calling this, the new fence may be selected with _allRF()
	 * it should be noted that after submitting, _allRF()->setIsSubmitted must 
	 * be called. 
	 */ 
	#ifdef NDEBUG
	void _selectNextFence() noexcept;
	#else
	void _selectNextFence();
	#endif
	
	
	// relates semaphores to a specific fence
	// call only once for each fence
	// mutex lock needed : _renderPullQueueMutex(if the render pulls are not in a single thread)
	void _addCurrentFenceDump( std::array<std::vector<VkSemaphore>,pipeGroupCount> &&prevPreRenderWaitSems ,
	std::array<std::vector<VkSemaphore>,pipeGroupCount> &&prevRenderWaitSems,
	std::array<std::vector<VkSemaphore>,pipeGroupCount> &&prevPostRenderWaitSems,
	std::vector<VkSemaphore> &&prevExtraSems )
	#ifdef NDEBUG
	noexcept
	#endif
	;
	
	// cleans up a fence
	// mutex lock needed : _renderPullQueueMutex(if the render pulls are not in a single thread)
	void _cleanupFence(size_t selection);
	// cleans up the entire render queue and some other resources. commonly used for swapchain reset to 
	//make sure resources are not in use by command buffers.
	// mutex lock needed : _renderPullQueueMutex(if the render pulls are not in a single thread)
	void _cleanupRenderQueue() noexcept;
	
	auto _prevFenceSelection() const noexcept;
	auto _nextFenceSelection() const noexcept;
	auto &_allRF() noexcept;
	const auto &_allRF() const noexcept;
	auto &_prevAllRF() noexcept;
	auto &_nextAllRF() noexcept;
	
	
	template<size_t increaseSizeIfEmpty = 0>
	void _reFillReservedSems();
	const std::shared_ptr<appInstance_t> _appInstance;
	
public:
	// these fences should be added to allSemFences arguemnt in graphicsPull() after each successful submit.
private:
	
	
	grPipesTuple_t _grPipesTuple;
	
	//std::array<std::array<VkFence,1>, reusableCount> _reusableFences{}; // zero initialize is the same as VK_NULL_HANDLE
	std::vector<std::shared_ptr<rvkFence>> _reusableFences;
	//mutable std::array<std::array<std::shared_mutex,1>, reusableCount> _reusablesMs;
	uint64_t _renderPullcounter = 0;
	//std::mutex gm; // for manipulating internal data.
	
	std::array<bool, prevDumpCount> _prevCleanedUp{}; // if _prevXXX[i] is free to use.
	std::array<std::vector<VkSemaphore>,prevDumpCount> _prevPreRenderWaitSems;
	std::array<std::vector<VkSemaphore>,prevDumpCount> _prevRenderWaitSems;
	std::array<std::vector<VkSemaphore>,prevDumpCount> _prevPostRenderWaitSems;
	std::array<std::vector<VkSemaphore>,prevDumpCount> _prevExtraSems;
	VkSemaphore _nextRenderWaitSem = VK_NULL_HANDLE; // currently not synched for multithreading. access only in render pull
	//std::vector<std::unique_lock<std::mutex>> _prevLocks;
	
	//
	//
	//
	mutable std::mutex _renderPullQueueMutex; // sychs the below variable group
	std::condition_variable _renderPullQueueStartCV;
	std::condition_variable _renderPullQueueEndCV;
	std::condition_variable _renderOpFinishedCV;
	std::array<std::vector<VkCommandBuffer>,pipeGroupCount> _preRenderCmds;
	std::array<std::vector<VkCommandBuffer>,pipeGroupCount> _postRenderCmds;
	std::array<std::vector<VkSemaphore>,pipeGroupCount> _preRenderWaitSems;
	std::array<std::vector<VkPipelineStageFlags>,pipeGroupCount> _preRenderWaitFlags;
	std::array<std::vector<VkSemaphore>,pipeGroupCount> _postRenderWaitSems;
	std::array<std::vector<VkPipelineStageFlags>,pipeGroupCount> _postRenderWaitFlags;
	std::array<std::vector<rvkPostRenderCopyRequest_t<grPipeGroupTs...>>,pipeGroupCount> _postRenderCopyRequests;
	
	size_t _reusableFenceSelection = 0;
	// if !has_value() then the fence is reset and it's queue is cleaned up(doesn't have one anymore).
	std::array<std::optional<size_t>, reusableCount> _reusableDumpSelection{};
	//
	//
	//
	
	size_t _reservedSemsStorageSize = 10;
	std::vector<VkSemaphore> _reservedSems;
	
	#include "complex_decs/rvulkan_complex_decs_1.h"
};

struct rvkStagedImage : rvkImage
{
	
	
	rvkStagedImage(const std::shared_ptr<vulkanAppInstance> &appInstance,
	const size_t &width, const size_t &height, const size_t &planeCount, const VkFormat &format, VkImageTiling tiling,
	VkSharingMode sharingMode, VkImageUsageFlags usageFlags);
	
	rvkStagedImage(const std::shared_ptr<vulkanAppInstance> &appInstance,
	const rvkColorPlaneLayout_t &cpl, const rvkFormatInterface &rvkFI, VkImageTiling tiling,
	VkSharingMode sharingMode, VkImageUsageFlags usageFlags);
	
	rvkStagedImage( rvkStagedImage &&other);
	
	~rvkStagedImage();
	
	/* loads the image from the CPU side into a host_coherent and host_visible memory.
	 * next calls to this function will throw if a device load has not been called. this is to prevent any data overwriting.
	 * synchornization : image buffer shouldn't be in use.
	 */
	template<typename loaderCallableT> // a callable with a void * argument.
	void loadStaged_host(const loaderCallableT &loaderCallable);
	
	/* loads the image from the CPU side into a host_coherent and host_visible memory.
	 * fails if called before any call to loadStaged_host after construction.
	 * tries to safely override with data if it wasn't referenced in the device.
	 * return : true of attempt was successful, false if it wasn't.
	 * synchornization : image buffer shouldn't be in use.
	 */
	template<typename loaderCallableT> // a callable with a void * argument.
	bool try_overwrite_loadStaged_host(const loaderCallableT &loaderCallable);
	
	/* records a device load command into the command buffer if it has not been
	 * recorded before. a faster alternative is to use 
	 * preRecord_loadStaged_device once and use informDeviceLoad() instead.
	 * return : true if a recording was done on the command buffer.
	 * dev note : do not include internal command buffers as access to them
	 * cannot be synchronized in a flexible way.
	 */
	void loadStaged_device_cmd(const VkCommandBuffer &cmdBuffer, const rvkBufferImageCopyInfo &bici, 
	VkImageLayout srcImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		VkImageLayout dstImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
	
	// use with care (experimental).
	// pre precords loadStaged_device_cmd command, use in junction with informDeviceLoad().
	void preRecord_loadStaged_device(const VkCommandBuffer &cmdBuffer, const rvkBufferImageCopyInfo &bici, 
	VkImageLayout srcImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		VkImageLayout dstImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
	
	/* use with care(experimental).
	 * informs of attempt of the device to use the image.
	 * return true if the caller must submit a transfer command. 
	 */
	bool informDeviceLoad() noexcept;
	
	// for updating samplers.
	const auto &getImgHandle();
	
	const rvkImage &getRvkImage();
	
private:
	
	void _init();
	
	void _recordStagingCmd(const VkCommandBuffer &cmdBuffer, const rvkBufferImageCopyInfo &bici, 
	VkImageLayout srcImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		VkImageLayout dstImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
	
	std::mutex _gm; // rvkBuffer_basic and rvkImage calls are not thread safe.
	rvkBuffer_basic<> _coherent_buff;
	VkEvent _coherentCopyIsDone = VK_NULL_HANDLE; 
	bool _deviceLoadNotCalled = false;
	bool _initialHostLoad = true;
	
};

// currently unused.	
/*struct rvkDrawIndirectCommand {
uint32_t    vertexCount;
uint32_t    instanceCount;
uint32_t    firstVertex;
uint32_t    firstInstance;
};*/
}
}