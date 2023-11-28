
#pragma once
#include <array>
#include <type_traits>
#include "rfunctional.h"
namespace rtrc
{
	
	
	template<typename intType>
	constexpr intType ctsum()
	{
		return 0;
	}
	
	template<typename firstT, typename ...Args>
	constexpr auto ctsum(firstT first, Args ...args)
	{
		return first + ctsum<firstT>(args...);
	}
	
	template<typename valT>
	constexpr void ctArgCopyToArray(valT *ptr){}
	
	template<typename valT, typename firstT, typename ...Args>
	constexpr void ctArgCopyToArray(valT *ptr, firstT first, Args ... args)
	{
		*ptr = first;
		++ptr;
		ctArgCopyToArray(ptr,args...);
	}
	
	template<typename intT>
	constexpr std::array<intT,0> ctArgArray()
	{
		return std::array<intT,0>{};
	}
	
	template<typename firstT, typename ...Args>
	constexpr std::array<firstT,sizeof...(Args)+1> ctArgArray(firstT first, Args ...args)
	{
		const size_t size = sizeof...(Args)+1;
		std::array<firstT,size> retAr{};
		retAr[0] = first;
		ctArgCopyToArray(retAr.data()+1,args...);
		return retAr;
	}
	
	
	/* an alternative to boost::fusion::for_each that also gives the index to the callable.
	 * callable form :
	 * struct callable
	 * {
	 *		template<size_t index, typename T>
	 *		void operator()(std::integral_constant<size_t,index>, T &subData);
	 * };
	 * data_retriever form:
	 * struct data_retriever
	 * {
	 *		template<size_t index>
	 *		auto &operator()(std::integral_constant<size_t,index>, T &data);
	 * };
	 */
	template<size_t index, typename data_retriever_t,
		typename callable_t, typename T>
	static inline 
		typename std::enable_if<index != 0,void>::type ctTFor_each(data_retriever_t &data_retriever, callable_t &callable, T &data)
	{
		callable(std::integral_constant<size_t,index>(), 
				data_retriever(std::integral_constant<size_t,index>(), data));
		ctTFor_each<index-1,data_retriever_t,callable_t,T>(data_retriever,
			callable, data);
	}
	
	template<size_t index, typename data_retriever_t,
		typename callable_t, typename T>
	static inline 
		typename std::enable_if<index == 0,void>::type ctTFor_each(data_retriever_t &data_retriever, callable_t &callable, T &data)
	{
		callable(std::integral_constant<size_t,index>(), 
			data_retriever(std::integral_constant<size_t,index>(), data));
	}
	
	
	
	template<typename tupleT, typename F, size_t Index = std::tuple_size<tupleT>::value - 1>
	typename std::enable_if<Index != 0,void>::type for_each_tuple_element(tupleT &tup,
		F &functor)
	{
		functor(std::get<Index>(tup));
		for_each_tuple_element<tupleT,F,Index-1>(tup,functor);
	}
	
	
	template<typename tupleT, typename F, size_t Index = std::tuple_size<tupleT>::value - 1>
	typename std::enable_if<Index == 0,void>::type for_each_tuple_element(tupleT &tup,
		F &functor)
	{
		functor(std::get<0>(tup));
	}
	
	template<typename tupleT, typename F, size_t Index = std::tuple_size<tupleT>::value - 1>
	typename std::enable_if<Index != 0,void>::type for_each_tuple_element_indexed(tupleT &tup,
		F &functor)
	{
		functor(std::integral_constant<size_t,Index>(), std::get<Index>(tup));
		for_each_tuple_element_indexed<tupleT,F,Index-1>(tup,functor);
	}
	
	template<typename tupleT, typename F, size_t Index = std::tuple_size<tupleT>::value - 1>
	typename std::enable_if<Index == 0,void>::type for_each_tuple_element_indexed(tupleT &tup,
		F &functor)
	{
		functor(std::integral_constant<size_t,0>(), std::get<Index>(tup));
	}
	
	
	static constexpr struct
	{
		template<size_t index, typename T>
		auto &operator()(std::integral_constant<size_t,index>, T &d) const
		{
			return std::get<index>(d);
		}
	} get;
	
	template<size_t ...ns>
	struct ct_size_sequence_type
	{
	};
	
	struct dummyUnpackerStruct
	{
		template<typename ...Args>
		dummyUnpackerStruct(const Args &...)
		{
		}
	};

	template<typename ...Args>
	static inline void dummyUnpack(const Args&...)
	{

	}
	
	template<typename F, typename ...Args, size_t ...ns>
	void ct_size_sequence_type_per_arg_invoke(const F &functor, Args ...args, const ct_size_sequence_type<ns...> &)
	{
		std::tuple<Args ...> tt { args ... };
		dummyUnpackerStruct(functor(std::get<ns>(tt))...);
	}
	
	template<typename F, typename tupleT, size_t ...ns>
	auto ct_size_sequence_type_invoke(F &functor, tupleT &tup, const ct_size_sequence_type<ns...> &)
	{
		//std::tuple<Args ...> tt { args ... };
		return functor(std::get<ns>(tup)...);
	}
	
	template<size_t index, typename T>
	using indexedRef = std::pair<std::integral_constant<size_t, index>, T &>;
	
	template<size_t index, typename TupleT>
	using indexed_tuple_element_ref = indexedRef<index, typename std::tuple_element<index,TupleT>::type>;
	
	template<size_t index, typename TupleT>
	indexed_tuple_element_ref<index, TupleT> getIndexed(TupleT &tuple)
	{
		return {{}, std::get<index>(tuple)};
	}
	
	template<typename F, typename tupleT, size_t ...ns>
	auto ct_size_sequence_type_invoke_indexed(F &functor, tupleT &tup, const ct_size_sequence_type<ns...> &)
	{
		//std::tuple<Args ...> tt { args ... };
		
		return functor(getIndexed<ns>(tup)...);
	}
	
	template<size_t appendD, size_t ...ns>
	constexpr ct_size_sequence_type<appendD,ns...> ct_append_size_sequence_type(const ct_size_sequence_type<ns...> &)
	{
		return {};
	}
	
	// N sequential numbers, starting from f.
	template<size_t f, size_t N>
	struct ct_size_series_type : decltype(ct_append_size_sequence_type<f+N>(ct_size_series_type<f, N-1>()))
	{
	};
	
	template<size_t f>
	struct ct_size_series_type<f, 1> : ct_size_sequence_type<f>
	{
		
	};
	
	template<size_t f>
	struct ct_size_series_type<f, 0> : ct_size_sequence_type<>
	{
		
	};
	
	template<size_t firstI, size_t endI, typename F, typename tupleT>
	auto partialUnpack(F &functor, tupleT &tup)
	{
		static_assert(firstI <= endI,"bad index");
		
		return ct_size_sequence_type_invoke(functor, tup, ct_size_series_type<firstI, endI - firstI>());
	}
	
	template<typename F, typename tupleT>
	auto fullUnpack_indexed(F &functor, tupleT &tup)
	{
		return partialUnpack<0,std::tuple_size<tupleT>>(functor, tup);
	}
	
//#define ct_rtrc_expand_tuple()
	
}