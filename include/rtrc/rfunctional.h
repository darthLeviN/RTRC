/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   rfunctional.h
 * Author: roohi
 *
 * Created on July 22, 2019, 2:44 PM
 */
#include "rvtypes.h"
#include <type_traits>
#include <stdio.h>
#include <functional>
#pragma once
namespace rtrc {
	
	template<typename ...Args>
	using crefTuple_t = std::tuple<std::reference_wrapper<const Args>...>;
	
	
}