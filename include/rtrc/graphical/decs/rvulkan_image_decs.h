/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   rvulkan_image.h
 * Author: roohi
 *
 * Created on September 14, 2019, 4:20 PM
 */
#pragma once
#include "../../rexception.h"
#include "../rvulkan_core.h"
#include "../rimage_core.h"
#include <vector>

// remove this later.
using namespace rtrc::rimagel;


namespace rtrc { namespace vkl {

static inline rvkBufferImageCopyInfo planeLayoutToBICI( const rvkColorPlaneLayout_t &cpl, const uint32_t &mipLevel = 0,
	const uint32_t &baseArrayLayer = 0, const uint32_t &layerCount = 1);

static inline void rvkCmdCopyBufferToImage(VkCommandBuffer cmdBuff, const rvkBufferImageCopyInfo &bici, 
	VkBuffer srcBuff,VkImage dstImage,VkImageLayout dstImageLayout);

static inline rvkColorPlaneLayout_t unimlpemented_getVkPlaneProc(const rimagel::rImage &);


template<size_t planeCount>
static inline rvkColorPlaneLayout_t getVkPlanes_common(const rImage &img);

	
inline constexpr std::array<std::array<rvkFormatInterface,4/*plane count - 1*/>,
	rimg_formats_entry_count> makeRImageToVkFIs();

struct rvkChromaLocation
{
	VkChromaLocation xl;
	VkChromaLocation yl;
};

inline constexpr std::array<rvkChromaLocation, 7> makeRtoRvkChromaLocation();


static inline const rvkFormatInterface &getRImageToVkFI(const rImageFormat &imgfmt);

static inline const rvkChromaLocation &getRvkChromaLocation( rChromaLocation rcl);

static inline VkFormat rImageFmtToVkFmt(const rImageFormat &imgfmt);

/* a VkSamplerCreateInfo struct to set sampler settings. 
 * they will be overridden if needed based on the vulkan documentation.
 * if only visual effects are overridden, the overriding class is responsible to
 * generate a warning.
 * if important values (addressModeX) needs to be overridden, an exception 
 * should be thrown.
 */
struct rvkSamplerSettings
{
	VkSamplerCreateInfo _info;
};

struct rvkSamplerDep
{
	virtual ~rvkSamplerDep();
};

struct rvkSamplerYcbcrConversion : rvkSamplerDep
{
	rvkSamplerYcbcrConversion(const rvkSamplerYcbcrConversion &) = delete;
	rvkSamplerYcbcrConversion &operator=(const rvkSamplerYcbcrConversion &) = delete;
	
	rvkSamplerYcbcrConversion(rvkSamplerYcbcrConversion &&other);
	
	rvkSamplerYcbcrConversion(const std::shared_ptr<vulkanAppInstance> &appInstance,
	const VkSamplerYcbcrConversionCreateInfo &sycCI, VkFormatFeatureFlags enabledFeatures = {});
	
	rvkSamplerYcbcrConversion(const std::shared_ptr<vulkanAppInstance> &appInstance,
	const rImageFormat &rImgFormat, VkFormatFeatureFlags enabledFeatures = {});
	
	static VkSamplerYcbcrConversionCreateInfo createCI(const std::shared_ptr<vulkanAppInstance> &appInstance,
	const rImageFormat &rImgFormat);
	
	~rvkSamplerYcbcrConversion();
	
	bool isCompatibleWith(const rvkImage &img);
	
	
	VkSamplerYcbcrConversion _syc = VK_NULL_HANDLE;
	VkFormat _format = {};
	VkFormatFeatureFlags _formatFeatures = {};
	std::shared_ptr<vulkanAppInstance> _appInstance;
};

struct rvkSamplerYcbcrSettings : rvkSamplerSettings
{
	rvkSamplerYcbcrSettings( const rvkSamplerYcbcrSettings &other);
	
	rvkSamplerYcbcrSettings(rvkSamplerYcbcrSettings && other);
	rvkSamplerYcbcrSettings &operator=(const rvkSamplerYcbcrSettings &other);
	
	rvkSamplerYcbcrSettings (const rvkSamplerSettings &settings,
	const std::shared_ptr<rvkSamplerYcbcrConversion> &_syc);
	VkSamplerYcbcrConversionInfo _samplerYcbcrConvI;
};


/* creates a VkSampler.
 * VkPhysicalDeviceLimits related settings will be overridden in it's constructor.
 */
struct rvkSampler
{
	rvkSampler(const rvkSampler &) = delete;
	rvkSampler(rvkSampler &&) = delete;
	rvkSampler &operator=(const rvkSampler &) = delete;
	
	
	
	typedef std::function<VkImageView(const VkImageViewCreateInfo &, const rvkImage &)> createImgViewProc_t;
	
	// imgVCISample's image, and subresourceRange::aspectMask and format will be overridden.
	// also, structs may be added to pNext chain of the create info struct.
	static constexpr auto defaultCreateColorImgVFunc = [](const VkImageViewCreateInfo &imgVCISample, 
	const rvkImage &img)
	{
		const auto &appInstance = img.getAppInstance();
		auto hLogicalDevice = appInstance->getLogicalDevice();
		auto pAllocator = appInstance->getpAllocator();
		auto imgVCI = imgVCISample;
		VkImageView retImgV = VK_NULL_HANDLE;
		
		imgVCI.image = img._hImage;
		imgVCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imgVCI.format = img.getFormat();
		
		if(vkCreateImageView(hLogicalDevice, &imgVCI, pAllocator, &retImgV) != VK_SUCCESS)
			throw graphicalError("could not create image view");
		
		return retImgV;
	};
	
	// imgVCISample's image, and subresourceRange::aspectMask will be overridden.
	// also, structs may be added to pNext chain of the create info struct.
	createImgViewProc_t _createColorImgVFunc = defaultCreateColorImgVFunc;
	
	template<typename depType = rvkSamplerDep>
	rvkSampler(const std::shared_ptr<vulkanAppInstance> &appInstance, 
	const rvkSamplerSettings &settings, const std::shared_ptr<depType> &samplerDep = std::shared_ptr<rvkSamplerDep>());
	
	virtual ~rvkSampler();
	
	virtual bool isImmutable() { return false; }
	
	const auto &getAppInstance() const noexcept;
	const VkSampler &getVkSampler() const noexcept;
protected:
	std::shared_ptr<vulkanAppInstance> _appInstance;
private:
	VkSampler _sampler = VK_NULL_HANDLE;
	std::shared_ptr<rvkSamplerDep> _samplerDep;
	
};



enum rvkSamplerImgViewType
{
	rvk_color_SamplerIV
};

template<rvkSamplerImgViewType imgVType>
struct rvkSamplerImgView
{
	
	rvkSamplerImgView(const rvkSamplerImgView &) = delete;
	rvkSamplerImgView &operator=(const rvkSamplerImgView &) = delete;
	template<typename imgType>
	rvkSamplerImgView(const std::shared_ptr<rvkSampler> &sampler, 
	const std::shared_ptr<imgType> &img,
	const VkImageViewCreateInfo &imgVCISample);
	
	/*rvkSamplerImgView(rvkSamplerImgView &&other)
		: _sampler(std::move(other._sampler)), _img(std::move(other._img)), _imgView(other._imgView)
	{
		other._imgView = VK_NULL_HANDLE;
	}*/
	
	~rvkSamplerImgView();
	
	
	auto getVkImageView() const noexcept;
	
private:
	std::shared_ptr<rvkSampler> _sampler;
	std::shared_ptr<rvkImage> _img;
	const std::shared_ptr<vulkanAppInstance> &_appInstance = _img->getAppInstance();
	VkImageView _imgView = VK_NULL_HANDLE;
};

struct rvkSamplerImgView_2D_color : rvkSamplerImgView<rvk_color_SamplerIV>
{
	typedef rvkSamplerImgView<rvk_color_SamplerIV> base_t;
	template<typename imgType>
	rvkSamplerImgView_2D_color(const std::shared_ptr<rvkSampler> &sampler, 
	const std::shared_ptr<imgType> &img);
	
private:
	static constexpr VkImageViewCreateInfo _internal_createCI();
};

struct rvkYcbcrSampler : rvkSampler
{
	rvkYcbcrSampler(const rvkYcbcrSampler &) = delete;
	rvkYcbcrSampler &operator=(const rvkYcbcrSampler &) = delete;
	rvkYcbcrSampler(rvkYcbcrSampler &&) = delete;
	
	rvkYcbcrSampler( const std::shared_ptr<vulkanAppInstance> &appInstance, 
	const rvkSamplerSettings &settings,
	const std::shared_ptr<rvkSamplerYcbcrConversion> &syc);
	
	bool isCompatibleWithImg(const rvkImage &img);
	
	virtual bool isImmutable() override { return true; }
	
private:
	static rvkSamplerYcbcrSettings createSettings(
	const std::shared_ptr<vulkanAppInstance> &appInstance,
		const rvkSamplerSettings &settings,
		const std::shared_ptr<rvkSamplerYcbcrConversion> &syc);
	
	
private:
	std::shared_ptr<rvkSamplerYcbcrConversion> _syc; // this is not a good way to store this. it's a duplicate.
	//VkFormatProperties _formatProps;
};


static inline std::shared_ptr<rvkSampler> make_sampler_shared(
const std::shared_ptr<vulkanAppInstance> &appInstance, 
	const rvkSamplerSettings &settings, const rImageFormat &format);

}
}