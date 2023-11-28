
#pragma once
#include "rvulkan.h"
#include "../dcsynch.h"
#include <type_traits>
#include <tuple>
#include <utility>
#include <functional>
namespace rtrc {
namespace vkl
{
	
	/* do not destroy rvkLiveResource before the pipeline is no 
	 * longer using it(it's usually used through perPassBuffer). it is 
	 * recommended to use commonPipeSettings to do this management.
	 * 
	 * rkvLiveResource :
	 * use for a circular resource to load live data to without synchronization 
	 * issues. a max_length of 3 is recommended.
	 */
	template<typename rT, size_t l>
	struct rvkLiveResource
	{
		
		typedef rT resource_t;
		typedef std::pair<indexedData<std::shared_ptr<const rvkFence>>, resource_t> resourcePair_t;
		static constexpr size_t max_length = l;
		static_assert(max_length > 0, "a max_length of 0 is meaningless");
		
		template<typename ...Args>
		rvkLiveResource( const Args &...args // arguments to default initialize the resource.
			)
		{
			_initResources(args...);
		}
		
		template<typename writeCallableT> // callable with resource_t &
		void addResource_initial(const writeCallableT &writeCallable)
		{
			std::lock_guard<std::mutex> lk(_gm);
			if(_newestI != -1 || _oldestI != -1)
			{
				throw invalidCallState("addResrouce_initial should be called once only");
			}
			_newestI = 0;
			writeCallable(_resourcePairs[0].second);
		}
		
		
		/* loads data into a resource slot that is no longer in use by the renderer(make sure render indexes are incremental)
		 * 
		 * read the description of callOnLastResource for synchronization.
		 * 
		 * warning : don't call this inside a perPassBuffer, it requires the 
		 * graphics puller's mutex lock and may cause a deadlock. although a 
		 * specific version can be implemented that takes a fence directly for 
		 * that purpose, it is not needed yet.
		 * 
		 * writeCallable : should be callable with resource_t & and write the data
		 * tryOverrideCallable : should be a callable with resource_t & and 
		 * return a bool representing the success of the attempt.
		 * 
		 */
		template<typename repT, typename periodT, typename callableT1, typename callableT2, typename grPullerT>
		void addResource(const grPullerT &grPuller
			, const std::chrono::duration<repT,periodT> &timeoutDuration
			, const callableT1 &tryOverrideCallable, const callableT2 &writeCallable)
		{
			// unlocking order matters to prevent new rendering pulls before 
			// next calls like callOnLastResource
			std::unique_lock<std::mutex> fenceLk; // unlocked second by destructor.
			std::lock_guard<std::mutex> lk(_gm); // unlocked first by destructor.
			// the current _newestI will become an outdated one.
			if(_newestI >= 0 && tryOverrideCallable(_resourcePairs[_newestI].second))
			{
				// host override successful, no change in the iterators is needed.
				return;
			}
			else
			{
				auto oldNewestI = _newestI;

				// incrementing _newestI and _oldestI
				_selectNext(timeoutDuration);
				// calling load command that load new data into the resource.(it takes some time to execute)
				writeCallable(_resourcePairs[_newestI].second); 

				if(oldNewestI >= 0) // this is a duplicate >= 0 check!.
				{
					grPuller.getCurrentFence(_resourcePairs[oldNewestI].first._data, fenceLk);
					auto submitIndexOp = _resourcePairs[oldNewestI].first._data->getLastSubmitIndex();
					_resourcePairs[oldNewestI].first._index = submitIndexOp.has_value() ? submitIndexOp.value() : 0;
				}
			}
			
		}
		
		/* recommended for vulkan command records and access to staging info.
		 * note that staging synchronization methods are different for each 
		 * resource. 
		 * 1.use of host coherent resource memory(not recommended)
		 * 2.one may include a single semaphore and require sequential rendering(not recommended).
		 * 3.use a memory barrier(recommended)
		 */
		template<typename callableT> // callableT should be callable with resource_t &
		void callOnLastResource(const callableT &callable)
		{
			std::lock_guard<std::mutex> lk(_gm);
			if(_newestI < 0)
			{
				throw rNotYetException("no available resource, try again later");
			}
			callable(_resourcePairs[_newestI].second);
		}
		
	private:
		
		template<typename ...Args>
		void _initResources(const Args &... args)
		{
			_resourcePairs.reserve(max_length);
			for(size_t i = 0; i < max_length; ++i)
			{
				_resourcePairs.emplace_back(std::piecewise_construct_t{},
					std::tuple<>{} /*shared_ptr args*/,
					std::tuple<std::reference_wrapper<const Args>...>(args...));
			}
		}
		
		
		// first, tries to free the old resources. waits if none is free.
		template<typename repT, typename periodT>
		void _selectNext(const std::chrono::duration<repT,periodT> &timeoutDuration)
#ifdef NDEBUG
			noexcept
#endif
		{
#ifndef NDEBUG
			if(_newestI >= 0 && _oldestI == _newestI) // if there is old resources.
				throw RbasicException("_oldestI == _newestI, fix the implementation of _selectNext()");
#endif
			_newestI = ++_newestI%max_length;
			if(_oldestI == _newestI)
			{
				// waitForFinish will see if the renderIndex has been passed before, if not waits for the operation completion  
				_resourcePairs[_oldestI].first._data->waitForFinish(_resourcePairs[_oldestI].first._index, timeoutDuration); // must wait.
				_oldestI = ++_oldestI % max_length;
			}
			/*while(_oldestI >= 0 /*if there is anything left to free*//*)
			{
				if(_resourcePairs[_oldestI].first._data->isFinished(_resourcePairs[_oldestI].first._index))
					if(_oldestI == _newestI)
					{
						_oldestI = -1;
						break; // for better performance.
					}
					else
						_oldestI = ++_oldestI % max_length; // there is more left to free.
				else
					break; // try another time, resources are busy.
			}*/
			
		}
		
		// general purpose mutex to protect all variables.
		mutable std::mutex _gm;
		
		std::vector<resourcePair_t> _resourcePairs;
		// if _oldestI == next _newestI, the buffer is full
		// _oldestI == _newestI is not allowed in the current version.
		// if _oldestI < 0, there is no old resources.
		// if _newestI < 0, there is no valid resources.
		int _oldestI = -1; 
		int _newestI = -1;
	};
}
}
