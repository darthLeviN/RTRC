
#pragma once
#include <vector>
#include <string>
#include <cstdlib>
#include "rvtypes.h"



extern "C"
{
#include <libavformat/avformat.h>
}

namespace rtrc
{

    
    struct RDict
    {
	RDict(size_t reserveSize = 0);
	RDict(const RDict& rDic) = delete; // currently not supported.
	RDict& operator=(const RDict&) = delete;
	void set(const char* key, const char* value);

	std::string getString(const char* key);
	//std::vector<std::string> getStringGroup(const char* key, char separator);
	int getInt(const char* key);
	//std::vector<int> getIntGroup(const char* key, char separator);
	RRational getRRational(const char* key);
	//std::vector<RRational> getRRationalGroup(const char* key, char separator);
	float getFloat(const char* key);
	//std::vector<float> GetFloatGroup(const char* key, char separator);
	
	void reserve(size_t rSize);
	
	~RDict();
	
	
    private:
	struct dictEntry
	{
	    /* performance dev note : use string_view in future.
	     * sharing string resources may or may not increase efficiency but will
	     * introduce a chance for instability in multithreading applications.
	     */
	    std::string fullKeyString, fullValueString;
	    //std::vector<std::string> keyStrings, valueStrings; unsupported at this moment.
	};

	std::vector<dictEntry> entries;
    };

	
}