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
#include "../compiletime.h"
#include <algorithm>
#include "../rexception.h"
#include <boost/circular_buffer.hpp>
namespace rtrc {
	
	template<typename T>
	struct indexedData
	{
		static_assert(std::is_copy_constructible<T>::value);
		template<typename ...Args>
		indexedData( uint64_t index, Args &&...args);
		
		template<typename ...Args>
		indexedData( uint64_t index, Args &...args);
		
		indexedData() = default;
		
		T _data;
		uint64_t _index;
	};
	
	/* developer note : the current version of boost::circular_buffer does not 
	 * support emplace_back, this is fine for trivially copyable data but for
	 * other types of data, a circular buffer with emplace feature is preferred.
	 * 
	 * guide :
	 * 1.insert data with push_back ( pass a shared_ptr to the objects that want to insert data ).
	 *	data insertion is thread safe and can be called from multiple threads.
	 * 2.use popLatestSynched() to see if data is available. throws an exception is fails.
	 *	low failure rate means more performance.
	 *	dev note : design a non exception based one for a high rate of failure.
	 * 
	 */
	template<typename ...Types>
	struct dataSynchronizer
	{
		static constexpr size_t elCount = sizeof...(Types);
		typedef std::tuple<boost::circular_buffer<indexedData<Types>>...> dataColl_t;
		typedef std::tuple<Types...> set_t;
		typedef indexedData<set_t> indexedSet_t;
		typedef std::tuple<Types *...> internal_indexetReturn_t;
		template<typename T>
		using iteratorOfCB = typename boost::circular_buffer<T>::iterator;
		typedef std::tuple<iteratorOfCB<indexedData<Types>> ...> itColl_t;
		
		
		dataSynchronizer(size_t capacity);
		
		void setAllCapacities(size_t newCap);
		
		template<size_t I>
		void setCapacity(size_t newCap);
		
		
		template<size_t I>
		void push_back(const indexedData<typename std::tuple_element<I,set_t>::type> &newIData);
		
		template<size_t I>
		void push_back(indexedData<typename std::tuple_element<I,set_t>::type> &&newIData);
		
		indexedSet_t popLatestSynched();
		
	private:
		
		void _setAllCapacities(size_t newCap);
		
		static constexpr auto &iTdata_retriever = rtrc::get;
		
		itColl_t getIts();
		
		size_t findOldestIndex(itColl_t &Its);
		
#include "complex_decs/dcsynch_complex_decs.h"
		
		mutable std::mutex _dataM;
		dataColl_t _data;
	};
	
}

