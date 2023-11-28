struct capSetter
{
	capSetter(size_t &newCap)
		: _newCap(newCap)
	{

	}

	template<typename buffT>
	void operator()(buffT &buff)
	{
		buff.set_capacity(_newCap);
	}

	size_t &_newCap;
};


struct iTCreator_callback_t
{
	iTCreator_callback_t(itColl_t &iTs)
		: _iTs(iTs) {}

	template<size_t index, typename buffT>
	void operator()(std::integral_constant<size_t, index>, buffT &buff)
	{
		auto end = buff.end();
		if(!buff.empty()) --end;
		std::get<index>(_iTs) = end;
	}

	itColl_t &_iTs;
};

struct oldestFinder
{	
	oldestFinder(size_t &oldestIndex)
		: _oldestIndex(oldestIndex)
	{
		_oldestIndex = UINT64_MAX;
	}
	template<typename itT>
	void operator()(itT &it)
	{
		_oldestIndex = std::min(_oldestIndex, (*it)._index);
	}

	size_t &_oldestIndex;
};

struct syncher
{
	syncher(dataColl_t &dataColl, size_t &oldestIndex)
		: _dataColl(dataColl), _oldestIndex(oldestIndex)
	{

	}

	template<size_t index, typename itT>
	void operator()(std::integral_constant<size_t, index>, itT &it)
	{
		if(it == std::get<index>(_dataColl).end())
			throw rNotYetException("could not synch inputs");
		if((*it)._index > _oldestIndex)
		{
			if(it == std::get<index>(_dataColl).begin())
				throw rNotYetException("could not synch inputs");
			--it;
			_isSynched = false;
		}
		// oldestIndex is should be updated after each trySynch call.
		// since the oldestIndex amongst the Iterators that the function is being called on, then not being greater means being equal.
	}

	bool trySynch(itColl_t &itColl)
	{
		_isSynched = true;
		for_each_tuple_element_indexed(itColl, *this);
		return _isSynched;
	}

	bool _isSynched = false;
	const size_t &_oldestIndex;
	dataColl_t &_dataColl;
};

static constexpr auto &rSMdata_retriever = rtrc::get;
struct returnSetMaker
{
	returnSetMaker(dataColl_t &dataColl, itColl_t &Its, indexedSet_t &Is)
		: _dataColl(dataColl), _iTs(Its), _iS(Is)
	{

	}

	template<size_t index, typename ItT>
	void operator()(std::integral_constant<size_t, index>, ItT &It)
	{
		std::get<index>(_iS._data) = (*It)._data;
		if(index == 0)
			_iS._index = (*It)._index;
		std::get<index>(_dataColl).erase(std::get<index>(_dataColl).begin(),It);
	}

	void make()
	{
		rtrc::ctTFor_each<elCount-1>
			(rSMdata_retriever, *this, _iTs);

	}
	indexedSet_t &_iS;
	itColl_t &_iTs;
	dataColl_t &_dataColl;
};