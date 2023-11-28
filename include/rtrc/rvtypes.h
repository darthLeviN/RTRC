
#pragma once

namespace rtrc
{
	typedef void (*voidproc_t)();
	
	template<typename retT, typename ...Args>
	using proc_t = retT (*)(Args...);
}