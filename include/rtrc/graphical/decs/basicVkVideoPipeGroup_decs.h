
#pragma once

//#include "main_utility.h"
#include "../ravlib.h"
#include "../rvulkan.h"
#include "../../rstring.h"
#include "../../rtrc.h"

#include <memory>

namespace rtrc
{
namespace vkPipes
{
	
static constexpr char basic_shader_01_vsh_path[] = SHADERS_FOLDER_PREFIX "vsh.spv";
static constexpr char basic_shader_01_fsh_path[] = SHADERS_FOLDER_PREFIX "fsh.spv";
// implement extern variables for shaders in future to prevent copies.

/* how to use :
 * 1.construct
 * 2.construct your graphics puller based on it and other pipe groups.
 * 3or4.call startReadingVideo to initiate the video reading thread.
 * 3or4.call issueLoad to request the graphics puller.
 * 5. use getDrawCallable(represents the draw pull callable) and getWriteCmd(represents the perPassBuffer input) for your graphics pull 
 * 6. destruct when you are done.
 */
template<typename appInstanceT>
struct basicVkVideoPipeGroup
{
	using appInstance_t = appInstanceT;
	
	// vertex data
	struct vertex2D
	{
		float x, y, z;
		float tx, ty;
	};
	static constexpr size_t videoPipeVertexCount = 4;
	static constexpr size_t videoPipeIndexCount = 6;
	static constexpr vertex2D videoPipeVertices[videoPipeVertexCount] =
	{
		{-0.9f, -0.9f, -0.0f, 0.0f, 1.0f},
		{0.9f, -0.9f, -0.0f, 1.0f, 1.0f},
		{0.9f, 0.9f, -0.0f, 1.0f, 0.0f},
		{-0.9f, 0.9f, -0.0f, 0.0f, 0.0f},
	};
	static constexpr uint16_t videoPipeVIndices[videoPipeIndexCount] =
	{
		2,1,0, 0,3,2
	};
	
	// per-instance data
	struct instanceData
	{
		glm::mat4x4 m; // model matrix
	};
	
	struct ubStruct
	{
		glm::mat4x4 vp;
	};
	
	/*struct mats
	{
		glm::mat4x4 vp;
	};*/
	
	struct perPassBuffer
	{
		static constexpr size_t dSynchI = 0;

		struct writeCmd_t
		{
			std::shared_ptr<rtrc::vkl::rvkImagePipe> videoImgPipe;
			std::shared_ptr<rtrc::layoutl::rviewPort_core> viewPort;
		};
		perPassBuffer(const perPassBuffer &) = delete;
		perPassBuffer(perPassBuffer &&) = delete;
		perPassBuffer &operator=(const perPassBuffer &) = delete;

		perPassBuffer(const std::shared_ptr<appInstance_t> &appInstance,
		const std::vector<VkDescriptorSet> &descSets);

		std::vector<VkSubmitInfo> cmdWrite(const writeCmd_t &wcmd, uint64_t renderIndex,
		const std::shared_ptr<rtrc::vkl::rvkFence> &prevFence,
		const std::shared_ptr<rtrc::vkl::rvkFence> &curFence,
		std::vector<VkSemaphore> &reservedSems, 
		std::vector<VkPipelineStageFlags> &renderWaitFlags, std::vector<VkSemaphore> &renderWaitSems);

		void mapDescSetWrite(rtrc::vkl::rvkWriteDescriptorSet &descSetWrite) const;

		rtrc::vkl::rvkPerPassVertexBindInfo getVertexBuffers() const noexcept;


	private:
		std::shared_ptr<appInstance_t> _appInstance;
		std::vector<VkDescriptorSet> _descSets;
		std::vector<VkSemaphore> _pSignalSemaphoresVec;
		rtrc::vkl::rvkBuffer _ubuffer;
		rtrc::vkl::rvkBuffer _vbuffer;

		// sampler transfers

		/* updated on each cmdWrite call and then used by main render draw command
		 */
		std::vector<VkEvent> transferIsDoneEvents; 

		// dev note : design command pool and buffers that don't dublicate _appInstance,
		std::shared_ptr<rtrc::vkl::rvkCmdPool> _cmdPool;
		rtrc::vkl::rvkCmdBuffers _cmdBuffers;

		// no multi threading for this. or things get messed up.
		//static glm::mat4x4 prevView;
		//static boost::circular_buffer<rtrc::indexedData<glm::mat4x4>> prevViews;

	};
	
	//
	// main data locader for video loading.
	//
	struct vkVPDLoader
	{

		vkVPDLoader() = delete;
		vkVPDLoader(const vkVPDLoader &) = delete;
		vkVPDLoader(vkVPDLoader &&) = delete;

		vkVPDLoader(const std::shared_ptr<appInstance_t> &appInstance);

		template<typename grPullerT>
		void issueLoad(const std::shared_ptr<rtrc::vkl::rvkBuffer> &dstvb, const std::shared_ptr<rtrc::vkl::rvkBuffer> &dstib,
		const std::shared_ptr<grPullerT> grPuller);

	private:

		rtrc::vkl::rvkBuffer _hostvb;
		rtrc::vkl::rvkBuffer _hostib;
		std::shared_ptr<rtrc::vkl::rvkCmdPool> _cmdPool;
		rtrc::vkl::rvkCmdBuffers _cmdBuffer;
	};
	
	using perPassBuffer_t = perPassBuffer;
	
	using settings_t = rtrc::vkl::commonVkPipeSettings<perPassBuffer_t>;
	
	using rvkPipeGroup_t = rtrc::vkl::rvkGrPipeGroup<settings_t, appInstance_t>;
	
	
	
	basicVkVideoPipeGroup(const std::shared_ptr<appInstance_t> &appInstance,
	const char *feed_path, AVInputFormat *demuxer_format);
	
	
	
	void drawPull(VkCommandBuffer cmdBuffer, const perPassBuffer_t &perPassBuffer/*rvkPerPassVertexBindInfo &perPassVBI*/);
	
	template<typename grPullerT>
	void issueLoad(const std::shared_ptr<grPullerT> &grPuller);
	
	// creates the video reading thread.
	template<typename grPullerT>
	void startReadingVideo(const std::shared_ptr<grPullerT> &grPuller);
	
	// silently returns if the video reading has already been stopped 
	void stopReadingVideo();
	
	// shouldn't be used when object is destroyed. only call when a callable is needed.
	auto getDrawCallable();
	
	// implement a const version of these in future.
	auto getWriteCmd(const std::shared_ptr<rtrc::layoutl::rviewPort_core> &viewPort);
	auto getPipeGroup();
	
	~basicVkVideoPipeGroup();
	
private:
	
	static auto _createVideoImgPipe(const std::shared_ptr<appInstance_t> &appInstance,
		const std::shared_ptr<rtrc::ravl::ravDecodedIFContext> &videoDecoder);
	// call when _imgPipe is initialized;
	static auto _createGrPipe1Settings(const std::shared_ptr<rtrc::vkl::rvkImagePipe> &imgPipe,
	const std::shared_ptr<rtrc::vkl::rvkBuffer> &vb, 
	const std::shared_ptr<rtrc::vkl::rvkBuffer> &ib);
	
	static auto _createGrPipeGroup(
	const std::shared_ptr<appInstance_t> &appInstance,
	const std::shared_ptr<rtrc::vkl::rvkImagePipe> &imgPipe,
	const std::shared_ptr<rtrc::vkl::rvkBuffer> &vb, 
	const std::shared_ptr<rtrc::vkl::rvkBuffer> &ib);
	
	// warning, don't create a second video reading thread before joining the previous one.(a second call to this function is untested)
	template<typename grPullerT>
	auto _createVideoReadThread(const std::shared_ptr<grPullerT> &grPuller);
	
	// constructor utility fucntions
	static auto _make_vertex_buffer(const std::shared_ptr<appInstance_t> &appInstance);
	static auto _make_index_buffer(const std::shared_ptr<appInstance_t> &appInstance);
	//void _make_command_buffer(); currently not implemented
	
	// member variables
	
	std::shared_ptr<appInstance_t> _appInstance;
	
	
	
	//
	// initialization order is important.
	//
	
	// make these buffers extern in future implementations.
	std::shared_ptr<rtrc::vkl::rvkBuffer> _vb; // virtually immutable, no synchronization needed. vertex buffer
	std::shared_ptr<rtrc::vkl::rvkBuffer> _ib; // virtually immutable, no synchornization needed. index buffer
	
	std::shared_ptr<rtrc::ravl::ravDecodedIFContext> _videoDecoder; // video decoder, implicit synchronization because there is only one 
	
	// multi threaded variables
	std::mutex _imgPipeM;
	std::shared_ptr<rtrc::vkl::rvkImagePipe> _imgPipe; // vulkan image pipe for the video.
	std::mutex _videoReadThreadM;
	std::thread _videoReadThread;
	std::mutex _continueReadingVideoM;
	bool _continueReadingVideo = false;
	
	std::shared_ptr<rvkPipeGroup_t> _pipeGroup;
	
	
	vkVPDLoader _dataLoader;
	std::mutex _dataLoadIssuedM;
	bool _dataLoadIssued = false; // change this mechanism with a fence based one in future.
	
	/* mutex locking order (experimental):
	 * internal _viewPort mutexes. 
	 * the rest sames as declaration order.
	 */
};

}

}