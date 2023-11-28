
#pragma once

extern "C"
{
#include <libavformat/avformat.h>
}


namespace rtrc
{
	class REvents
	{
		
	public:
		// default events.
	};

	class REventHandler : REvents
	{
		AVDictionary* dict = nullptr;
	public:
	};
}