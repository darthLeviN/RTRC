#pragma once

#include "../ravlib.h"

namespace rtrc { namespace media {
	
// creates a decoder.
static auto create_decoder(const char *feed_path, AVInputFormat *demuxer_format);

}
}