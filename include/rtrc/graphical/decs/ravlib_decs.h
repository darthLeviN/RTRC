/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ravlib.h
 * Author: roohi
 *
 * Created on September 10, 2019, 7:57 PM
 */
#pragma once
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/error.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}
#include "../../rexception.h"
#include "../rimage_core.h"
#include <memory>
#include <vector>
#include <exception>

// remove this using later.
using namespace rtrc::rimagel;

namespace rtrc { namespace ravl {

// converts AVPixelFormat to rImagePixFmt
static inline constexpr rImagePixFmt ravToRImagePixFmt(AVPixelFormat avfmt) noexcept;

constexpr rImagePixelComp AVPixDecsCompTorImage( const AVComponentDescriptor &comp);

static inline rColorSpace AVColorSpaceToRColorSpace( AVColorSpace cs);

static inline rYcbcrColorRange AVColorRangeToRYcbcrColorRange( AVColorRange cr);

static inline rChromaLocation AVChromaLocationToRChromaLocation( AVChromaLocation cl);

//
// library exceptions
//
template<size_t extraSize>
static inline auto getAvErrorStr(int error, const char (&extraStr)[extraSize]);

struct AVlibException : RbasicException
{
	template<size_t extraSize>
	AVlibException(int error, const char (&sourceName)[extraSize]) noexcept;
	AVlibException(const char *msg);
	const int value = 0;
};

struct AVEOFException : rEOFException
{
	AVEOFException(const char *msg);
};

struct AVInputTryAgainException : rReadTryAgainException
{
	AVInputTryAgainException(const char *msg);
};

template<size_t extraSize>
inline void checkAVError(int error, const char (&sourceName)[extraSize]);


inline void ravRemoveDeprecatedPixelformat(int *pix_fmt);

struct rav_codec_context_opaque
{
	AVPixelFormat hwPixelFormat = AV_PIX_FMT_NONE;
};

static enum AVPixelFormat rav_get_hw_format(AVCodecContext *ctx,
                                        const enum AVPixelFormat *pix_fmts);

static inline void ravcodec_free_context(AVCodecContext *&cctx);

static inline AVCodecContext *ravcodec_alloc_context(const AVCodec *codec);

// based on ffmpeg 4.1.
static inline const AVCodec *rav_find_decoder(const AVFormatContext *s, const AVStream *st, enum AVCodecID codec_id);

const AVCodecHWConfig *rav_get_decoder_hwconfig_dctx(const AVCodec *codec);

static inline size_t ravGetPlanetCount(const AVPixFmtDescriptor *desc);

inline size_t ravGetPlaneHeight(size_t picHeight, size_t planeIndex, const AVPixFmtDescriptor *desc);

inline size_t ravGetPlaneWidth(size_t picWidth, size_t planeIndex, const AVPixFmtDescriptor *desc);

void rav_remove_incompatible_pixel_formats(AVFrame *&frame);
// dev note : add a stream index parameter for the callable. AVFrame does not have it.
template<typename callable_t>
static inline void ravdecode(AVCodecContext *dec_ctx, AVFrame *frame, AVFrame *frame_tmp, AVPacket *pkt, callable_t &callable);

struct ravPacket
{
	ravPacket(const ravPacket &) = delete;
	ravPacket(ravPacket &&moved);
	ravPacket &operator=(const ravPacket &) = delete;

	ravPacket();

	void unref();

	~ravPacket();

	const AVPacket *get() const;
	AVPacket *get();

private:
	AVPacket *_ptr;
};

struct ravDecoderContext
{
	AVCodecContext *cctx;
	const AVCodecHWConfig *chwconfig;
};


struct ravDecodedIFContext
{
	ravDecodedIFContext(const ravDecodedIFContext &) = delete;
	ravDecodedIFContext(ravDecodedIFContext &&moved );
	ravDecodedIFContext &operator=(const ravDecodedIFContext &) = delete;

	ravDecodedIFContext(const char *url, /*const*/ AVInputFormat *fmt = nullptr, 
	AVDictionary **options = nullptr, AVDictionary **codecOptions = nullptr);
	~ravDecodedIFContext();

	/* reads data from the input and decodes it and calls the callable on the 
	 * decoded result.
	 * synchronization : thread safe
	 * exceptions : AVlibException, AVEOFException, AVInputTryAgainException
	 */
	template<typename callable_t, bool unrefPreviousPkt = true>
	void readAndDecode(callable_t &callable);
	
	void dump_format_all(const char *url);

private:

	AVFormatContext *_fctx() noexcept;
	AVPacket *_lastFramePacket() noexcept;
	template<bool unrefPreviousPkt = true>
	void _readNextFrame();

	template<typename callable_t>
	void _decodeLastFrame(callable_t &callable);

	void _cleanup() noexcept;

	template<typename callable_t>
	void _decodePacket(AVPacket *pkt, callable_t &callable);


	std::mutex _gm; // general mutex for all operations.
	bool _eofReached = false;
	AVFormatContext *_formatContext;
	AVPacket *_packet;
	AVFrame *_decodingFrame;
	AVFrame *_decodingFrame_tmp;
	std::vector<ravDecoderContext> _codecContexts;
};


struct ravDict
{
	ravDict(const ravDict &) = delete;
	ravDict(ravDict &&moved);
	ravDict &operator=(const ravDict &) = delete;
	ravDict();

	~ravDict();

	AVDictionary *_ptr = nullptr;
};

struct ravCodecParser
{
	ravCodecParser(const ravCodecParser &) = delete;
	ravCodecParser(ravCodecParser &&) = delete;
	ravCodecParser &operator=(const ravCodecParser &) = delete;

	ravCodecParser(int codec_id);

	~ravCodecParser();

private:
	AVCodecParserContext *_ptr;
};





struct ravPicture : rImage
{
	ravPicture(const ravPicture &) = delete;
	ravPicture &operator=(const ravPicture &) = delete;

	ravPicture(ravPicture &&other);

	ravPicture(AVFrame *fr);
	ravPicture(const int &w, const int &h, const AVPixelFormat &pix_fmt, 
	const AVChromaLocation &chromaLoc = AVCHROMA_LOC_UNSPECIFIED,
	const AVColorSpace &colorSpace = AVCOL_SPC_NB, 
	const AVColorRange &colorRange = AVCOL_RANGE_UNSPECIFIED);

	// copies all data to a single block of memory with a capacity of _dataSize;
	/*void copyTo(void *dst) const noexcept
	{
		memcpy(dst, _data[0], _dataSize);
	}*/

	~ravPicture();

	AVPixelFormat _avpix_fmt;

	const AVPixFmtDescriptor *_avpix_desc = nullptr;
	AVFrame *_pavframe = nullptr;
};



struct rswsContext
{
	rswsContext(const rswsContext &) = delete;
	rswsContext &operator=(const rswsContext &) = delete;
	rswsContext( rswsContext &&other );

	rswsContext(int srcW, int srcH, AVPixelFormat srcFormat,
							  int dstW, int dstH, AVPixelFormat dstFormat,
							  int flags, SwsFilter *srcFilter,
							  SwsFilter *dstFilter, const double *param);

	rswsContext(AVFrame *srcSample, AVPixelFormat dstFormat);

	void scale(ravPicture &dst, const AVFrame *src);

	~rswsContext();

private:
	SwsContext *_ctx = nullptr;
};

	
}
}

