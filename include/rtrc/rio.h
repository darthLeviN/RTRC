/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   RIO.h
 * Author: roohi
 *
 * Created on June 29, 2019, 4:22 PM
 */
#pragma once
#include "rstring.h"
namespace rtrc { namespace iol
{
	enum txtEncodings
	{
		utf8, utf16, utf32
	};
	
	template<typename outT, txtEncodings encoding>
	struct txtfileStCodes;
    
    template<typename>
	struct txtfileStCodes<wchar_t,utf16>
	{
		enum codes : wchar_t
		{
			BOM = 0xfeffllu, reverseBOM = 0xfffellu
		};
	};
    
 }}