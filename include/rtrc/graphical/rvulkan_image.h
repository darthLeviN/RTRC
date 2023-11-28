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
#include "decs/rvulkan_image_decs.h"
#include "../rexception.h"

#include "rvulkan_core.h"

#include "rimage_core.h"
#include <vector>

// remove this later.
using namespace rtrc::rimagel;

namespace rtrc { namespace vkl {

static inline rvkBufferImageCopyInfo planeLayoutToBICI( const rvkColorPlaneLayout_t &cpl, const uint32_t &mipLevel,
	const uint32_t &baseArrayLayer, const uint32_t &layerCount)
{
	rvkBufferImageCopyInfo bici{};
	
	bici._rgCount = cpl._planeCount;
	size_t offset = 0;
	for(size_t i = 0; i < bici._rgCount; ++i)
	{
		auto &region = bici._regions[i];
		region.imageExtent.depth = 1;
		region.imageExtent.height = cpl._planeHeights[i];
		region.imageExtent.width = cpl._planeWidths[i];//cpl._linesizes[i];
		region.bufferRowLength = cpl._planeWidths[i];//cpl._linesizes[i];
		region.bufferImageHeight = cpl._planeHeights[i];
		region.bufferOffset = offset;
		region.imageSubresource.aspectMask = rvkPlaneFlags[i];
		region.imageSubresource.mipLevel = mipLevel;
		region.imageSubresource.baseArrayLayer = baseArrayLayer;
		region.imageSubresource.layerCount = layerCount;
		offset += cpl._linesizes[i]*cpl._planeHeights[i];
	}
	//
	// experimental
	//
	if(bici._rgCount == 1) bici._regions[0].imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	return bici;
}

static inline void rvkCmdCopyBufferToImage(VkCommandBuffer cmdBuff, const rvkBufferImageCopyInfo &bici, 
	VkBuffer srcBuff,VkImage dstImage,VkImageLayout dstImageLayout)
{
	vkCmdCopyBufferToImage(cmdBuff, srcBuff, dstImage, dstImageLayout, bici._rgCount, bici._regions);
}

static inline rvkColorPlaneLayout_t unimlpemented_getVkPlaneProc(const rimagel::rImage &)
{
	throw rImplementationError("unimplemented getVkPlaneProc used");
}


template<size_t planeCount>
static inline rvkColorPlaneLayout_t getVkPlanes_common(const rImage &img)
{
	rvkColorPlaneLayout_t cpl{};
	cpl._planeCount = planeCount;
	//cpl._data = img._data[0];
	cpl._height = img._height;
	cpl._width = img._width;
	for(size_t j = 0; j < planeCount; ++j)
	{
		// i is rImage plane index and j is vulkan plane index.
		size_t i = img._pixComps[j].planeIndex;
		cpl._linesizes[j] = img._linesizes[i];
		cpl._planeHeights[j] = img._planeHeights[i];
		cpl._planeWidths[j] = img._planeWidths[i];
		cpl._rvkPlaneData[j] = img._data[i];
	}
	return cpl;
}

static constexpr getVkPlaneProc_t getVkPlane_420_3P = &getVkPlanes_common<3>;
static constexpr getVkPlaneProc_t getVkPlane_420_2P = &getVkPlanes_common<2>;
static constexpr getVkPlaneProc_t getVkPlane_rgba32_1P = &getVkPlanes_common<1>;
	
inline constexpr std::array<std::array<rvkFormatInterface,4/*plane count - 1*/>,
	rimg_formats_entry_count> makeRImageToVkFIs()
{
	std::array<std::array<rvkFormatInterface,4/*plane count from index 1 : 1 planes to index 3 : 3 planes*/>,
	rimg_formats_entry_count> retArr{};
	for(size_t i = 0; i < rimg_formats_entry_count; ++i)
	{
		for(size_t j = 0; j < 4; ++j)
		{
			retArr[i][j]._vkformat = VK_FORMAT_UNDEFINED;
			retArr[i][j]._rImgPixFormat = rimg_noFormat;
			retArr[i][j]._getPlaneProc = unimlpemented_getVkPlaneProc;
			
		}
	}
	retArr[rimg_yuv420p][3]._vkformat = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM;
	retArr[rimg_yuv420p][3]._getPlaneProc = getVkPlane_420_3P;
	retArr[rimg_yuv420p][3]._rImgPixFormat = rimg_yuv420p;
	
	retArr[rimg_yuv420p_NV12][2]._vkformat = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
	retArr[rimg_yuv420p_NV12][2]._getPlaneProc = getVkPlane_420_2P;
	retArr[rimg_yuv420p_NV12][2]._rImgPixFormat = rimg_yuv420p_NV12;
	
	retArr[rimg_rgba32][1]._vkformat = VK_FORMAT_R8G8B8A8_UNORM;
	retArr[rimg_rgba32][1]._getPlaneProc = getVkPlane_rgba32_1P;
	retArr[rimg_rgba32][1]._rImgPixFormat = rimg_rgba32;
	
	return retArr;
}

inline constexpr std::array<rvkChromaLocation, 7> makeRtoRvkChromaLocation()
{
	std::array<rvkChromaLocation, 7> retArr{};
	retArr[rchroma_loc_center].xl = VK_CHROMA_LOCATION_MIDPOINT;
	retArr[rchroma_loc_center].yl = VK_CHROMA_LOCATION_MIDPOINT;
	
	retArr[rchroma_loc_left].xl = VK_CHROMA_LOCATION_COSITED_EVEN; // yl ignored
	
	retArr[rchroma_loc_topleft].xl = VK_CHROMA_LOCATION_COSITED_EVEN;
	retArr[rchroma_loc_topleft].yl = VK_CHROMA_LOCATION_COSITED_EVEN;
	
	// the rest are unimplemented. but produce no warnings.
	return retArr;
}


static constexpr auto rToRvkChromaLocationArr = makeRtoRvkChromaLocation();
static constexpr auto rImageToVkFIArr = makeRImageToVkFIs();

static inline const rvkFormatInterface &getRImageToVkFI(const rImageFormat &imgfmt)
{	
	return rImageToVkFIArr[imgfmt._pixFmt][imgfmt._planeCount];
}

static inline const rvkChromaLocation &getRvkChromaLocation( rChromaLocation rcl)
{
	return rToRvkChromaLocationArr[rcl];
}

static inline VkFormat rImageFmtToVkFmt(const rImageFormat &imgfmt)
{
	return getRImageToVkFI(imgfmt)._vkformat;
}

inline rvkSamplerDep::~rvkSamplerDep() {}
	
inline rvkSamplerYcbcrConversion::rvkSamplerYcbcrConversion(rvkSamplerYcbcrConversion &&other)
	: _appInstance(std::move(other._appInstance)), _syc(other._syc), 
	_format(other._format), _formatFeatures(other._formatFeatures)
{
	other._syc = VK_NULL_HANDLE;
}

inline rvkSamplerYcbcrConversion::rvkSamplerYcbcrConversion(const std::shared_ptr<vulkanAppInstance> &appInstance,
const VkSamplerYcbcrConversionCreateInfo &sycCI, VkFormatFeatureFlags enabledFeatures)
	: _appInstance(appInstance), _format(sycCI.format), _formatFeatures(enabledFeatures)
{
	auto hLogicalDevice = _appInstance->getLogicalDevice();
	auto pAllocator = _appInstance->getpAllocator();

	if( vkCreateSamplerYcbcrConversion(hLogicalDevice, &sycCI, pAllocator, &_syc) != VK_SUCCESS )
		throw graphicalError("could not create VkSamplerYcbcrConversion");
}

inline rvkSamplerYcbcrConversion::rvkSamplerYcbcrConversion(const std::shared_ptr<vulkanAppInstance> &appInstance,
const rImageFormat &rImgFormat, VkFormatFeatureFlags enabledFeatures)
	: rvkSamplerYcbcrConversion(appInstance, createCI(appInstance, rImgFormat),
	enabledFeatures)
{
}

inline VkSamplerYcbcrConversionCreateInfo rvkSamplerYcbcrConversion::createCI(const std::shared_ptr<vulkanAppInstance> &appInstance,
const rImageFormat &rImgFormat)
{

	VkSamplerYcbcrConversionCreateInfo ci{ VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_CREATE_INFO };

	ci.format = rImageFmtToVkFmt(rImgFormat);
	ci.chromaFilter = VK_FILTER_LINEAR;
	switch(rImgFormat._chromaLoc)
	{
		case rchroma_loc_center:
		{
			ci.xChromaOffset = VK_CHROMA_LOCATION_MIDPOINT;
			ci.yChromaOffset = VK_CHROMA_LOCATION_MIDPOINT;
			break;
		}
		case rchroma_loc_topleft:
		{
			ci.xChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
			ci.yChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
			break;
		}
		case rchroma_loc_top:
		{
			ci.xChromaOffset = VK_CHROMA_LOCATION_MIDPOINT;
			ci.yChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
			break;
		}
		case rchroma_loc_left:
		{
			ci.xChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
			ci.yChromaOffset = VK_CHROMA_LOCATION_MIDPOINT;
			break;
		}
		default:
		{
			ci.xChromaOffset = VK_CHROMA_LOCATION_MIDPOINT;
			ci.yChromaOffset = VK_CHROMA_LOCATION_MIDPOINT;
			appInstance->generateWarning(rvkWarnings_overriddenSettings,
				"unsupported chroma location for image. center was choosen but will result in innacurate luma of image");
		}
	}
	switch(rImgFormat._colorSpace)
	{
		case rColorSpace_601:
		{
			ci.ycbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_601;
			break;
		}
		case rColorSpace_709:
		{
			ci.ycbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_709;
			break;
		}
		case rColorSpace_2020:
		{
			ci.ycbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_2020;
			break;
		}
		default:
		{
			ci.ycbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_601;


			appInstance->generateWarning(rvkWarnings_overriddenSettings,
				"unsupported color space for image. rColorSpace_601 was choosen but will result in innacurate color");
		}
	}
	switch(rImgFormat._colorRange)
	{
		case rYcbcrColorRange_ITU_narrow:
		{
			ci.ycbcrRange = VK_SAMPLER_YCBCR_RANGE_ITU_NARROW;
			break;
		}
		case rYcbcrColorRange_full:
		{
			ci.ycbcrRange = VK_SAMPLER_YCBCR_RANGE_ITU_FULL;
			break;
		}
		default:
		{
			ci.ycbcrRange = VK_SAMPLER_YCBCR_RANGE_ITU_FULL;
			appInstance->generateWarning(rvkWarnings_overriddenSettings,
				"unsupported color space for image. full range was choosen but will result in innacurate color and luma");
		}
	}
	return ci;
}

inline rvkSamplerYcbcrConversion::~rvkSamplerYcbcrConversion()
{
	auto hLogicalDevice = _appInstance->getLogicalDevice();
	auto pAllocator = _appInstance->getpAllocator();

	vkDestroySamplerYcbcrConversion(hLogicalDevice, _syc, pAllocator);
}

inline bool rvkSamplerYcbcrConversion::isCompatibleWith(const rvkImage &img)
{
	if(_format != img.getFormat())
		return false;
	if(_formatFeatures& VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT)
	{
		if(!(img.getFormatFeatures() & VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT))
			return false;
	}
	return true;
}
	
	
inline rvkSamplerYcbcrSettings::rvkSamplerYcbcrSettings( const rvkSamplerYcbcrSettings &other)
	: rvkSamplerSettings(other), _samplerYcbcrConvI(other._samplerYcbcrConvI)
{
	_info.pNext = &_samplerYcbcrConvI;
}

inline rvkSamplerYcbcrSettings::rvkSamplerYcbcrSettings(rvkSamplerYcbcrSettings && other)
	: rvkSamplerSettings(other), _samplerYcbcrConvI{other._samplerYcbcrConvI}
{
	_info.pNext = &_samplerYcbcrConvI;
}
inline rvkSamplerYcbcrSettings &rvkSamplerYcbcrSettings::operator=(const rvkSamplerYcbcrSettings &other)
{
	rvkSamplerSettings::operator=(other);
	_samplerYcbcrConvI = other._samplerYcbcrConvI;
	_info.pNext = &_samplerYcbcrConvI;

	return *this;
}

inline rvkSamplerYcbcrSettings::rvkSamplerYcbcrSettings (const rvkSamplerSettings &settings,
const std::shared_ptr<rvkSamplerYcbcrConversion> &_syc)
	: rvkSamplerSettings(settings), _samplerYcbcrConvI{VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_INFO}
{
	_samplerYcbcrConvI.conversion = _syc->_syc;
	_samplerYcbcrConvI.pNext = settings._info.pNext;
	_info.pNext = &_samplerYcbcrConvI;
}
	
template<typename depType>
inline rvkSampler::rvkSampler(const std::shared_ptr<vulkanAppInstance> &appInstance, 
const rvkSamplerSettings &settings, const std::shared_ptr<depType> &samplerDep)
	: _appInstance(appInstance), _samplerDep(samplerDep)
{
	auto hLogicalDevice = _appInstance->getLogicalDevice();
	auto pAllocator = _appInstance->getpAllocator();
	const rvkPhysicalDeviceFeatures &features = _appInstance->getDeviceFeatures();

	VkSamplerCreateInfo samplerCI = settings._info;

	const VkPhysicalDeviceLimits &limits = _appInstance->getDeviceProperties().limits;

	// applying API's LOD limits
	/* .The absolute value of mipLodBias must be less than or equal to 
	 * VkPhysicalDeviceLimits::maxSamplerLodBias
	 * .maxLod must be greater than or equal to minLod
	 */
	if(samplerCI.maxLod < samplerCI.minLod)
	{
		_appInstance->generateWarning(rvkWarnings_overriddenSettings,
			"sampler maxLod smaller than minLod. the value was overridden");
	}
	if(samplerCI.minLod > limits.maxSamplerLodBias)
	{
		samplerCI.minLod = limits.maxSamplerLodBias;
		_appInstance->generateWarning(rvkWarnings_overriddenSettings,
			"sampler minLod to high. the value was overridden");
	}
	if(samplerCI.maxLod < samplerCI.minLod)
	{
		samplerCI.maxLod = samplerCI.minLod;
	}

	// anisotropic filtering feature
	/* .If the anisotropic sampling feature is not enabled, anisotropyEnable 
	 * must be VK_FALSE
	 * .If anisotropyEnable is VK_TRUE, maxAnisotropy must be between 1.0 
	 * and VkPhysicalDeviceLimits::maxSamplerAnisotropy, inclusive
	 */
	if(samplerCI.anisotropyEnable != VK_FALSE)
	{
		if(features._devFeatures.samplerAnisotropy == VK_FALSE)
		{
			samplerCI.anisotropyEnable = VK_FALSE;
			_appInstance->generateWarning(rvkWarnings_overriddenSettings,
				"anisotropic filtering device feature not enabled,"
				" sampler settings were overridden");
		}
		else
		{
			if(samplerCI.maxAnisotropy < 1.0f)
			{
				samplerCI.maxAnisotropy = 1.0f;
				_appInstance->generateWarning(rvkWarnings_overriddenSettings,
					"invalid maxAnisotropy, the value was overridden");
			}
			else if(samplerCI.maxAnisotropy > limits.maxSamplerAnisotropy)
			{
				samplerCI.maxAnisotropy = limits.maxSamplerAnisotropy;
				_appInstance->generateWarning(rvkWarnings_overriddenSettings,
					"maxAnisotropy higher than device limits,"
					" the value was overridden");
			}
		}
	}


	// the rest of the API's specifications will be implemented in future.



	if(vkCreateSampler(hLogicalDevice, &samplerCI, pAllocator, &_sampler)
		!= VK_SUCCESS)
		throw graphicalError("could not create sampler");
}

inline rvkSampler::~rvkSampler()
{
	auto hLogicalDevice = _appInstance->getLogicalDevice();
	auto pAllocator = _appInstance->getpAllocator();

	vkDestroySampler(hLogicalDevice, _sampler, pAllocator);
}



inline const auto &rvkSampler::getAppInstance() const noexcept { return _appInstance; }
inline const VkSampler &rvkSampler::getVkSampler() const noexcept { return _sampler; }

template<rvkSamplerImgViewType imgVType>
template<typename imgType>
inline rvkSamplerImgView<imgVType>::rvkSamplerImgView(const std::shared_ptr<rvkSampler> &sampler, 
const std::shared_ptr<imgType> &img,
const VkImageViewCreateInfo &imgVCISample)
	: _sampler(sampler), _img(img)
{
	_imgView = sampler->_createColorImgVFunc(imgVCISample, *_img.get());
}

/*rvkSamplerImgView(rvkSamplerImgView &&other)
	: _sampler(std::move(other._sampler)), _img(std::move(other._img)), _imgView(other._imgView)
{
	other._imgView = VK_NULL_HANDLE;
}*/

template<rvkSamplerImgViewType imgVType>
inline rvkSamplerImgView<imgVType>::~rvkSamplerImgView()
{
	auto hLogicalDevice = _appInstance->getLogicalDevice();
	auto pAllocator = _appInstance->getpAllocator();
	vkDestroyImageView(hLogicalDevice, _imgView, pAllocator);
}

template<rvkSamplerImgViewType imgVType>
inline auto rvkSamplerImgView<imgVType>::getVkImageView() const noexcept 
{ 
/*#ifndef NDEBUG
	if(_imgView == VK_NULL_HANDLE)
		throw graphicalError("image view has been moved");
#endif*/
	return _imgView; 
}

template<typename imgType>
inline rvkSamplerImgView_2D_color::rvkSamplerImgView_2D_color(const std::shared_ptr<rvkSampler> &sampler, 
const std::shared_ptr<imgType> &img)
	: base_t(sampler, img, _internal_createCI())
{

}

inline constexpr VkImageViewCreateInfo rvkSamplerImgView_2D_color::_internal_createCI()
{
	VkImageViewCreateInfo imgVCIS { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
	imgVCIS.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imgVCIS.subresourceRange.layerCount = 1;
	imgVCIS.subresourceRange.levelCount = 1;
        //imgVCIS.subresourceRange.aspectMask = rvkPlaneFlags[planeIndex];

	return imgVCIS;
}

inline rvkYcbcrSampler::rvkYcbcrSampler( const std::shared_ptr<vulkanAppInstance> &appInstance, 
const rvkSamplerSettings &settings,
const std::shared_ptr<rvkSamplerYcbcrConversion> &syc)
	: rvkSampler(appInstance, createSettings(appInstance, settings, syc), syc), _syc(syc)
{
	_createColorImgVFunc = [this](const VkImageViewCreateInfo &imgVCISample, 
const rvkImage &img)
	{
		if(!this->isCompatibleWithImg(img))
			throw graphicalError("incompatible image with sampler was choosen");
		VkImageViewCreateInfo ivci = imgVCISample;
		VkSamplerYcbcrConversionInfo syconvI{ VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_INFO };
		syconvI.pNext = ivci.pNext;
		syconvI.conversion = this->_syc->_syc;
		ivci.pNext = &syconvI;

		return rvkSampler::defaultCreateColorImgVFunc(ivci, img);
	};
}

inline bool rvkYcbcrSampler::isCompatibleWithImg(const rvkImage &img)
{
	return _syc->isCompatibleWith(img);
}


inline rvkSamplerYcbcrSettings rvkYcbcrSampler::createSettings(
const std::shared_ptr<vulkanAppInstance> &appInstance,
	const rvkSamplerSettings &settings,
	const std::shared_ptr<rvkSamplerYcbcrConversion> &syc)
{
	rvkSamplerYcbcrSettings retSettings(settings, syc);


	auto hLogicalDevice = appInstance->getLogicalDevice();
	auto pAllocator = appInstance->getpAllocator();
	auto formatFeatureFlags = syc->_formatFeatures;
	const rvkPhysicalDeviceFeatures &deviceFeatures = appInstance->getDeviceFeatures();

	if(deviceFeatures._samplerYcbcrConversion == VK_FALSE)
		throw graphicalError("ycbcr feature for device not enabled,"
			" cannot create a ycbcr sampler");

	// applying API's specifications for when ycbcr conversion is enabled

	/* .If sampler YCBCR conversion is enabled and 
	 * VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT 
	 * is not set for the format, minFilter and magFilter must be equal to 
	 * the sampler YCBCR conversionâ€™s chromaFilter
	 * 
	 * .If sampler YCBCR conversion is enabled, addressModeU, addressModeV, 
	 * and addressModeW must be VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, 
	 * anisotropyEnable must be VK_FALSE, and unnormalizedCoordinates 
	 * must be VK_FALSE
	 */
	if(!(VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT &
		formatFeatureFlags))
	//if(!allowDifferentMinAndMagFiltersArg)
	{
		if(retSettings._info.minFilter != retSettings._info.magFilter)
		{
			retSettings._info.magFilter = retSettings._info.minFilter;
			appInstance->generateWarning(rvkWarnings_overriddenSettings,
				"different minFilter and magFilter not supported, "
			"maxFilter was overridden");
		}
	}
	if(retSettings._info.addressModeU != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
	{
		retSettings._info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		appInstance->generateWarning(rvkWarnings_overriddenSettings,
			"addressModeU was overridden to VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE. "
		"other values are not supported with a ycbcr sampler");
	}
	if(retSettings._info.addressModeV != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
	{
		retSettings._info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		appInstance->generateWarning(rvkWarnings_overriddenSettings,
			"addressModeV was overridden to VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE. "
		"other values are not supported with a ycbcr sampler");
	}
	if(retSettings._info.addressModeW != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
	{
		retSettings._info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		appInstance->generateWarning(rvkWarnings_overriddenSettings,
			"addressModeW was overridden to VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE. "
		"other values are not supported with a ycbcr sampler");
	}
	if(retSettings._info.anisotropyEnable != VK_FALSE)
	{
		retSettings._info.anisotropyEnable = VK_FALSE;
		appInstance->generateWarning(rvkWarnings_overriddenSettings,
			"anisotropic filtering is not supported with a ycbcr sampler. "
		"it was disabled automatically");
	}
	if(retSettings._info.unnormalizedCoordinates != VK_FALSE)
	{
		throw graphicalError("unnormalized Coordinates not supported with a ycbcr sampler");
	}

	return retSettings;
}


static inline std::shared_ptr<rvkSampler> make_sampler_shared(
const std::shared_ptr<vulkanAppInstance> &appInstance, 
	const rvkSamplerSettings &settings, const rImageFormat &format)
{
	if(
		rimagel::isYcbcr(format._pixFmt))
	{
		return std::make_shared<rvkYcbcrSampler>(appInstance, settings,
			std::make_shared<rvkSamplerYcbcrConversion>(
		appInstance, format));
	}
	else
	{
		return std::make_shared<rvkSampler>(appInstance, settings);
	}
}

}
}