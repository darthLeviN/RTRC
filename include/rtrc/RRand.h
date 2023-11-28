/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   RRand.h
 * Author: roohi
 *
 * Created on July 4, 2019, 7:00 PM
 */

#pragma once
namespace rtrc
{
	// is unefficient,
	struct RBasicPRNG
	{
		RBasicPRNG();
		size_t generate();
		size_t seed;
	};
}

