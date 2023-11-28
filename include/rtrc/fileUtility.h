
#pragma once

namespace rtrc
{
	
std::vector<char> readFile(const char *filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);
	
	if(!file.is_open())
	{
		throw rtrc::mainAppObjInitializationFailure("could not read file");
	}
	
	size_t fileSize = (size_t) file.tellg();
	std::vector<char> buffer(fileSize);
	file.seekg(0);
	file.read(buffer.data(), fileSize);
	return std::move(buffer);
}

}

