/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   rcircular_buffer.h
 * Author: roohi
 *
 * Created on October 13, 2019, 7:52 PM
 */
#pragma once
#include <boost/circular_buffer.hpp>
#include "../rmemory.h"
#include <iterator>
#include <memory>
#include <new>
namespace rtrc
{ namespace coll
{
	template<typename T>
	struct circularIterator : std::iterator<std::bidirectional_iterator_tag, T>
	{
		
		bool operator==(circularIterator &other) const noexcept
		{
			return _cur == other._cur;
		}
		bool operator==(circularIterator other) const noexcept
		{
			return _cur == other._cur;
		}
		bool operator!=(circularIterator &other) const noexcept
		{
			return _cur != other._cur;
		}
		
		bool operator!=(circularIterator other) const noexcept
		{
			return _cur != other._cur;
		}
		
		reference operator*() const noexcept
		{
			return *_data[_cur];
		}
		
		
		
		// pre-increment
		circularIterator &operator++() noexcept
		{
			_cur+= sizeof(T);
			_cur %= _size;
			return *this;
		}
		
		circularIterator operator++(int) noexcept
		{
			auto tmpCur = _cur;
			_cur += sizeof(T);
			_cur %= _size;
			return circularIterator{_data,_begin,tmpCur,_size};
		}
		
		// ore-decrement
		circularIterator &operator--() noexcept
		{
			_cur += _size;
			_cur -= sizeof(T);
			_cur %= _size;
			return *this;
		}
		
		circularIterator operator--(int) noexcept
		{
			auto tmpCur = _cur;
			_cur += _size;
			_cur -= sizeof(T);
			_cur %= _size;
			
			return circularIterator{_data,_begin,tmpCur,_size};
		}
		
		circularIterator begin() const noexcept
		{
			return circularIterator{_data,_begin,_begin-_data,_size};
		}

		circularIterator end() const noexcept
		{
			return begin();
		}
		
		char *_data;
		char *_begin; // begin and end are the same in a circular buffer
		size_t _cur; // raw offset from _data
		size_t _size; // raw size
	};
	
	enum circularFillStrategy
	{
		forward_older, // ++ operator on iterator gives older elements.
		forward_newer, // ++ operator on iterator gives newer elements
	};
	
	template<typename T, circularFillStrategy Stra = forward_newer>
	struct rcircular_buffer
	{
		typedef T value_type;
		static constexpr auto strategy = Stra;
		typedef memoryl::replacer<value_type> replacer_t;
		
		rcircular_buffer(const rcircular_buffer &other)
			: _allValid(other._allValid), _elCunt(other._elCount)
		{
			_mainIt._size = other._mainIt._size;
			_mainIt._cur = other._mainIt._cur;
			_mainIt._data = new char[other._mainIt._size];
			_mainIt._begin = _mainIt._data + (other._mainIt._begin - other._mainIt._data);
			
			
			// don't call destructor, storage is uninitialized.
			replacer_t::replace<false>(_mainIt._begin, other._mainIt,
				memoryl::copySize<memoryl::size_types::raw> {other._mainIt._cur});
		}
		
		void setSize(size_t newCount) noexcept
		{
			if(newCount > _elCount)
			{
				auto oldIt = _mainIt;
				//auto oldIt2 = oldIt;
				_mainIt._begin = new char[newCount*sizeof(value_type)];
				_mainIt._size = newCount * sizeof(value_type);
				if(_allValid)
				{
					if(strategy == forward_newer)
					{
						++oldIt;
						// remain copy size
						memoryl::copySize<memoryl::size_types::raw> rs{oldIt._cur};
						// first copy size
						memoryl::copySize<memoryl::size_types::raw> cs{oldIt._size - oldIt._cur};
						replacer_t::replace<false>((value_type *)_mainIt._data, (value_type *)&*oldIt, cs);
						replacer_t::replace<false>((value_type *)(_mainIt._data + cs.size), 
							(value_type *)oldIt._data, rs);
					}
					else
					{
						
					}
				}
				else
				{
					//memoryl::copySize<memoryl::size_types::raw> cs{oldIt._size};
					//	replacer_t::replace<false>((value_type *)_mainIt._data, 
					//		(value_type *)oldIt._data, cs);
				}
			}
			else if(newCount < _elCount)
			{
				auto oldIt = _mainIt;
				auto remainCap = newCount;
			}
		}
		
		
	private:
		
		
		circularIterator<T> _mainIt;
		size_t _elCount;
		bool _allValid; // if it is false then the access is linear, with the first element at index 0.
	};
}


}

