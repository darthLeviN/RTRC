
#pragma once
#include <stdint.h>
// this header is used for communication between some libraries without referencing each other.
namespace rtrc {
namespace rimagel {


enum rImagePixFmt
{
	rimg_noFormat = 0,
	rimg_yuvStart,
	rimg_yuv420p,
	rimg_yuv420p_NV12,
	rimg_yuvEnd,
	rimg_rgbStart,
	rimg_rgba32, // 8 bit for each component.
	rimg_rgbEnd,
	rimg_formats_entry_count
};

static inline bool isYcbcr(rImagePixFmt fmt)
{
	if(fmt > rimg_yuvStart && fmt < rimg_yuvEnd)
		return true;
	return false;
}

enum rChromaLocation
{
	rchroma_loc_unspecified = 0,
	rchroma_loc_left = 1,
	rchroma_loc_center = 2,
	rchroma_loc_topleft = 3,
	rchroma_loc_top = 4,
	rchroma_loc_bottomleft = 5,
	rchroma_loc_bottom = 6,
	rchroma_loc_nb // not part of ABI
};

enum rYcbcrColorRange
{
	rYcbcrColorRange_undefined = 0,
	rYcbcrColorRange_full,
	rYcbcrColorRange_ITU_narrow
};

enum rColorSpace
{
	rColorSpace_undefined = 0,
	rColorSpace_srgb,
	rColorSpace_601,
	rColorSpace_709,
	rColorSpace_2020,
};

typedef int64_t rImagePixFmtFlags_t;
enum rImagePixFmtFlagBits : rImagePixFmtFlags_t
{
	rImage_pixFmt_flag_bigEndian = 1<<0,
	rImage_pixFmt_flag_pal = 1<<1,
	rImage_pixFmt_flag_bitstream = 1<<2,
	rImage_pixFmt_flag_planar = 1<<4,
	rImage_pixFmt_flag_rgb = 1<<5,
	// up to 1<<14 reserved for ffmpeg compatibility.
	rImage_pixFmt_flag_secondType = 1<<15
};

struct rImageFormat
{
	size_t _planeCount;
	rImagePixFmt _pixFmt;
	rImagePixFmtFlags_t _flags;
	rChromaLocation _chromaLoc;
	rYcbcrColorRange _colorRange;
	rColorSpace _colorSpace;
};



struct rImagePixelComp
{
	size_t planeIndex; // index in the 4 planes.
	size_t stepSize; // bit for bitStream, byte otherwise. space between components.
	size_t offset; // bit for bitStream, byte otherwise. offset of the starting component.
	size_t shift; // right shift needed to get the value.
	size_t depth; // number of bits in the component for bitStream.
};


struct rImage
{
	/* _data[0] must be less than or equal to _data[i]
	 */
	uint8_t *_data[4]; 
	int _linesizes[4];
	int _planeHeights[4];
	int _planeWidths[4];
	size_t _planeSizes[4]; 
	/* _pixComps :
	 * if rImage_pixFmt_flag_secondType is NOT set :
	 *	if _compCount is 1 or 2, _pixComps[0] is luma. 
	 *	if _compCount is 3 or 4 
	 *		if rgb flag is set _pixComps = { R,G,B,A(if exists) } otherwise
	 *		_pixComps = { luma, chroma-U, chroma-V, A(if exists) }
	 */
	rImagePixelComp _pixComps_internal[8];
	rImagePixelComp *_pixComps = &_pixComps_internal[2];
	
	size_t _compCount;
	rImageFormat _format;
	size_t _dataSize; // of all allocated data from _data[0] up to the last image byte.
	size_t _width;
	size_t _height;
	
};

}
}