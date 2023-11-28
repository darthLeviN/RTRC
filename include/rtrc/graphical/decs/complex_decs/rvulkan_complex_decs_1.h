private:
/* ut us responsible to create a renderPassLock by forwarding the parsed 
 * pipeline arguments at compile time to a beginRenderRecording call.
 */
template<typename flusher_t>
struct _rplCreator_t
{
	_rplCreator_t(grPipesTuple_t &grPipesTuple, VkSemaphore &avs, VkFence kf, VkSubpassContents con, flusher_t fl)
		: _grPipesTuple(grPipesTuple), _iavs(avs), _khrFence(kf), _contents(con), _flusher(fl)
	{}
	template<typename ...Args>
	auto operator()(const Args &...args)
	{
		return std::get<0>(_grPipesTuple)->beginRenderRecording(
			_iavs, _khrFence, _contents, _flusher, args... /*other graphics pipelines as shared ptrs */);
	}
	grPipesTuple_t &_grPipesTuple;
	VkSemaphore &_iavs;
	VkFence _khrFence;
	VkSubpassContents _contents;
	flusher_t _flusher;
};

/* Creates an array of VkSubmitInfo vectors that later get processed and 
 * chained up together for submitting to the graphics queue.
 */

struct _submitInfosCreator_t
{
	renderPassLock_t &_rpL;
	const std::tuple<perPassWriteCmd_t<grPipeGroupTs>...> &_perPassWrites;
	uint64_t &_renderIndex;
	const std::shared_ptr<rvkFence> &_prevAllRF;
	const std::shared_ptr<rvkFence> &_allRF;
	std::vector<VkSemaphore> &_reservedSems;
	std::array<std::vector<VkPipelineStageFlags>,pipeGroupCount> &_renderWaitFlags;
	std::array<std::vector<VkSemaphore>, pipeGroupCount> &_renderWaitSems;

	// creates one VkSubmitInfo vector related to the pipe index.
	template<size_t pipeIndex>
	std::vector<VkSubmitInfo> getOne()
	{
		return _rpL.template getPerPassBuffer<pipeIndex>()->cmdWrite(
			std::get<pipeIndex>(_perPassWrites), _renderIndex, _prevAllRF, _allRF,
			_reservedSems, _renderWaitFlags[pipeIndex], _renderWaitSems[pipeIndex]);
	}

	// creates an array of VkSubmitInfo vectors related to the indexes in
	// ct_size_sequence_type with a similar order.
	template<size_t ...ns>
	std::array<std::vector<VkSubmitInfo>,sizeof...(ns)> operator()(ct_size_sequence_type<ns...>)
	{
		return { getOne<ns>()... };
	}
};

/* handles calling render draw callables from inside the tuple that holds
 * the draw callables related to each pipeline with the same index.
 */
template<typename cmdCallablesTupleT>
struct _drawPuller_t
{
	_drawPuller_t(renderPassLock_t &rpL, const cmdCallablesTupleT &drawPulls)
		: _rpL(rpL), _drawPulls(drawPulls)
	{

	}
	renderPassLock_t &_rpL;
	const cmdCallablesTupleT &_drawPulls;

	// call the callable related to pipeIndex
	template<size_t pipeIndex>
	int pullOne()
	{
		std::get<pipeIndex>(_drawPulls)(_rpL.template getCmdBuffer<pipeIndex>(), *_rpL.template getPerPassBuffer<pipeIndex>().get(),
			_rpL.getImageIndex());
		return 0; // dummy return value. should get discarded.
	}

	/* calls the callables related to the pipeIndexes in 
	 * ct_size_sequence_type with the same order. although this order is not
	 * important. because these calls are only meant to record commands and
	 * do nothing else.
	 */
	template<size_t ...ns>
	int operator()(ct_size_sequence_type<ns...>)
	{
		//dummyUnpackerStruct(pullOne<ns>()...);
		dummyUnpack(pullOne<ns>()...);
		return 0;
	}
};

