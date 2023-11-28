/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   base.h
 * Author: roohi
 *
 * Created on October 13, 2019, 8:08 PM
 */
#pragma once
#include <type_traits>
namespace rtrc
{
namespace coll{

namespace collbase
{
enum replacement_method
{
	none,
	copyConstruct,
	moveConstruct,
	copyAssign
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
	
template<typename T, replacement_method>
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
	
	template<bool callDestructor>
	static void replace(T *dst, T *src, size_t size) noexcept
	{
		memcpy(dst,src,sizeof(T)*size);
	}
};

}
}
	
}