/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   rviewPort_core.h
 * Author: roohi
 *
 * Created on October 16, 2019, 1:29 PM
 */
#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "../rvulkan.h"
#include "../../dcsynch.h"
#include "../../rinput.h"
#include <optional>
#include <glm/gtx/transform.hpp>
#include "../../rstring.h"

namespace rtrc {
namespace layoutl
{

struct viewPort_point
{
	glm::ivec2 clickCoords;
	double depth;
};	

struct fullVp_target_coords
{
	viewPort_point vp_coords;
	glm::vec3 world_coords;
	
};

struct fullVp_view_data
{
	glm::vec3 cam_coords;
	glm::mat4 view_mat;
	glm::mat4 projection_mat;
	glm::mat4 p_x_v;
	glm::mat4 p_x_v_inverse;
	VkExtent2D swapchainExtent;
};


struct fullVp_data
{
	fullVp_target_coords target;
	fullVp_view_data view;
};

static inline bool hasNan(const glm::mat4x4 &m);

static inline glm::mat3x3 getViewAngle(glm::mat4x4 &m);

static inline void removeScale(glm::mat3x3 &R);

static inline void removeScale(glm::mat4x4 &R);

static rtrc::layoutl::fullVp_view_data createDefaultViewData();

/*glm::mat4x4 getCameraView_Spherical(const glm::vec3 &camPos, const glm::vec3 &camTarget)
{
	//
	// note : this can be done more efficiently if needed
	//
	if(camPos == camTarget) return glm::identity<glm::mat4x4>();
	glm::vec3 camVector = camPos - camTarget;
	float flatSizePw2 = camVector.x * camVector.x + camVector.y * camVector.y;
	float flatSize = glm::sqrt(flatSizePw2);
	float flatSizeR = 1/flatSize;
	float fullSizeR = 1/glm::sqrt(flatSizePw2 + camVector.z * camVector.z);
	float cg = flatSize*fullSizeR;
	float sg = camVector.z *fullSizeR;
	
	float camDistance = glm::length(camVector);
	// defines default flat rotation
	float ca = flatSize != 0.f ? camVector.x * flatSizeR : 1;
	float sa = flatSize != 0.f ? camVector.y * flatSizeR : 0;
	
	glm::mat4x4 ret{};
	ret[0][0]= -sa;
	ret[0][1]= -ca*sg;
	ret[0][2]= ca*cg;
	//ret[0][3]= 0;
	
	ret[1][0] = ca;
	ret[1][1] = -sa*sg;
	ret[1][2] = sa*cg;
	//ret[1][3] = 0;
	
	//ret[2][0] = 0;
	ret[2][1] = cg;
	ret[2][2] = sg;
	// ...
	// ...
	ret[3][3] = 1;
	
	
	return ret*glm::translate(glm::vec3(-camPos.x,-camPos.y,-camPos.z));
}*/

static inline glm::mat4x4 getCameraView_PrevBased
	(const glm::vec3 &camPos, const glm::vec3 &camTarget, 
	const glm::mat4x4 &prevV = glm::identity<glm::mat4x4>());

static inline void printMat(const glm::mat4x4 &m);

static inline constexpr void reduce(char *&ptr, size_t &remain, size_t amount);

static inline void logMat(const char *name, const glm::mat4x4 &m);



static inline void RotateVp(fullVp_data &fvd, double x, double y, double angleFactor = 300.0);

template<typename appInstanceT, typename dataSyncherT, size_t dataSyncherIndex>
struct depthCopier
{
	depthCopier(const depthCopier &) = delete;
	depthCopier &operator=(const depthCopier &) = delete;
	depthCopier(depthCopier &&) = delete; // if implemented, all queue threads should be joined before moving.
	typedef appInstanceT appInstance_t;
	
	typedef indexedData<viewPort_point> indexedData_t;
	typedef dataSyncherT dataSyncher_t;

	static constexpr size_t dSynchI = dataSyncherIndex;

	depthCopier(const std::shared_ptr<appInstance_t> appInstance,
	std::shared_ptr<dataSyncherT> dataSyncher);
	
	/* this function orders a read pull for a single pixel, and waits for the
	 * completion of the whole queue operation related to that render and it's 
	 * post copies.
	 * it is thread safe.
	 * and new queue orders are discarded in this version.
	 * additional requests from another thread will return without doing anything.
	 * dev note :
	 * try updating the request location when discarding the new request if possible.
	 * try using vulkan events instead of waiting for the other operations to complete.
	 * 
	 * current usage case :
	 * .a read trigger in a new thread when a related mouse event is detected.
	 * .a frequent re requesting in each rendering loop( in a new thread) which 
	 *		is to update the depth value in the same location in each render pull.
	 *		dev note :
	 *		currently there is no guarantee that there will be 1 update request
	 *		for each pull since thread's "_isWaiting = false;" is not guaranteed
	 *		to execute before the render puller's "_selectNextFence()" 
	 *		
	 * 
	 */
	template<typename pipeSettings, typename appiT>
	void readPull(const std::shared_ptr<vkl::mainGraphicsPuller<pipeSettings, appiT>> &puller, int x, int y) noexcept;

	template<size_t dSI, typename grPullerT, typename depthCT>
	static void queueReadPull(const std::shared_ptr<depthCT> &dc,
	const std::shared_ptr<grPullerT> &puller, int x, int y);

	~depthCopier();

private:

	template<typename rPLT>
	void _updateSwapchainIfNeeded(rPLT &rPL);



	// lock _cmdBuffersM before calling this.
	void _recreate();

	// only call when swapchain is locked.
	void _recordCommandBuffer(int x, int y, uint32_t imageIndex);


	void _cleanup();

	/* lock order is :
	 * 1. _iwM
	 * 2. _pQM
	 * //3. _lbvm removed.
	 * 4. _cmdBuffersM
	 * 5. _scoptsM
	 */


	std::shared_ptr<appInstance_t> _appInstance;
	std::shared_ptr<dataSyncher_t> _dataSyncher;
	mutable std::mutex _scoptsM; // protects _swapchainUpdateFeed, _lastScopts
	std::shared_ptr<char> _swapchainUpdateFeed;
	vkl::swapchainProperties _lastScopts{};

	vkl::rvkBuffer _buffer; // implicitly accessed by one thread only at a time(either GPU or CPU).
	mutable std::mutex _iwM; // for _isWaiting and _wait
	std::condition_variable _iwCV; // for _isWaiting
	bool _wait = true; // for ignoring a call if _isWaiting is set.
	bool _isWaiting = false;
	size_t _depthValuesCountLimit = 20;
	mutable std::mutex _cmdBuffersM;
	std::shared_ptr<vkl::rvkCmdPool> _cmdPool;
	std::unique_ptr<vkl::rvkCmdBuffers> _cmdBuffers;

	mutable std::mutex _pQM; // for _pullQueue, _finishGc, _garbageCollector
	std::vector<std::thread> _pullQueue;
	bool _finishGc = false;
	std::thread _garbageCollector;
};

// defines an interface to matrix related data and the pointer data.
struct rpointer_core
{
	
	// this function should throw rNotYetException if it fails. it is thread safe.
	// tries to update the world coordinates if the proper data is available.
	// it is best to call this when attempting to call getLatestData().
	void tryUpdateWorldAndVpCoords();
	
	void setLatestFullData(const std::optional<fullVp_data> &new_full_data);
	
	// queues a depth read. should not block. it is thread safe.
	void queueDepthRead();
	
	// updates the data that is going to be used by the next queueDepthRead() callS
	void updatePointer(double x, double y);
	
	// adds the view data used for a specific render to calculate world coordinates.
	void addViewData(const indexedData<fullVp_view_data> &viewData);
	
	// gets the latest available data. it is thread safe.
	auto getLatestData();
	
	auto getLatestSubmittedViewData();
	
	//template<double angleFactor = 300.0>
	void RotateLatestView(double x, double y);
	
	void discardQueue();

protected:
	// adds a view data from an external perPassBuffer
	virtual void onAddViewData(const indexedData<fullVp_view_data> &viewData) = 0;
	// updates latest_full_data or throws rNotYetException
	virtual void onTryUpdateWorldAndVpCoords(std::optional<fullVp_data> &latest_full_data) = 0;
	// this function should not block.
	virtual void onQueueDepthRead(const double &x, const double &y) = 0;
	
	virtual void onDiscardQueue() = 0;
	
private:
	std::mutex _current_coords_M;
	// current coordinates.
	double _viewX = 0.0;
	double _viewY = 0.0;
	
	// latest retrieved view port coordinates.
	std::mutex _latest_submitted_dataM;
	std::optional<fullVp_view_data> _latest_submitted_data;
	std::mutex _latest_full_data_M;
	std::optional<fullVp_data> _latest_full_data;
	
	/* mutex locking order :
	 * same as declaration
	 */
	
};

template<typename appInstanceT, typename grPullerT>
struct rpointer_basic : rpointer_core
{
	typedef appInstanceT appInstance_t;
	typedef grPullerT graphicsPuller_t;
	typedef dataSynchronizer<viewPort_point, fullVp_view_data> dataSyncher_t;
	static constexpr size_t depthDSynchI = 0;
	static constexpr size_t viewDSynchI = 1;
	
	typedef depthCopier<appInstance_t, dataSyncher_t, depthDSynchI> depthCopier_t;
	
	rpointer_basic( const std::shared_ptr<appInstance_t> &appInstance, const std::shared_ptr<graphicsPuller_t> &grPuller );
	
protected:
	
	virtual void onQueueDepthRead(const double& x, const double& y) override;
	
	virtual void onTryUpdateWorldAndVpCoords(std::optional<fullVp_data> &latest_full_data) override;
	
	virtual void onAddViewData(const indexedData<fullVp_view_data>& viewData) override;
	
	virtual void onDiscardQueue() override;
	
private:
	
	void _queueDepthRead(const double &x,const double &y);
	
	void _onTryUpdateWorldAndVpCoords(std::optional<fullVp_data> &latest_full_data);
	
	void _onAddViewData(const indexedData<fullVp_view_data> &viewData);
	
	void _replaceQueueMembers();
	
	/* locking order :
	 * 1.base mutexes.
	 * 2._gm
	 * 3. member variable internal mutexes.
	 */
	
	std::shared_ptr<appInstance_t> _appInstance;
	std::mutex _gm; // used for _dSyncher and _depthC.
	std::shared_ptr<dataSyncher_t> _dSyncher = std::make_shared<dataSyncher_t> (20);
	std::shared_ptr<graphicsPuller_t> _grPuller;
	std::shared_ptr<depthCopier_t> _depthC;
};

struct rviewPort_core
{
	
	
	void configureInputs();
	
	// polls input processing related events.
	void pollEvents();
	
	void getMovePars(double &moveRight, double &moveUp);
	
	void addViewData(size_t renderIndex);
	
	// temporary implementation
	auto getLatestSubmittedViewData();
	
protected:
	std::vector<std::unique_ptr<rpointer_core>> _pointers;
	std::mutex _viewPortParsM;
	double _moveUpPar = 0.0;
	double _moveRightPar = 0.0;
	int _width = 0;
	int _height = 0;
	int _xoffset = 0;
	int _yoffset = 0;
	
	virtual void onConfigureInputs() = 0;
	virtual void onPollEvents() = 0;
	virtual void onAddViewData(size_t renderIndex) = 0;
};

/* all public functions are thread safe
 */
template<typename appInstanceT, typename grPullerT>
struct rviewPort_desktop : rviewPort_core
{

	typedef appInstanceT appInstance_t;
	typedef grPullerT graphicsPuller_t;
	typedef uinl::userInputOptions_core::channel2D_callback_t channel2D_callback_t;
	typedef uinl::userInputOptions_core::channel1D_callback_t channel1D_callback_t;
	typedef uinl::dragFeature_core<appInstance_t> dragFeature_t;
	typedef rviewPort_desktop<appInstanceT, grPullerT> this_type;
	typedef std::function<void(this_type &/* *this */, std::array<double,2> /*startPos*/, 
	std::array<double,2> /*drag*/, uinl::rtrc_user_input_trigger_types /*trigger type*/)> dragCallback_t;
	rviewPort_desktop( const std::shared_ptr<appInstance_t> &appInstance, const std::shared_ptr<graphicsPuller_t> &grPuller,
	bool useRawDrag = false,
	int width = 0, int height = 0, int xoffset = 0, int yoffset = 0);
	
	
	void setAdditiveDragCallback(const dragCallback_t &cb);
	
	// unused
	// adds matrix view data
	//void addViewData(const indexedData<fullVp_view_data> &viewData);
	
	// this does not cause deadlock when called from a callback in dragCallback.
	void pointers_RotateView(double x, double y);
	
	// gets the latest view data.
	auto getLatestFullData();
	
	virtual void onAddViewData(size_t renderIndex) override;
	
	virtual ~rviewPort_desktop();
	
protected:
	
	virtual void onConfigureInputs() override;
	
	virtual void onPollEvents() override;
	
private:
	
	auto &_getInputObj();
	
	void _onPollEvents();
	
	void _onConfigureInputs();
	
	channel2D_callback_t _default_cursorCb = [this](std::array<double,2> pos, std::array<double,2>, double time,
		uinl::rtrc_channel_modes channel_mode)
	{
		mainLogger.logMutexLock("calling _default_cursorCb");
		double xoffset{};
		double yoffset{};
		std::lock_guard<std::mutex> isRlk(this->_mainDragStateM);
		this->_pointerIsRaw = channel_mode == uinl::rtrc_channel_modes::rtrc_channel_mode_raw;
		{
			std::lock_guard<std::mutex> vplk(this->_viewPortParsM);
			xoffset = this->_xoffset;
			yoffset = this->_yoffset;
		}
		bool isDragging;
		auto dragFeatureLk = this->_mainDragFeature.getIsDragging(isDragging);
		if(!(this->_useMainDragForRotate&&isDragging))
			// updates the location for the
			this->_pointers[0]->updatePointer(pos[0] - xoffset,pos[1] - yoffset);
		mainLogger.logMutexLock("unlocking _gm of dragFeature_core in _default_cursorCb");
	};
	
	channel1D_callback_t _default_upCallback = [this](double y, double speed, double time, 
	uinl::rtrc_channel_modes channel_mode)
	{
		std::lock_guard<std::mutex> vplk(this->_viewPortParsM);
		this->_moveUpPar = y;
	};
	
	channel1D_callback_t _default_rightCallback = [this](double x, double speed, double time,
	uinl::rtrc_channel_modes channel_mode)
	{
		std::lock_guard<std::mutex> vplk(this->_viewPortParsM);
		this->_moveRightPar = x;
	};

	/* locking order: 
	 * 1. _mainDragStateM 
	 * 2. base mutexes.
	 * 3. _mainDragFeature functions. 
	 * 4. _pointers functions
	 * // this allows the dependence of _pointers calls to getIsDragging.
	 * // it also allows the call of pointer_XXX functions inside the drag 
	 * // callback without a deadlock.
	 * 
	 */
	
	std::shared_ptr<appInstance_t> _appInstance;
	dragFeature_t _mainDragFeature;
	std::mutex _mainDragStateM; // for _pointerIsRaw, _isDragging and _useMainDragForRotate
	bool _pointerIsRaw{};
	bool _useMainDragForRotate = true;
};
}
}

