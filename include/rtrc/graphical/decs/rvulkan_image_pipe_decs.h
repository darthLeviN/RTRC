
#pragma once
#include "../rvulkan_circular_resource.h"
#include "../rimage_core.h"
#include "../rvulkan_image.h"
//#include <functional>
#include <utility>
namespace rtrc
{
namespace vkl
{
	
	struct rvkImagePipe
	{
		typedef rvkSamplerImgView_2D_color imgView_t;
		struct _internal_liveRes
		{
			template<typename ...Args>
			_internal_liveRes(const Args &... args);
			
			std::shared_ptr<rvkStagedImage> _stgImg;
			//std::vector<std::shared_ptr<imgView_t>> _imgV; // maximum size is 3
                        std::shared_ptr<imgView_t> _imgV;
		};
		typedef _internal_liveRes liveRes_t;
		typedef rvkLiveResource<liveRes_t, 3>
			liveResMgr_t;
		
		rvkImagePipe(const rvkImagePipe &) = delete;
		rvkImagePipe(rvkImagePipe &&) = delete;
		rvkImagePipe &operator=(const rvkImagePipe &) = delete;
		
		rvkImagePipe(const std::shared_ptr<vulkanAppInstance> &appInstance,
			const size_t &width, const size_t &height, const size_t &planeCount,
			const VkFormat &vk_format,
			const rImageFormat &rImgFormat,
			const VkImageTiling &tiling, const VkSharingMode &sharingMode, 
			const VkImageUsageFlags &usageFlags);
		
		rvkImagePipe(const std::shared_ptr<vulkanAppInstance> &appInstance,
			const rvkColorPlaneLayout_t &cpl, const rvkFormatInterface &rvkFI,
			const rImageFormat &rImgFormat,
			const VkImageTiling &tiling, const VkSharingMode &sharingMode, 
			const VkImageUsageFlags &usageFlags);
		
		// easiest to use constructor, with only a appInstance and a image sample.
		rvkImagePipe(const std::shared_ptr<vulkanAppInstance> &appInstance,
			const rimagel::rImage &sample, const VkImageTiling &tiling = VK_IMAGE_TILING_LINEAR, 
			const VkSharingMode &sharingMode = VK_SHARING_MODE_EXCLUSIVE, 
			const VkImageUsageFlags &usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT);
		
		// loads the image into host coherent memory.
		template<typename repT, typename periodT, typename grPullerT>
		void loadImage_host(const rimagel::rImage &rimg
			, const grPullerT &grPuller
			, const std::chrono::duration<repT,periodT> &timeoutDuration);
		
		// uses the last valid image data
		// may throw.
		void loadImage_device_cmd(const VkCommandBuffer &cmdBuffer, 
		const VkDescriptorSet &dstSet, const uint32_t &dstBinding, const uint32_t &dstArrayElement,
		const VkImageLayout &srcImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		const VkPipelineStageFlags &srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		const VkImageLayout &dstImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		const VkPipelineStageFlags &dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
		
		
		// used to initialize descriptors with immutable samplers.
		const auto &getSampler() const noexcept;
		
		rvkImagePipe(const std::shared_ptr<vulkanAppInstance> &appInstance,
			const rimagel::rImage &sample, const rvkFormatInterface &rvkFI, 
			const VkImageTiling &tiling, const VkSharingMode &sharingMode, 
			const VkImageUsageFlags &usageFlags);
		
		static constexpr rvkSamplerSettings _internal_sampler_settings();
		
		
		std::shared_ptr<vulkanAppInstance> _appInstance;
		std::shared_ptr<rvkSampler> _sampler;
		liveResMgr_t _resourceMgr;
		rvkBufferImageCopyInfo _defaultBICI{};
	};
	
}
}