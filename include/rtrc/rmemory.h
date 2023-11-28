

#pragma once
#include <memory>
#include <cstring>

#include "rexception.h"
#include "rvtypes.h"
#include <vector>
namespace rtrc 
{ namespace memoryl {

enum replacement_method
{
	none,
	copyConstruct,
	moveConstruct,
	copyAssign
};

enum size_types
{
	raw,
	element
};

enum copy_directions
{
	forward_copy,
	reverse_copy
};

template<size_types St>
struct copySize
{
	static constexpr size_types size_type = St;
	size_t size;
};


template<typename T>
static inline constexpr replacement_method choose_replacement_method()
{
	if(std::is_trivially_copy_assignable<T>::value)
		return replacement_method::copyAssign;
	else if(std::is_copy_constructible<T>::value)
		return replacement_method::copyConstruct;
	else if(std::is_move_constructible<T>::value)
		return replacement_method::moveConstruct;
	else
		return replacement_method::none;
}
	
template<typename T, replacement_method = choose_replacement_method<T>()>
struct replacer;

template<typename T>
struct replacer<T,replacement_method::copyAssign>
{
	template<bool callDestructor>
	static void replace(T&dst, T &&src) noexcept
	{
		dst = src;
	}
	
	template<bool callDestructor>
	static void replace(T&dst, T &src) noexcept
	{
		dst = src;
	}
	
	template<bool callDestructor, size_types St, copy_directions cd = copy_directions::forward_copy>
	static void replace(T *dst, T *src, copySize<St> cs) noexcept
	{
		if(cs.size_type == size_types::element)
			if(cd == copy_directions::forward_copy)
			{
				memcpy(dst,src,sizeof(T)*cs.size);
			}
			else
			{
				// experimental.
				auto srcCur = src+cs.size-1;
				auto dstCur = dst;
				auto dstCurEnd = dst + cs.size;
				for(;dstCur != dstCurEnd; ++dstCur,--srcCur)
				{
					replace(*dstCur,*srcCur);
				}
			}
			
		else // raw size
		{
			if(cd == copy_directions::forward_copy)
			{
				memcpy(dst,src,cs.size);
			}
			else
			{
				size_t realSize = cs.size/sizeof(T);
				// experimental.
				auto srcCur = src+realSize-1;
				auto dstCur = dst;
				auto dstCurEnd = dst + realSize;
				for(;dstCur != dstCurEnd; ++dstCur,--srcCur)
				{
					replace(*dstCur,*srcCur);
				}
			}
		}
	}
};
	
template<typename T, typename Allocator>
std::vector<T,Allocator> &operator<<(std::vector<T,Allocator> &vc, const T &obj)
{
	vc.push_back(obj);
	return vc;
}

	
/* things that can be done :
 * .vector's  issues :
 * 1- emplace_back does not have a version that takes references, make an alternative.
 * 2- vector does not have the option to specify if range check should be done or not.
 *	this leads to issues, like not having the option to not do it for the first reserve call.
 * .tuple construction is not optimal, make a more raw form of tuple that avoids move constructors.
 */
}
}