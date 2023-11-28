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
#include "decs/ravlib_decs.h"
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/error.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}
#include "../rexception.h"
#include "rimage_core.h"
#include <memory>
#include <vector>
#include <exception>

// remove this using later.
using namespace rtrc::rimagel;

namespace rtrc { namespace ravl {
	
inline constexpr auto makeRavpToRImagePixFmt()
{
	std::array<rImagePixFmt,AV_PIX_FMT_NB + 1 /*one extra for AV_PIX_FMT_NONE*/> retArr{};
	retArr[AV_PIX_FMT_NONE + 1] = rimg_noFormat; /*should be index of zero*/
	retArr[AV_PIX_FMT_YUV420P + 1] = rimg_yuv420p;
	retArr[AV_PIX_FMT_NV12 + 1] = rimg_yuv420p_NV12;
	retArr[AV_PIX_FMT_RGBA + 1] = rimg_rgba32;
	return retArr;
}

static constexpr auto ravToRImagePixFmtArr = makeRavpToRImagePixFmt();


// converts AVPixelFormat to rImagePixFmt
static inline constexpr rImagePixFmt ravToRImagePixFmt(AVPixelFormat avfmt) noexcept
{
	return ravToRImagePixFmtArr[avfmt+1];
}

constexpr rImagePixelComp AVPixDecsCompTorImage( const AVComponentDescriptor &comp)
{
	rImagePixelComp retComp{};
	retComp.planeIndex = comp.plane;
	retComp.stepSize = comp.step;
	retComp.offset = comp.offset;
	retComp.shift = comp.shift;
	retComp.depth = comp.depth;
	return retComp;
}

static inline  rColorSpace AVColorSpaceToRColorSpace( AVColorSpace cs)
{
	switch(cs)
	{
		case AVCOL_SPC_RGB:
			return rColorSpace::rColorSpace_srgb;
			break;
		case AVCOL_SPC_BT709:
			return rColorSpace::rColorSpace_709;
			break;
		case AVCOL_SPC_BT470BG:
			return rColorSpace::rColorSpace_601;
			break;
		case AVCOL_SPC_BT2020_CL:
		case AVCOL_SPC_BT2020_NCL:
			return rColorSpace::rColorSpace_2020;
			break;
		default:
			return rColorSpace::rColorSpace_undefined;
	}
}

static inline  rYcbcrColorRange AVColorRangeToRYcbcrColorRange( AVColorRange cr)
{
	switch(cr)
	{
		case AVCOL_RANGE_MPEG:
			return rYcbcrColorRange::rYcbcrColorRange_ITU_narrow;
			break;
		case AVCOL_RANGE_JPEG:
			return rYcbcrColorRange::rYcbcrColorRange_full;
			break;
		default:
			return rYcbcrColorRange::rYcbcrColorRange_undefined;
	}
}

static inline rChromaLocation AVChromaLocationToRChromaLocation( AVChromaLocation cl)
{
	return (rChromaLocation) cl;
}

//
// library exceptions
//
template<size_t extraSize>
static inline auto getAvErrorStr(int error, const char (&extraStr)[extraSize])
{
	static constexpr size_t totalSize = 64+extraSize;
	std::array<char,64> errStr;
	int secondError = av_strerror(error, errStr.data(), errStr.size());
	if(secondError < 0) av_strerror(secondError, errStr.data(), errStr.size());
	return Rsnprintf<char,totalSize+2/*? not sure if this 2 is needed*/>("%s:%s", extraStr, errStr.data()); 
}

template<size_t extraSize>
inline AVlibException::AVlibException(int error, const char (&sourceName)[extraSize]) noexcept : value(error), RbasicException(getAvErrorStr(error, sourceName).data) {}
inline AVlibException::AVlibException(const char *msg) : RbasicException(msg) {}


inline AVEOFException::AVEOFException(const char *msg) : rEOFException(msg) {}

inline AVInputTryAgainException::AVInputTryAgainException(const char *msg) : rReadTryAgainException(msg) {}

template<size_t extraSize>
inline void checkAVError(int error, const char (&sourceName)[extraSize])
{
	if(error < 0)
		throw AVlibException(error,sourceName);
}


inline void ravRemoveDeprecatedPixelformat(int *pix_fmt)
{
	switch(*pix_fmt)
	{

		case AV_PIX_FMT_YUVJ420P:
		{
			*pix_fmt = AV_PIX_FMT_YUV420P;
			break;
		}
		case AV_PIX_FMT_YUVJ411P:
		{
			*pix_fmt = AV_PIX_FMT_YUV411P;
			break;
		}
		case AV_PIX_FMT_YUVJ422P:
		{
			*pix_fmt = AV_PIX_FMT_YUV422P;
			break;
		}
		case AV_PIX_FMT_YUVJ440P:
		{
			*pix_fmt = AV_PIX_FMT_YUV440P;
			break;
		}
		case AV_PIX_FMT_YUVJ444P:
		{
			*pix_fmt = AV_PIX_FMT_YUV444P;
			break;
		}
		default:
			break;
	}
}

static enum AVPixelFormat rav_get_hw_format(AVCodecContext *ctx,
                                        const enum AVPixelFormat *pix_fmts)
{
    const enum AVPixelFormat *p;
	
	const auto &opq = (rav_codec_context_opaque *)ctx->opaque;
	if(opq)
	{
		for (p = pix_fmts; *p != -1; p++) {
			if (*p == opq->hwPixelFormat)
				return *p;
			else
			{
				auto fmtName = av_get_pix_fmt_name(*p);
				//mainLogger.logText(Rsnprintf<char,100>("rav_get_hw_format did not find %s pixel format",
					//fmtName ? fmtName : "unknown pixel format"));
			}
		}
	}
	mainLogger.logText("rav_get_hw_format did not find pixel format");
	//return opq ? opq->hwPixelFormat : nullptr;
    return AV_PIX_FMT_NONE;
}

static inline void ravcodec_free_context(AVCodecContext *&cctx)
{
	if(cctx->opaque)
	{
		std::free(cctx->opaque);
		//cctx->opaque = nullptr; not needed
	}
	avcodec_free_context(&cctx);
}

static inline AVCodecContext *ravcodec_alloc_context(const AVCodec *codec)
{
	auto ret = avcodec_alloc_context3(codec);
	if(!ret)
		throw AVlibException("could not allocate codec context");
	ret->opaque = std::malloc(sizeof(rav_codec_context_opaque));
	if(!ret->opaque)
		throw AVlibException("could not allocate codec context opaque");
	//ret->get_format = rav_get_hw_format;
	return ret;
}

// based on ffmpeg 4.1.
static inline const AVCodec *rav_find_decoder(const AVFormatContext *s, const AVStream *st, enum AVCodecID codec_id)
{
#if FF_API_LAVF_AVCTX
	if (st->codec->codec)
		return st->codec->codec;
#endif
	switch (st->codecpar->codec_type) {
	case AVMEDIA_TYPE_VIDEO:
		if (s->video_codec)    return s->video_codec;
		break;
	case AVMEDIA_TYPE_AUDIO:
		if (s->audio_codec)    return s->audio_codec;
		break;
	case AVMEDIA_TYPE_SUBTITLE:
		if (s->subtitle_codec) return s->subtitle_codec;
		break;
	}
	return avcodec_find_decoder(codec_id);
}

const AVCodecHWConfig *rav_get_decoder_hwconfig_dctx(const AVCodec *codec)
{
	size_t i = 0;
	auto ret = avcodec_get_hw_config(codec,i);
	while(ret != nullptr && (ret->device_type == AV_HWDEVICE_TYPE_NONE || 
		ret->device_type == AV_HWDEVICE_TYPE_CUDA /*excluding cuda for issues with the get_format function*/ || 
		!(ret->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX)))
	{
		// this one was not suitable, but the next one may be.
		++i;
		ret = avcodec_get_hw_config(codec,i);
	}
	// returns nullptr if no suitable device is found.
	if(ret)
		mainLogger.logText(Rsnprintf<char,100>("hardware decoder with device type of %d found", ret->device_type).data);
	return ret;
}

static inline size_t ravGetPlanetCount(const AVPixFmtDescriptor *desc)
{
	// based on undocumented details from ffmpeg 4.1 code.
	size_t planesCount = 0;
	for (size_t i = 0; i < desc->nb_components; i++)
		planesCount = FFMAX(desc->comp[i].plane, planesCount);
	return ++planesCount;
}

inline size_t ravGetPlaneHeight(size_t picHeight, size_t planeIndex, const AVPixFmtDescriptor *desc)
{
	// based on undocumented details from ffmpeg 4.1 code.
	// plane counts can be pre calculated for all libav pixel formats. but for now this will do.
	size_t planesCount(ravGetPlanetCount(desc));
	if(planeIndex >= planesCount) return 0;
	size_t s = (planeIndex == 1 || planeIndex == 2) ? desc->log2_chroma_h : 0;
	return (picHeight + (1 << s) - 1) >> s;
}

inline size_t ravGetPlaneWidth(size_t picWidth, size_t planeIndex, const AVPixFmtDescriptor *desc)
{
	// based on undocumented details from ffmpeg 4.1 code.
	// plane counts can be pre calculated for all libav pixel formats. but for now this will do.
	size_t planesCount(ravGetPlanetCount(desc));
	if(planeIndex >= planesCount) return 0;
	size_t s = (planeIndex == 1 || planeIndex == 2) ? desc->log2_chroma_w : 0;
	return (picWidth + (1 << s) - 1) >> s;
}

void rav_remove_incompatible_pixel_formats(AVFrame *&frame)
{
	
}

// dev note : add a stream index parameter for the callable. AVFrame does not have it.
template<typename callable_t>
static inline void ravdecode(AVCodecContext *dec_ctx, AVFrame *frame, AVFrame *frame_tmp, AVPacket *pkt, callable_t &callable)
{
	bool gotFrame = false;
	if(pkt->flags & AV_PKT_FLAG_CORRUPT)
		throw AVInputTryAgainException("cannot decode packet. try again");
	int ret = avcodec_send_packet(dec_ctx, pkt);
	if(ret == AVERROR(EINVAL))
	{
		avcodec_flush_buffers(dec_ctx);
		ret = avcodec_send_packet(dec_ctx, pkt);
	}
	checkAVError(ret, "ravdecode avcodec_send_packet");
	while(true)
	{
		ret = avcodec_receive_frame(dec_ctx, frame);
		if(ret == AVERROR(EAGAIN))
			break;
		else if(ret == AVERROR_EOF)
			throw AVEOFException("codec reached eof");
		else
			checkAVError(ret,"ravdecode avcodec_receive_frame");
		ravRemoveDeprecatedPixelformat(&frame->format);
		gotFrame = true;
		if(frame->format != AV_PIX_FMT_NONE && frame->format == ((rav_codec_context_opaque *)dec_ctx->opaque)->hwPixelFormat)
		{
			av_frame_unref(frame_tmp);
			if(dec_ctx->codec_type == AVMEDIA_TYPE_VIDEO)
			{
				checkAVError(av_hwframe_transfer_data(frame_tmp, frame, 0), "ravdecode av_hwframe_transfer_data");	
			}
			else
			{
				throw AVlibException("unimplemented pixel media type for hardware acceleration");
			}
			callable(frame_tmp);
		}
		else
		{
			callable(frame);
		}
	}
	if(!gotFrame)
		throw AVInputTryAgainException("cannot decode packet. try again");
}

inline ravPacket::ravPacket(ravPacket &&moved) : _ptr(moved._ptr)
{
	moved._ptr = nullptr;
}


inline ravPacket::ravPacket() : _ptr(av_packet_alloc())
{
	if(!_ptr) throw AVlibException("could not allocate packet");
}

inline void ravPacket::unref()
{
	if(_ptr)
		av_packet_unref(_ptr);
	else
		throw AVlibException("cannot unref a moved packet.");
}

inline ravPacket::~ravPacket()
{
	if(_ptr)
		av_packet_free(&_ptr);
}

inline const AVPacket *ravPacket::get() const { return _ptr; };
inline AVPacket *ravPacket::get() { return _ptr; };



inline ravDecodedIFContext::ravDecodedIFContext(ravDecodedIFContext &&moved ) : _formatContext(moved._formatContext), _packet(moved._packet),
_decodingFrame(moved._decodingFrame)
{
	std::lock_guard<std::mutex> lk(moved._gm);
	moved._formatContext = nullptr;
	moved._packet = nullptr;
	moved._decodingFrame = nullptr;
}


inline ravDecodedIFContext::ravDecodedIFContext(const char *url, /*const */AVInputFormat *fmt, 
AVDictionary **options, AVDictionary **codecOptions)
	: _formatContext(nullptr), _packet(av_packet_alloc()), _decodingFrame(av_frame_alloc()),
	_decodingFrame_tmp(av_frame_alloc())
{
	//AVDictionary *retCodecOptions = nullptr;
	AVDictionary *tmpDict = nullptr;
	try
	{
		if(!_packet) throw AVlibException("could not allocate AVPacket");
		if(!_decodingFrame || !_decodingFrame_tmp) throw AVlibException("could not allocate AVFrame");
		checkAVError(avformat_open_input(&_formatContext, url, fmt, options), "ravDecodedIFContext constructor avformat_open_input");
		const size_t streamsCount = _formatContext->nb_streams;
		if(streamsCount == 0)
			throw AVlibException("AVFormatContext has no streams!");

		_codecContexts.resize(streamsCount); // zero initialized

		// open a decoder for each stream.
		for( size_t i = 0; i < streamsCount; ++i )
		{
			const auto &stream = _formatContext->streams[i];
			const auto &codecPar = stream->codecpar;
			auto &cctx = _codecContexts[i].cctx;
			auto &chwConfig = _codecContexts[i].chwconfig;
			// doing this over and over may not be the best option for too many similar files
			const AVCodec *codec = rav_find_decoder(_formatContext, stream,codecPar->codec_id); 
			//const AVCodec *codec = avcodec_find_decoder(codecPar->codec_id);
			mainLogger.logText(Rsnprintf<char,100>("using codec id : %d", codecPar->codec_id).data);
			chwConfig = rav_get_decoder_hwconfig_dctx(codec);
			cctx = ravcodec_alloc_context(codec);
			checkAVError(avcodec_parameters_to_context(cctx,codecPar), "ravDecodedIFContext constructor avcodec_parameters_to_context");
			/*if(chwConfig)
			{
				mainLogger.logText("trying to initialize decoder with hardware acceleration");
				checkAVError(av_hwdevice_ctx_create(&cctx->hw_device_ctx, chwConfig->device_type,
					nullptr, nullptr, 0), "ravDecodedIFContext constructor av_hwdevice_ctx_create");
				auto ravopq = (rav_codec_context_opaque *)cctx->opaque;
				ravopq->hwPixelFormat = chwConfig->pix_fmt;
				cctx->get_format = rav_get_hw_format;
			}*/
			if(codecOptions)checkAVError(av_dict_copy(&tmpDict, *codecOptions, 0), "ravDecodedIFContext constructor av_dict_copy");		
			if(!cctx->hw_device_ctx)
			{
				mainLogger.logText(Rsnprintf<char,100>("%s decoder without hardware acceleration initialized", 
					codec->name).data);
			}

			else 
			{
				auto fmtName = av_get_pix_fmt_name(chwConfig->pix_fmt);
				mainLogger.logText(Rsnprintf<char,128>("%s decoder with hardware acceleration initialized with %s pixel format", 
					codec->name, fmtName ? fmtName : "unknown pixel format").data);
			}
			checkAVError(avcodec_open2(cctx, codec, &tmpDict),"ravDecodedIFContext constructor avcodec_open2");

			av_dict_free(&tmpDict);
		}
		if(codecOptions)av_dict_free(codecOptions);
	}
	catch(...)
	{
		av_dict_free(&tmpDict);
		if(options)av_dict_free(options);
		if(codecOptions)av_dict_free(codecOptions);
		//av_dict_free(&retCodecOptions);
		_cleanup();
		std::rethrow_exception(std::current_exception());
	}
}

inline ravDecodedIFContext::~ravDecodedIFContext()
{
	_cleanup();
}

/* reads data from the input and decodes it and calls the callable on the 
 * decoded result.
 * synchronization : thread safe
 * exceptions : AVlibException, AVEOFException, AVInputTryAgainException
 */
template<typename callable_t, bool unrefPreviousPkt>
inline void ravDecodedIFContext::readAndDecode(callable_t &callable)
{
	std::lock_guard<std::mutex> lk(_gm);
#ifndef NDEBUG
	if(!_formatContext)
		throw AVlibException("trying to use a moved object");
#endif

	_readNextFrame<unrefPreviousPkt>();
	auto tmpStartTime = std::chrono::high_resolution_clock::now();
	_decodeLastFrame(callable);
	size_t currCallTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - tmpStartTime).count();
	mainLogger.logPerformance(Rsnprintf<char,100>("_decodeLastFrame call time = %llums", currCallTime).data);
}

inline void ravDecodedIFContext::dump_format_all(
			const char *url)
{
	std::lock_guard<std::mutex> lk(_gm);
#ifndef NDEBUG
	if(!_formatContext)
		throw AVlibException("trying to use a moved object");
#endif
	for(size_t i = 0; i < _formatContext->nb_streams; ++i)
	{
		av_dump_format(_formatContext, i, url, 0);

	}
}


inline AVFormatContext *ravDecodedIFContext::_fctx() noexcept { return _formatContext; }
inline AVPacket *ravDecodedIFContext::_lastFramePacket() noexcept { return _packet; }
template<bool unrefPreviousPkt>
inline void ravDecodedIFContext::_readNextFrame()
{
	if(_eofReached)
		throw AVEOFException("EOF reached, cannot read more frames");
	if(unrefPreviousPkt)av_packet_unref(_packet);
	int ret = av_read_frame(_formatContext, _packet);
	if(ret)
	{
		mainLogger.logText2("failed to reat packet from input");
		if(ret == AVERROR_EOF)
		{
			throw AVEOFException("eof reached while trying to read input");
			_eofReached = true;
		}
		else
			throw AVlibException(ret, "ravDecodedIFContext::_readNextFrame()");
	}
}

template<typename callable_t>
inline void ravDecodedIFContext::_decodeLastFrame(callable_t &callable)
{
	_decodePacket(_packet,callable);
}

void ravDecodedIFContext::_cleanup() noexcept
{
	if(_decodingFrame)av_frame_free(&_decodingFrame);
	if(_decodingFrame_tmp)av_frame_free(&_decodingFrame_tmp);
	if(_packet)av_packet_free(&_packet);
	for( auto &cctxInfo : _codecContexts )
	{
		ravcodec_free_context(cctxInfo.cctx);
	}
	if(_formatContext)avformat_close_input(&_formatContext);
}

template<typename callable_t>
inline void ravDecodedIFContext::_decodePacket(AVPacket *pkt, callable_t &callable)
{
	AVCodecContext *&cctx = _codecContexts[pkt->stream_index].cctx;

	ravdecode(cctx,_decodingFrame, _decodingFrame_tmp,pkt,callable);
}


inline ravDict::ravDict(ravDict &&moved) : _ptr(moved._ptr) { moved._ptr = nullptr; }

inline ravDict::ravDict()
{
}

inline ravDict::~ravDict()
{
	av_dict_free(&_ptr);
}	

inline ravCodecParser::ravCodecParser(int codec_id)
	: _ptr(av_parser_init(codec_id))
{
	if(!_ptr)
		throw AVlibException("codec parser not found");
}

inline ravCodecParser::~ravCodecParser()
{
	av_parser_close(_ptr);
}

inline ravPicture::ravPicture(ravPicture &&other)
{
	memcpy(this, &other, sizeof(ravPicture));
	memset(&other, 0, sizeof(ravPicture));
}

inline ravPicture::ravPicture(AVFrame *fr)
	: rImage{}, _pavframe(av_frame_alloc())
{
	if(!_pavframe)
		throw AVlibException("could not allocate frame");
	try
	{
		checkAVError(av_frame_ref(_pavframe, fr), "ravPicture constructor, av_frame_ref");
		_width = fr->width;
		_height = fr->height;
		_format._pixFmt = ravToRImagePixFmt((AVPixelFormat)fr->format);
		_format._chromaLoc = AVChromaLocationToRChromaLocation(fr->chroma_location);
		_format._colorSpace = AVColorSpaceToRColorSpace(fr->colorspace);
		_format._colorRange = AVColorRangeToRYcbcrColorRange(fr->color_range);
		_avpix_desc = av_pix_fmt_desc_get((AVPixelFormat)fr->format);
		_format._planeCount = 0;
		_dataSize = 0;
		for(size_t i/*plane index*/ = 0; i < 4; ++i)
		{
			_data[i] = fr->data[i];
			_planeHeights[i] = ravGetPlaneHeight(_height, i, _avpix_desc);
			_planeWidths[i] = ravGetPlaneWidth(_width, i, _avpix_desc);
			if(_planeHeights[i]) ++_format._planeCount;
			_linesizes[i] = fr->linesize[i];
			_planeSizes[i] = _linesizes[i] * _planeHeights[i];
			_dataSize += _planeSizes[i];

		}
		_format._flags = _avpix_desc->flags;
		_compCount = _avpix_desc->nb_components;
		for(size_t i = 0; i < _compCount; ++i)
		{
			_pixComps[i] = AVPixDecsCompTorImage(_avpix_desc->comp[i]);
		}
	}
	catch(...)
	{
		av_frame_free(&_pavframe);
		std::rethrow_exception(std::current_exception());
	}

}

inline ravPicture::ravPicture(const int &w, const int &h, const AVPixelFormat &pix_fmt, 
const AVChromaLocation &chromaLoc,
const AVColorSpace &colorSpace, 
const AVColorRange &colorRange)
	: rImage{}
{
	_width = w;
	_height = h;
	_avpix_fmt = pix_fmt;
	_format._pixFmt = ravToRImagePixFmt(pix_fmt);
	_format._chromaLoc = AVChromaLocationToRChromaLocation(chromaLoc);
	_format._colorSpace = AVColorSpaceToRColorSpace(colorSpace);
	_format._colorRange = AVColorRangeToRYcbcrColorRange(colorRange);
	_dataSize = av_image_alloc(_data, _linesizes, w, h, pix_fmt, 1);
	checkAVError(_dataSize, "ravPicture constructor av_image_alloc");
	_avpix_desc = av_pix_fmt_desc_get(pix_fmt);
	_format._planeCount = 0;
	for(size_t i/*plane index*/ = 0; i < 4; ++i)
	{
		_planeHeights[i] = ravGetPlaneHeight(h, i, _avpix_desc);
		_planeWidths[i] = ravGetPlaneWidth(w, i, _avpix_desc);
		if(_planeHeights[i]) ++_format._planeCount;
		_planeSizes[i] = _linesizes[i] * _planeHeights[i];
	}
	// compatible flags
	_format._flags = _avpix_desc->flags;
	_compCount = _avpix_desc->nb_components;
	for(size_t i = 0; i < _compCount; ++i)
	{
		_pixComps[i] = AVPixDecsCompTorImage(_avpix_desc->comp[i]);
	}
	int a = 0;
}

inline ravPicture::~ravPicture()
{
	if(_pavframe)
	{
		// frame referenced
		av_frame_free(&_pavframe);
	}
	else
	{
		// locally created buffer
		av_freep(&_data[0]);
	}

}


inline rswsContext::rswsContext( rswsContext &&other )
	: _ctx(other._ctx)
{
	other._ctx = nullptr;
}

inline rswsContext::rswsContext(int srcW, int srcH, AVPixelFormat srcFormat,
						  int dstW, int dstH, AVPixelFormat dstFormat,
						  int flags, SwsFilter *srcFilter,
						  SwsFilter *dstFilter, const double *param)
	: _ctx(sws_getContext(srcW, srcH, srcFormat, dstW, dstH, dstFormat, 
	flags, srcFilter, dstFilter, param))
{
	if(!_ctx)
		throw AVlibException("could not create Swscontext");
}

inline rswsContext::rswsContext(AVFrame *srcSample, AVPixelFormat dstFormat)
	: rswsContext(srcSample->width, srcSample->height, (AVPixelFormat)srcSample->format,
	srcSample->width, srcSample->height, dstFormat, SWS_BILINEAR, nullptr,
	nullptr, nullptr)
{
}

inline void rswsContext::scale(ravPicture &dst, const AVFrame *src)
{
	sws_scale(_ctx, src->data, src->linesize, 0,
		src->height, dst._data, dst._linesizes);
}

inline rswsContext::~rswsContext()
{
	sws_freeContext(_ctx);
}
	
}
}

