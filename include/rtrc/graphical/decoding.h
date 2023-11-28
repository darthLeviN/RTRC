

#pragma once

#include "decs/ravlib_decs.h"

namespace rtrc { namespace media {

// creates a decoder
inline static auto create_decoder(const char *feed_path, AVInputFormat *demuxer_format)
{
	using namespace rtrc::stringsl;
	
	std::shared_ptr<rtrc::ravl::ravDecodedIFContext> decoder;
	
	AVDictionary *opts{};
	AVDictionary *codecOpts{};
	av_dict_set(&opts, "protocol_whitelist", "file,http,https,rtp,udp,tcp,tls,srt", 0);
	av_dict_set(&opts, "probesize", "32", 0);
	av_dict_set(&opts, "fflags", "+nobuffer", 0);
	av_dict_set(&opts, "analyzeduration", "1", 0);
	av_dict_set(&opts, "flush_packets", "1", 0);
	try
	{
		decoder = std::make_shared<rtrc::ravl::ravDecodedIFContext>(feed_path, demuxer_format, &opts, &codecOpts);
		decoder->dump_format_all(feed_path);
	}
	catch(rtrc::RbasicException &err)
	{
		printf("error : %s\n", err.what());
		mainLogger.logText(Rsnprintf<char,100>("error : %s",err.what()));
		av_dict_free(&opts);
		av_dict_free(&codecOpts);
		exit(-1);
	}
	catch(...)
	{
		printf("could not read input stream\n");
		mainLogger.logText("could not read input stream, unknown error");
		av_dict_free(&opts);
		av_dict_free(&codecOpts);
		exit(-1);
	}
	
	return decoder;
}


}
}