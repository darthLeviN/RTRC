/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   dcsynch.h
 * Author: roohi
 *
 * Created on October 13, 2019, 12:07 PM
 */
#pragma once
#include <type_traits>
#include <tuple>
#include <boost/fusion/include/for_each.hpp>
#include "compiletime.h"
#include <algorithm>
#include "rexception.h"
#include <boost/circular_buffer.hpp>
#include "decs/dcsynch_decs.h"
namespace rtrc {
	
	template<typename T>
	template<typename ...Args>
	inline indexedData<T>::indexedData( uint64_t index, Args &&...args)
		: _data(std::move(args)...), _index(index)
	{
	}
	template<typename T>
	template<typename ...Args>
	inline indexedData<T>::indexedData( uint64_t index, Args &...args)
		: _data(std::move(args)...), _index(index)
	{
	}		
		
	template<typename ...Types>
	inline dataSynchronizer<Types...>::dataSynchronizer(size_t capacity)
	{
		_setAllCapacities(capacity);
	}


	template<typename ...Types>
	inline void dataSynchronizer<Types...>::setAllCapacities(size_t newCap)
	{
		std::lock_guard<std::mutex> lk(_dataM);
		_setAllCapacities(newCap);
	}

	template<typename ...Types>
	template<size_t I>
	inline void dataSynchronizer<Types...>::setCapacity(size_t newCap)
	{
		std::lock_guard<std::mutex> lk(_dataM);
		std::get<I>(_data).set_capacity(newCap);
	}

	template<typename ...Types>
	template<size_t I>
	inline void dataSynchronizer<Types...>::push_back(const indexedData<typename std::tuple_element<I,set_t>::type> &newIData)
	{
		std::lock_guard<std::mutex> lk(_dataM);
		std::get<I>(_data).push_back(newIData);
	}

	template<typename ...Types>
	template<size_t I>
	inline void dataSynchronizer<Types...>::push_back(indexedData<typename std::tuple_element<I,set_t>::type> &&newIData)
	{
		push_back<I>(newIData);
	}

	template<typename ...Types>
	inline typename dataSynchronizer<Types...>::indexedSet_t dataSynchronizer<Types...>::popLatestSynched()
	{
		std::lock_guard<std::mutex> lk(_dataM);
		auto Its = getIts();
		size_t oldestIndex;
		syncher sy(_data, oldestIndex);
		while(true)
		{
			oldestIndex = findOldestIndex(Its);
			if(sy.trySynch(Its))
				break;
		}
		indexedSet_t ret;
		returnSetMaker rsM(_data, Its, ret);
		rsM.make();
		return ret;
	}

	template<typename ...Types>
	inline void dataSynchronizer<Types...>::_setAllCapacities(size_t newCap)
	{
		capSetter cS(newCap);
		for_each_tuple_element(_data, cS);
	}



	template<typename ...Types>
	inline typename dataSynchronizer<Types...>::itColl_t dataSynchronizer<Types...>::getIts()
	{
		itColl_t Its;
		iTCreator_callback_t cb(Its);
		for_each_tuple_element_indexed(_data, cb);
		return Its;
	}

	template<typename ...Types>
	inline size_t dataSynchronizer<Types...>::findOldestIndex(itColl_t &Its)
	{
		size_t oi;
		oldestFinder of(oi);
		return oi;
	}
		
		
		
			
}

