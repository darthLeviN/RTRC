
#pragma once
#include "decs/rvulkan_image_pipe_decs.h"
#include "rvulkan_circular_resource.h"
#include "rimage_core.h"
#include "rvulkan_image.h"
//#include <functional>
#include <utility>
namespace rtrc
{
namespace vkl
{
	
template<typename ...Args>
inline rvkImagePipe::_internal_liveRes::_internal_liveRes(const Args &... args)
	: _stgImg(std::make_shared<rvkStagedImage>(args...))
{
}
	
inline rvkImagePipe::rvkImagePipe(const std::shared_ptr<vulkanAppInstance> &appInstance,
	const size_t &width, const size_t &height, const size_t &planeCount,
	const VkFormat &vk_format,
	const rImageFormat &rImgFormat,
	const VkImageTiling &tiling, const VkSharingMode &sharingMode, 
	const VkImageUsageFlags &usageFlags)
	: _appInstance(appInstance), _sampler(make_sampler_shared(appInstance, 
	_internal_sampler_settings(), rImgFormat)),
	_resourceMgr(appInstance, width, height, planeCount, vk_format, tiling, 
	sharingMode, usageFlags) // constructs three images with these arguments.
{
}

inline rvkImagePipe::rvkImagePipe(const std::shared_ptr<vulkanAppInstance> &appInstance,
	const rvkColorPlaneLayout_t &cpl, const rvkFormatInterface &rvkFI,
	const rImageFormat &rImgFormat,
	const VkImageTiling &tiling, const VkSharingMode &sharingMode, 
	const VkImageUsageFlags &usageFlags)
	: rvkImagePipe(appInstance, cpl._width, cpl._height, cpl._planeCount,
	rvkFI._vkformat, rImgFormat, tiling, sharingMode, usageFlags)
{

}

// easiest to use constructor, with only a appInstance and a image sample.
inline rvkImagePipe::rvkImagePipe(const std::shared_ptr<vulkanAppInstance> &appInstance,
	const rimagel::rImage &sample, const VkImageTiling &tiling, 
	const VkSharingMode &sharingMode, 
	const VkImageUsageFlags &usageFlags)
	: rvkImagePipe(appInstance, sample, getRImageToVkFI(sample._format),
	tiling, sharingMode, usageFlags)
{
	auto loaderCallable = [&sample,this](void *dstBuff)
	{
		const rvkFormatInterface &rvkFI = getRImageToVkFI(sample._format);
		auto cpl = rvkFI._getPlaneProc(sample);
		// this only needs to be done once. modify this procedure in future.
		this->_defaultBICI = planeLayoutToBICI(cpl);
		cpl.copyTo(dstBuff);
	};

	auto resWriteCallable = [this, loaderCallable](liveRes_t &resource)
	{
		//auto bici = planeLayoutToBICI(cpl); // rvkBufferImageCopyInfo
		// create image view if it's not created(not the most efficient approach but meh.)
		if(!resource._imgV)
		{
			resource._imgV = std::make_shared<imgView_t>(this->_sampler,
				resource._stgImg);
		}
		resource._stgImg->loadStaged_host(loaderCallable);
	};
	_resourceMgr.addResource_initial(resWriteCallable);
}

/* loads the image into host coherent memory.
 * 
 * notes :
 * don't call inside perPassBuffer, it causes deadlock
 * includes a mutex lock that prevents new render pulls from initiation.
 */
template<typename repT, typename periodT, typename grPullerT>
inline void rvkImagePipe::loadImage_host(const rimagel::rImage &rimg
	, const grPullerT &grPuller
	, const std::chrono::duration<repT,periodT> &timeoutDuration)
{
	auto loaderCallable = [&rimg,this](void *dstBuff)
	{
		const rvkFormatInterface &rvkFI = getRImageToVkFI(rimg._format);
		auto cpl = rvkFI._getPlaneProc(rimg);
		// this only needs to be done once. modify this procedure in future.
		this->_defaultBICI = planeLayoutToBICI(cpl);
		cpl.copyTo(dstBuff);
	};
	auto resWriteCallable = [this, loaderCallable](liveRes_t &resource)
	{
		//auto bici = planeLayoutToBICI(cpl); // rvkBufferImageCopyInfo
		// create image view if it's not created(not the most efficient approach but meh.)
		if(!resource._imgV)
		{
			resource._imgV = std::make_shared<imgView_t>(this->_sampler,
				resource._stgImg);
		}
		resource._stgImg->loadStaged_host(loaderCallable);
	};

	auto resOverrideCallable = [this, loaderCallable](liveRes_t &resource) -> bool
	{
		// create image view if it's not created(not the most efficient approach but meh.)
		if(!resource._imgV)
		{
			resource._imgV = std::make_shared<imgView_t>(this->_sampler,
				resource._stgImg);
		}
		return resource._stgImg->try_overwrite_loadStaged_host(loaderCallable);
	};
	_resourceMgr.addResource(grPuller, timeoutDuration, resOverrideCallable, resWriteCallable);
}

// uses the last valid image data
// may throw.
// 
/* dev note : multiple calls to this for multiple image pipelines will create 
 * multiple calls to vkUpdateDescriptorSets, find a way to group them together 
 * if needed.
 */
inline void rvkImagePipe::loadImage_device_cmd(const VkCommandBuffer &cmdBuffer, 
const VkDescriptorSet &dstSet, const uint32_t &dstBinding, const uint32_t &dstArrayElement,
const VkImageLayout &srcImageLayout,
const VkPipelineStageFlags &srcStage,
const VkImageLayout &dstImageLayout,
const VkPipelineStageFlags &dstStage)
{
	const auto &hLogicalDevice = _appInstance->getLogicalDevice();
	const auto &bici = _defaultBICI;
	VkWriteDescriptorSet writeSet{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
	writeSet.dstSet = dstSet;
	writeSet.dstBinding = dstBinding;
	writeSet.dstArrayElement = dstArrayElement;
	writeSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	writeSet.descriptorCount = 1;
	VkDescriptorImageInfo imgInfo{};
	
	imgInfo.sampler = _sampler->isImmutable() ? VK_NULL_HANDLE : _sampler->getVkSampler(); // ignored if sampler is immutable.
	imgInfo.imageLayout = dstImageLayout;
	writeSet.pImageInfo = &imgInfo;
	auto resCallable = [&](liveRes_t &resource)
	{
		imgInfo.imageView = resource._imgV->getVkImageView();
		vkUpdateDescriptorSets(hLogicalDevice, 1, &writeSet, 0, nullptr);
		resource._stgImg->loadStaged_device_cmd(
			cmdBuffer, bici, srcImageLayout, srcStage, dstImageLayout, dstStage
			);
	};
	_resourceMgr.callOnLastResource(resCallable); // may throw.
}



// used to initialize descriptors with immutable samplers.
inline const auto &rvkImagePipe::getSampler() const noexcept
{
	return _sampler;
}

inline rvkImagePipe::rvkImagePipe(const std::shared_ptr<vulkanAppInstance> &appInstance,
	const rimagel::rImage &sample, const rvkFormatInterface &rvkFI, 
	const VkImageTiling &tiling, const VkSharingMode &sharingMode, 
	const VkImageUsageFlags &usageFlags)
	: rvkImagePipe(appInstance, rvkFI._getPlaneProc(sample), rvkFI,
		sample._format, tiling, sharingMode, usageFlags)
{

}

inline constexpr rvkSamplerSettings rvkImagePipe::_internal_sampler_settings()
{
	rvkSamplerSettings settings{};
	settings._info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	settings._info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	settings._info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	settings._info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	settings._info.magFilter = VK_FILTER_LINEAR;
	settings._info.minFilter = VK_FILTER_LINEAR;
	settings._info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;

	return settings;
}
		

		
}
}