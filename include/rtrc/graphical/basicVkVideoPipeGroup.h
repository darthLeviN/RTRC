
#pragma once

//#include "basicVkVideoPipeGroup_decs.h"
#include "decs/basicVkVideoPipeGroup_decs.h"
#include "decoding.h"
#include "decs/rviewPort_core_decs.h"
#include "decs/decoding_decs.h"
#include "../fileUtility.h"

namespace rtrc
{
	
namespace vkPipes
{
	

template<typename appInstanceT>
inline basicVkVideoPipeGroup<appInstanceT>::perPassBuffer::perPassBuffer(const std::shared_ptr<appInstance_t> &appInstance,
const std::vector<VkDescriptorSet> &descSets)
	: _appInstance(appInstance), _descSets(descSets),
	_ubuffer(_appInstance, sizeof(ubStruct), 
	VK_SHARING_MODE_EXCLUSIVE, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
	VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT),
	_vbuffer(_appInstance, sizeof(ubStruct)*10,
	VK_SHARING_MODE_EXCLUSIVE, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT),
	_cmdPool(std::make_shared<rtrc::vkl::rvkCmdPool>(_appInstance, 
	_appInstance->getGraphicsQueueIndex(),
	VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT)),
	_cmdBuffers(_appInstance, _cmdPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1)

{
	ubStruct ms { glm::identity<glm::mat4x4>() };
	_ubuffer.map();
	_vbuffer.map();
	memcpy(_ubuffer._pData, &ms, sizeof(ms));
	std::vector<ubStruct> vps(10, ms);
	memcpy(_vbuffer._pData, vps.data(), sizeof(ms)*10);
}

template<typename appInstanceT>
inline std::vector<VkSubmitInfo> basicVkVideoPipeGroup<appInstanceT>::perPassBuffer::cmdWrite(
	const writeCmd_t &wcmd, uint64_t renderIndex,
	const std::shared_ptr<rtrc::vkl::rvkFence> &prevFence,
	const std::shared_ptr<rtrc::vkl::rvkFence> &curFence,
	std::vector<VkSemaphore> &reservedSems, 
	std::vector<VkPipelineStageFlags> &renderWaitFlags, std::vector<VkSemaphore> &renderWaitSems)
{
	using namespace rtrc::vkl;
	
	if(!wcmd.viewPort)
		throw RbasicException("empty/invalid viewPort in perPassBuffer::cmdWrite");
	wcmd.viewPort->addViewData(renderIndex); // calling this is only needed in once per viewPort in the perPassBuffers of each graphicsPull call. a argument to control this has not been implemented
	
	auto vd = wcmd.viewPort->getLatestSubmittedViewData();
	

	instanceData  vps[2];
	vps[0].m = glm::identity<glm::mat4x4>();
	vps[1].m = glm::translate(glm::vec3(0.0f,0.0f,1.0f))*vps[0].m;

	memcpy(_ubuffer._pData, &vd.p_x_v, sizeof(vd.p_x_v));
	memcpy(_vbuffer._pData, vps, 2*sizeof(instanceData));

	_pSignalSemaphoresVec.clear();
	_pSignalSemaphoresVec.push_back(renderWaitSems.emplace_back(rvkCreateSemaphore(_appInstance)));
	std::vector<VkSubmitInfo> sis(1,{ VK_STRUCTURE_TYPE_SUBMIT_INFO });
	auto &si = sis.back();
	si.signalSemaphoreCount = 1;
	si.pSignalSemaphores = _pSignalSemaphoresVec.data();
	si.pCommandBuffers = _cmdBuffers._cmdBuffers.data();
	si.commandBufferCount = 1;

	_cmdBuffers.beginI(0, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	try
	{
		if(wcmd.videoImgPipe)
			printf("fuck0\n");
			wcmd.videoImgPipe->loadImage_device_cmd(_cmdBuffers._cmdBuffers.back(), _descSets[0], 1 , 0);
			printf("fuck1\n");
	}
	catch(rtrc::RbasicException &err)
	{
		mainLogger.logText2(Rsnprintf<char,100>("transfer cmd recording failed %s", err.what()).data);
	}
	_cmdBuffers.endI(0);


	renderWaitFlags.push_back( VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
	mainLogger.logMutexLock("unlocking mainVPM");
	return sis;
}

template<typename appInstanceT>
inline void basicVkVideoPipeGroup<appInstanceT>::perPassBuffer::mapDescSetWrite(
	rtrc::vkl::rvkWriteDescriptorSet &descSetWrite) const
{
	switch(descSetWrite._winfo.descriptorType)
	{
		case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER :
		{
			if(descSetWrite._winfo.descriptorCount != 1) 
				throw rtrc::graphicalError("invalid values in VkWriteDescriptorSet");
			if(descSetWrite._bufferInfos[0].buffer)
				throw rtrc::graphicalError("invalid values in VkWriteDescriptorSet");

			descSetWrite._bufferInfos[0].buffer = _ubuffer._hBuffer;
			//for(size_t i = 0; i < descSetWrite.descriptorCount; ++i)
			//{

			//}
			break;
		}
		case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
			break; // do nothing. real values are supplied.
		default :
			throw rtrc::graphicalError("bad VkDescriptorType in VkWriteDescriptorSet");
	}

}

template<typename appInstanceT>
inline rtrc::vkl::rvkPerPassVertexBindInfo basicVkVideoPipeGroup<appInstanceT>::perPassBuffer::getVertexBuffers() const noexcept
{
	return { {_vbuffer._hBuffer}, { 0 } };
}

	
template<typename appInstanceT>
inline basicVkVideoPipeGroup<appInstanceT>::vkVPDLoader::vkVPDLoader(
	const std::shared_ptr<appInstance_t> &appInstance)
	: _hostvb(appInstance, sizeof(videoPipeVertices), VK_SHARING_MODE_EXCLUSIVE, 
	VK_BUFFER_USAGE_TRANSFER_SRC_BIT , 
	VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT),

	_hostib(appInstance, sizeof(videoPipeVIndices), VK_SHARING_MODE_EXCLUSIVE,
	VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
	VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT),

	_cmdPool(std::make_shared<rtrc::vkl::rvkCmdPool>(appInstance, appInstance->getGraphicsQueueIndex())),
	_cmdBuffer(appInstance, _cmdPool, 
	VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1)
{
	using appInstance_t = std::shared_ptr<appInstanceT>;
	_hostvb.map();
	memcpy(_hostvb._pData, videoPipeVertices, sizeof(videoPipeVertices));
	_hostvb.unmap();
	_hostib.map();
	memcpy(_hostib._pData, videoPipeVIndices, sizeof(videoPipeVIndices));
	_hostib.unmap();
}

template<typename appInstanceT>
template<typename grPullerT>
inline void basicVkVideoPipeGroup<appInstanceT>::vkVPDLoader::issueLoad(
	const std::shared_ptr<rtrc::vkl::rvkBuffer> &dstvb, const std::shared_ptr<rtrc::vkl::rvkBuffer> &dstib,
	const std::shared_ptr<grPullerT> grPuller)
{
	_cmdBuffer.beginI(0,VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	VkCommandBuffer cmd = _cmdBuffer._cmdBuffers[0];

	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = _hostvb._byteSize;

	vkCmdCopyBuffer( cmd, _hostvb._hBuffer, dstvb->_hBuffer, 1, &copyRegion);
	copyRegion.size = _hostib._byteSize;
	vkCmdCopyBuffer( cmd, _hostib._hBuffer, dstib->_hBuffer, 1, &copyRegion);
	_cmdBuffer.endI(0);

	grPuller->queuePreRenderCmd(0, {{cmd}},{},{});
}

template<typename appInstanceT>
inline auto basicVkVideoPipeGroup<appInstanceT>::_createVideoImgPipe(
	const std::shared_ptr<appInstance_t> &appInstance,
	const std::shared_ptr<rtrc::ravl::ravDecodedIFContext> &videoDecoder)
{
	using namespace rtrc::vkl;
	using namespace rtrc::ravl;
	bool initVideoImgPipeSucceeded = false;
	std::shared_ptr<rtrc::vkl::rvkImagePipe> imgPipe;
	auto firstDecodel = [&](AVFrame *nextfr)
	{
		ravPicture pic(nextfr);
		if(!imgPipe)
		{
			imgPipe = std::make_shared<rvkImagePipe>(
			appInstance, pic);
			mainLogger.logText(Rsnprintf<char, 100>("image view detected with the pixel format of %d\n", nextfr->format).data);
		}
		else
		{
			printf("undefined behaviour while initializing imgPipe, trying to recreate imgPipe\n");
			mainLogger.logText("undefined behaviour while initializing imgPipe, trying to recreate imgPipe");
			exit(-1);
		}
		initVideoImgPipeSucceeded = true;
	};
	
	while(!initVideoImgPipeSucceeded)
	{
		try
		{
			videoDecoder->readAndDecode(firstDecodel);
		}
		catch(rtrc::RbasicException &err)
		{
			mainLogger.logText2(Rsnprintf<char,100>("decoding failed with message \"%s\" trying again...", err.what()).data);
		}
	}
	
	return imgPipe;
}

template<typename appInstanceT>
inline auto basicVkVideoPipeGroup<appInstanceT>::_createGrPipe1Settings(
	const std::shared_ptr<rtrc::vkl::rvkImagePipe> &imgPipe,
	const std::shared_ptr<rtrc::vkl::rvkBuffer> &vb, 
	const std::shared_ptr<rtrc::vkl::rvkBuffer> &ib)
{
	using namespace rtrc::vkl;
	
	
	// bindings
	std::vector<VkVertexInputBindingDescription> bindingDescs(2);
	bindingDescs[0].binding = 0;
	bindingDescs[0].stride = sizeof(vertex2D);
	bindingDescs[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	
	bindingDescs[1].binding = 1;
	bindingDescs[1].stride = sizeof(instanceData);
	bindingDescs[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
	
	// attributes
	std::vector<VkVertexInputAttributeDescription> attrDescs(6);
	// vPos description
	attrDescs[0].location = 0;
	attrDescs[0].binding = 0;
	attrDescs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attrDescs[0].offset = offsetof(vertex2D, x);
	
	// vTexPos description
	attrDescs[1].location = 1;
	attrDescs[1].binding = 0;
	attrDescs[1].format = VK_FORMAT_R32G32_SFLOAT;
	attrDescs[1].offset = offsetof(vertex2D, tx);
	
	// vec4 m_0
	attrDescs[2].location = 2;
	attrDescs[2].binding = 1;
	attrDescs[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attrDescs[2].offset = offsetof(instanceData, instanceData::m) + 0*sizeof(glm::mat4x4::col_type);
	
	// vec4 m_1
	attrDescs[3].location = 3;
	attrDescs[3].binding = 1;
	attrDescs[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attrDescs[3].offset = offsetof(instanceData, instanceData::m) + 1*sizeof(glm::mat4x4::col_type);
	
	// vec4 m_2
	attrDescs[4].location = 4;
	attrDescs[4].binding = 1;
	attrDescs[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attrDescs[4].offset = offsetof(instanceData, instanceData::m) + 2*sizeof(glm::mat4x4::col_type);
	
	// vec4 m_3
	attrDescs[5].location = 5;
	attrDescs[5].binding = 1;
	attrDescs[5].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attrDescs[5].offset = offsetof(instanceData, instanceData::m) + 3*sizeof(glm::mat4x4::col_type);;
	
	// VkDescriptorSetLayoutbinding describes the initial descriptor information.
	std::vector<std::vector<VkDescriptorSetLayoutBinding>> descSetLBs{
		//std::vector<VkDescriptorSetLayoutBinding>(1)
		std::vector<VkDescriptorSetLayoutBinding>(2)
	};
	descSetLBs[0][0].binding = 0;
	descSetLBs[0][0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descSetLBs[0][0].descriptorCount = 1;
	descSetLBs[0][0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	
	// warning, using a local pointer for pImmutableSamplers will result in a 
	// dangling reference
	const VkSampler &samplers = imgPipe->getSampler()->getVkSampler();
	descSetLBs[0][1].binding = 1;
	descSetLBs[0][1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descSetLBs[0][1].descriptorCount = 1;
	descSetLBs[0][1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	descSetLBs[0][1].pImmutableSamplers = &samplers;
	
	// buffer infos
	std::vector<VkDescriptorBufferInfo> descBufferInfos(1);
	descBufferInfos[0].buffer = 0;
	descBufferInfos[0].offset = 0;
	descBufferInfos[0].range = sizeof(ubStruct);
	
	// image infos
	/*std::vector<VkDescriptorImageInfo> descImageInfos(1);
	descImageInfos[0].sampler = VK_NULL_HANDLE; // ignored, immutable sampler
	descImageInfos[0].imageView = img_smp_imgV->getVkImageView();
	descImageInfos[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;*/
	
	// write sets specify the content of uniforms.
	std::vector<rvkWriteDescriptorSet> writeSet;
	writeSet.emplace_back(std::move(descBufferInfos));
	writeSet[0]._winfo.dstSet = 0; // outer index in descSetLBs for the first descriptor set
	writeSet[0]._winfo.dstBinding = 0;
	writeSet[0]._winfo.dstArrayElement = 0;
	writeSet[0]._winfo.descriptorCount = 1;
	writeSet[0]._winfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	
	/*writeSet.emplace_back(std::move(descImageInfos));
	writeSet[1]._winfo.dstSet = 0;
	writeSet[1]._winfo.dstBinding = 1;
	writeSet[1]._winfo.dstArrayElement = 0;
	writeSet[1]._winfo.descriptorCount = 1;
	writeSet[1]._winfo.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;*/
	
	return settings_t(
	std::move(bindingDescs),std::move(attrDescs), std::move(descSetLBs), 
	rvkDescSameUpdater(std::move(writeSet)), {imgPipe,vb,ib});
}

template<typename appInstanceT>
inline auto basicVkVideoPipeGroup<appInstanceT>::_createGrPipeGroup(
	const std::shared_ptr<appInstance_t> &appInstance,
	const std::shared_ptr<rtrc::vkl::rvkImagePipe> &imgPipe,
	const std::shared_ptr<rtrc::vkl::rvkBuffer> &vb, 
	const std::shared_ptr<rtrc::vkl::rvkBuffer> &ib)
{
	using namespace rtrc::vkl;
	auto pipe1Settings = _createGrPipe1Settings(imgPipe, vb, ib);
	
	auto frShader =  std::make_shared<rvkShaderModule<VK_SHADER_STAGE_FRAGMENT_BIT>>(appInstance, readFile(basic_shader_01_fsh_path));
	auto vrShader = std::make_shared<rvkShaderModule<VK_SHADER_STAGE_VERTEX_BIT>>(appInstance, readFile(basic_shader_01_vsh_path));
	
	return std::make_shared<rvkPipeGroup_t>(appInstance, std::move(pipe1Settings), vrShader, frShader);
}

template<typename appInstanceT>
template<typename grPullerT>
inline auto basicVkVideoPipeGroup<appInstanceT>::_createVideoReadThread(
	const std::shared_ptr<grPullerT> &grPuller)
{
	using namespace rtrc::vkl;
	using namespace rtrc::ravl;
	using namespace rtrc;
	using namespace std;
	
	
	auto videoReadLambda = [this](std::shared_ptr<grPullerT> gPuller)
	{
		auto decodel = [&](AVFrame *nextfr)
		{
			auto htStTime = std::chrono::high_resolution_clock::now();
			ravPicture pic(nextfr);
			std::lock_guard<std::mutex> lk(_imgPipeM);
			if(!this->_imgPipe)
			{
				printf("something went wrong, the reference to videoImgPipe is not valid while decoding\n");
				mainLogger.logText("something went wrong, the reference to videoImgPipe is not valid while decoding");
				exit(-1);
			}
			this->_imgPipe->loadImage_host(pic, *gPuller.get(), chrono::seconds(10));
			size_t htTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - htStTime).count();
			mainLogger.logPerformance(Rsnprintf<char,100>("host transfer time of the decoding proc was %llums", htTime));
		};
		bool con{};
		{
			std::lock_guard<std::mutex> lk(this->_continueReadingVideoM);
			con = _continueReadingVideo;
		}
		while(con)
		{
			try
			{
				{
					// access to decoder is implicitly synchronized
					this->_videoDecoder->readAndDecode(decodel);
				}
			}
			catch(rtrc::RbasicException &err)
			{
				printf("decod failed\n");
				printf("err : %s", err.what());
				std::this_thread::sleep_for(std::chrono::microseconds(300));
			}
			std::lock_guard<std::mutex> lk(this->_continueReadingVideoM);
			con = _continueReadingVideo;
		}
	};
	
	return std::thread(videoReadLambda, grPuller);
}

template<typename appInstanceT>
inline basicVkVideoPipeGroup<appInstanceT>::basicVkVideoPipeGroup(
	const std::shared_ptr<appInstance_t> &appInstance,
	const char* feed_path, AVInputFormat* demuxer_format)
	: //setting references
		_appInstance(appInstance), 
		//constructing buffers
		_vb(_make_vertex_buffer(appInstance)), _ib(_make_index_buffer(appInstance)),
		// constructing video decoder and image pipeline
		_videoDecoder(media::create_decoder(feed_path, demuxer_format)),
		_imgPipe(_createVideoImgPipe(appInstance, _videoDecoder)),
		_pipeGroup(_createGrPipeGroup(appInstance, _imgPipe, _vb, _ib)),
		_dataLoader(appInstance)
{
	
}

template<typename appInstanceT>
inline void basicVkVideoPipeGroup<appInstanceT>::drawPull(VkCommandBuffer cmdBuffer, 
	const perPassBuffer_t& perPassBuffer)
{
	using namespace rtrc::vkl;
	
	{
		std::lock_guard<std::mutex> lk(_dataLoadIssuedM);
		if(!_dataLoadIssued)
			throw invalidCallState("called basicVkVideoPipeGroup::drawPull without calling issueLoad beforehand");
	}
	
	auto perPassVBI = perPassBuffer.getVertexBuffers();
	//vb.cmdBind(cmdBuffer, 0);
	perPassVBI.buffers.insert(perPassVBI.buffers.begin(), _vb->_hBuffer);
	perPassVBI.offsets.insert(perPassVBI.offsets.begin(), 0);
	//VkDeviceSize offset = 0;
	//vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &vb._hBuffer, &offset);
	vkCmdBindVertexBuffers(cmdBuffer, 0, perPassVBI.buffers.size(), perPassVBI.buffers.data(), perPassVBI.offsets.data());
	vkCmdBindIndexBuffer(cmdBuffer, _ib->_hBuffer, 0, VK_INDEX_TYPE_UINT16);
	
	vkCmdDrawIndexed(cmdBuffer, videoPipeIndexCount, 2, 0, 0, 0);
	vkCmdEndRenderPass(cmdBuffer);
}

template<typename appInstanceT>
template<typename grPullerT>
inline void basicVkVideoPipeGroup<appInstanceT>::issueLoad(const std::shared_ptr<grPullerT> &grPuller)
{
	std::lock_guard<std::mutex> lk(_dataLoadIssuedM);
	{
		if(_dataLoadIssued) throw invalidCallState("in the current implementation, calling issueLoad more than once is not allowed");
		_dataLoader.issueLoad(_vb, _ib, grPuller);
	}
}

template<typename appInstanceT>
template<typename grPullerT>
inline void basicVkVideoPipeGroup<appInstanceT>::startReadingVideo(const std::shared_ptr<grPullerT> &grPuller)
{
	std::lock_guard<std::mutex> tlk(_videoReadThreadM);
	if(_videoReadThread.joinable())
	{
		throw invalidCallState("video reading has already started");
	}
	_videoReadThread = _createVideoReadThread(grPuller);
}

template<typename appInstanceT>
inline void basicVkVideoPipeGroup<appInstanceT>::stopReadingVideo()
{
	if(_videoReadThread.joinable())
	{
		{
			std::lock_guard<std::mutex> clk(_continueReadingVideoM);
			_continueReadingVideo = false;
		}
		_videoReadThread.join();
	}
}

template<typename appInstanceT>
inline auto basicVkVideoPipeGroup<appInstanceT>::getDrawCallable()
{
	return [this](VkCommandBuffer cmdBuffer, const perPassBuffer_t &perPassBuffer, uint32_t imageIndex)
	{
		drawPull(cmdBuffer, perPassBuffer);
	};
}

template<typename appInstanceT>
inline auto basicVkVideoPipeGroup<appInstanceT>::getWriteCmd(const std::shared_ptr<rtrc::layoutl::rviewPort_core> &viewPort)
{
	using writeCmd_t = typename perPassBuffer_t::writeCmd_t;
	return writeCmd_t{_imgPipe, viewPort};
}

template<typename appInstanceT>
inline auto basicVkVideoPipeGroup<appInstanceT>::getPipeGroup()
{
	return _pipeGroup;
}

template<typename appInstanceT>
inline basicVkVideoPipeGroup<appInstanceT>::~basicVkVideoPipeGroup()
{
	stopReadingVideo();
}

template<typename appInstanceT>
inline auto basicVkVideoPipeGroup<appInstanceT>::_make_vertex_buffer(const std::shared_ptr<appInstance_t> &appInstance)
{
	using namespace rtrc::vkl;
	return std::make_shared<rvkBuffer>(appInstance, sizeof(videoPipeVertices), VK_SHARING_MODE_EXCLUSIVE, 
				VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
}

template<typename appInstanceT>
inline auto basicVkVideoPipeGroup<appInstanceT>::_make_index_buffer(const std::shared_ptr<appInstance_t> &appInstance)
{
	using namespace rtrc::vkl;
	return std::make_shared<rvkBuffer>(appInstance, sizeof(videoPipeVIndices), VK_SHARING_MODE_EXCLUSIVE,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
}

}

}