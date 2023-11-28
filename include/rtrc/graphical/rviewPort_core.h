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
#include "decs/rviewPort_core_decs.h"
#include <vector>
#include <glm/glm.hpp>
#include "rvulkan.h"
#include "../dcsynch.h"
#include "../rinput.h"
#include <optional>
#include <glm/gtx/transform.hpp>
#include "../rstring.h"

namespace rtrc {
namespace layoutl
{



inline static bool hasNan(const glm::mat4x4 &m)
{
	for(int i = 0; i < 4; ++i)
	{
		for(int j = 0; j < 4; ++j)
			if(std::isnan(m[i][j])) return true;
	}
	return false;
}

inline static glm::mat3x3 getViewAngle(glm::mat4x4 &m)
{
	glm::mat3x3 ret{};
	for(size_t i = 0; i < 3; ++i)
		for(size_t j = 0; j < 3; ++j)
		{
			ret[i][j] = m[i][j];
		}
	return ret;
}

static inline void removeScale(glm::mat3x3 &R)
{
	float lx = glm::length(R*glm::vec3(1.0f,0.0f,0.0f));
	R[0] = lx != 0.0f ? R[0]/lx : glm::vec3(1.0f,0.0f,0.0f); // identity.
	float ly = glm::length(R*glm::vec3(0.0f,1.0f,0.0f));
	R[1] = ly != 0.0f ? R[1]/ly : glm::vec3(0.0f,1.0f,0.0f); // identity.
	float lz = glm::length(R*glm::vec3(0.0f,0.0f,1.0f));
	R[2] = lz != 0.0f ? R[2]/lz : glm::vec3(0.0f,0.0f,1.0f); // identity.
}

static inline void removeScale(glm::mat4x4 &R)
{
	auto retAngle = getViewAngle(R);
	removeScale(retAngle);
	for(size_t i = 0; i < 3; ++i)
		for(size_t j = 0; j < 3; ++j)
		{
			R[i][j] = retAngle[i][j];
		}
	R[0][3] = 0;
	R[1][3] = 0;
	R[2][3] = 0;
	R[3][3] = 1;
}

inline rtrc::layoutl::fullVp_view_data createDefaultViewData()
{
	rtrc::layoutl::fullVp_view_data vpData{};
	vpData.cam_coords.z = 10.0f;
	vpData.view_mat = glm::translate(-vpData.cam_coords);//glm::identity<glm::mat4x4>();
	return vpData;
}

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

static inline glm::mat4x4 getCameraView_PrevBased(const glm::vec3 &camPos, const glm::vec3 &camTarget, const glm::mat4x4 &prevV)
{
	if(camPos == camTarget) return prevV;
	glm::vec3 camVector = camPos - camTarget;
	
	glm::mat3x3 prevC_St_R{};
	prevC_St_R[0][0] = prevV[0][0];
	prevC_St_R[0][1] = prevV[0][1];
	prevC_St_R[0][2] = prevV[0][2];
	prevC_St_R[1][0] = prevV[1][0];
	prevC_St_R[1][1] = prevV[1][1];
	prevC_St_R[1][2] = prevV[1][2];
	prevC_St_R[2][0] = prevV[2][0];
	prevC_St_R[2][1] = prevV[2][1];
	prevC_St_R[2][2] = prevV[2][2];
	
	glm::mat3x3 prevSt_C_R = glm::transpose(prevC_St_R);
	
	glm::vec3 prevCamVector = prevSt_C_R * glm::vec3(.0f,.0f,1.0f);
	
	float camVecL = glm::length(camVector);
	float prevCamVecL = glm::length(prevCamVector);
	float ca = glm::dot(camVector,prevCamVector)/
	(camVecL*prevCamVecL);
	if(ca>1.0f) ca = 1.0f; // because of error
	if(ca<-1.0f) ca = -1.0f; // because of error
	float sa = glm::sqrt(1.0f-ca*ca); // 0 <= a <= pi
	float va = 1-ca;
	glm::mat3x3 C_St_R; // C_PrevC_R : rotation matrix by a radians around k.
	 
	if(ca == 1.0f)
	{
		C_St_R = prevC_St_R; // same direction, no rotation needed.
	}
	else if(ca == -1.0f)
	{
		C_St_R = prevC_St_R; // reverse Z direction, Z flip needed.
		C_St_R[0][2] *= -1;
		C_St_R[1][2] *= -1;
		C_St_R[2][2] *= -1;
	}
	else
	{
		glm::vec3 k = glm::cross(prevCamVector,camVector);
		float kl = glm::length(k);
		if(kl == 0.0f) 
		{
			C_St_R = prevC_St_R;
			goto gotoStep1;
		}
		k = k / kl;
		C_St_R[0][0] = k.x*k.x*va+ca;
		C_St_R[0][1] = k.x*k.y*va+k.z*sa;
		C_St_R[0][2] = k.x*k.z*va-k.y*sa;

		C_St_R[1][0] = k.x*k.y*va-k.z*sa;
		C_St_R[1][1] = k.y*k.y*va+ca;
		C_St_R[1][2] = k.y*k.z*va+k.x*sa;

		C_St_R[2][0] = k.x*k.z*va+k.y*sa;
		C_St_R[2][1] = k.y*k.z*va-k.x*sa;
		C_St_R[2][2] = k.z*k.z*va+ca;

		C_St_R = glm::transpose(C_St_R * prevSt_C_R); // C_St_R : rotate previous cam about k.
		
	}
gotoStep1:
		removeScale(C_St_R); // remove scale caused by calculation errors. otherwise the scaling caused by error by thousands of calls will add up.
	
	glm::vec3 C_St_Transfer = C_St_R*(-camPos);
	
	glm::mat4x4 C_St_T{};
	C_St_T[0][0] = C_St_R[0][0];
	C_St_T[0][1] = C_St_R[0][1];
	C_St_T[0][2] = C_St_R[0][2];
		
	C_St_T[1][0] = C_St_R[1][0];
	C_St_T[1][1] = C_St_R[1][1];
	C_St_T[1][2] = C_St_R[1][2];
	
	C_St_T[2][0] = C_St_R[2][0];
	C_St_T[2][1] = C_St_R[2][1];
	C_St_T[2][2] = C_St_R[2][2];
	
	C_St_T[3][0] = C_St_Transfer.x;
	C_St_T[3][1] = C_St_Transfer.y;
	C_St_T[3][2] = C_St_Transfer.z;
	
	C_St_T[3][3] = 1.0f;
	
	return C_St_T;
}

static inline void printMat(const glm::mat4x4 &m)
{
	printf("view mat :\n");
	for(int i = 0; i < 4; ++i)
	{
		printf("\t");
		for(int j = 0; j < 4; ++j)
			printf("%f \t",m[j][i]);
		printf("\n");
	}
}

static inline constexpr void reduce(char *&ptr, size_t &remain, size_t amount)
{
	if(amount > remain)
	{
		ptr += remain;
		remain = 0;
	}
	else
	{
		ptr += amount;
		remain -= amount;
	}
}

static inline void logMat(const char *name, const glm::mat4x4 &m)
{
	char temp[256];
	char *ptr = temp;
	size_t remain = 256;
	reduce(ptr, remain, snprintf(ptr, remain, "%s mat :\n", name));
	for(int i = 0; i < 4; ++i)
	{
		reduce(ptr, remain, snprintf(ptr, remain, "\t"));
		for(int j = 0; j < 4; ++j)
		{
			reduce(ptr, remain, snprintf(ptr, remain, "%f \t",m[j][i]));
		}
		reduce(ptr, remain, snprintf(ptr, remain, "\n"));
	}
}



static inline void RotateVp(fullVp_data &fvd, double x, double y, double angleFactor)
{
	if(x == 0 && y == 0)
	{
		return;
	}
	glm::vec3 kc((float)y,(float)-x,0.0f); // without projection (x,y,0)cross(0,0,-1)
	auto inverseAngle = getViewAngle(fvd.view.p_x_v_inverse);
	glm::vec3 k = inverseAngle * kc; // from projection to world.
	//printf("rotating with %f,%f about (%f,%f,%f)\n", x, y, k.x, k.y, k.z);
	float angle = glm::sqrt(x*x+y*y)/angleFactor;
	glm::vec4 camPos(fvd.view.cam_coords.x,fvd.view.cam_coords.y,fvd.view.cam_coords.z,1.0f);
	glm::vec4 newCamPos = glm::rotate(angle,k)*camPos;
	fvd.view.cam_coords = glm::vec3(newCamPos.x, newCamPos.y, newCamPos.z);
	auto axis = k/glm::length(k);
	fvd.view.view_mat = //glm::inverse(glm::rotate(glm::inverse(fvd.view.view_mat), -angle, k));
		glm::inverse(glm::rotate(-angle, axis)*glm::inverse(fvd.view.view_mat));
		//glm::rotate(-angle, glm::vec3(0.0f,0.0f,1.0f))*fvd.view.view_mat;
	removeScale(fvd.view.view_mat);
		//glm::rotate(-angle, k)*fvd.view.view_mat;
	fvd.view.p_x_v = fvd.view.projection_mat * fvd.view.view_mat;
	fvd.view.p_x_v_inverse = glm::inverse(fvd.view.p_x_v);
}

template<typename appInstanceT, typename dataSyncherT, size_t dataSyncherIndex>
inline depthCopier<appInstanceT,dataSyncherT,dataSyncherIndex>::depthCopier(const std::shared_ptr<appInstance_t> appInstance,
std::shared_ptr<dataSyncherT> dataSyncher)
	: _appInstance(appInstance), _buffer(_appInstance, 4, VK_SHARING_MODE_EXCLUSIVE,
	VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT|
VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT), _cmdPool(std::make_shared<vkl::rvkCmdPool>(
_appInstance, _appInstance->getGraphicsQueueIndex(), VK_COMMAND_POOL_CREATE_TRANSIENT_BIT|
VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT)), 
_cmdBuffers(std::make_unique<vkl::rvkCmdBuffers>(_appInstance, _cmdPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 0)),
_dataSyncher(dataSyncher),
_garbageCollector([this]()
{

	//_depthValues.set_capacity(20);
	std::vector<std::thread> oldQueue;
	while(1)
	{
		{
			std::lock_guard<std::mutex> qlk(_pQM);
			if(!_finishGc)
			{
				oldQueue = std::move(_pullQueue);
				_pullQueue.reserve(200);
			}
			else
				break;
		}
		for( auto &th : oldQueue )
		{
			th.join();
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	std::lock_guard<std::mutex> qlk(_pQM);
	oldQueue = std::move(_pullQueue);
	for( auto &th : oldQueue )
	{
		th.join();
	}

})
{
	_buffer.map();
}

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
template<typename appInstanceT, typename dataSyncherT, size_t dataSyncherIndex>
template<typename pipeSettings, typename appiT>
inline void depthCopier<appInstanceT,dataSyncherT,dataSyncherIndex>::
readPull(const std::shared_ptr<vkl::mainGraphicsPuller<pipeSettings, appiT>> &puller, int x, int y) noexcept
{
	//typedef typename pipeSettings::perPassBuffer_t perPassBuffer_t;
	//typedef vkl::renderPassLock<perPassBuffer_t,1> renderPassLock_t;
	using renderPassLock_t = typename vkl::mainGraphicsPuller<pipeSettings, appiT>::renderPassLock_t;
	try
	{
		mainLogger.logMutexLock("locking _iwM of depthCopier");
		std::unique_lock<std::mutex> iwlk(_iwM);
		if(!_isWaiting)
		{
			auto copyRequest = [this,x,y](std::vector<VkCommandBuffer> &renderCmds, std::vector<VkCommandBuffer> &postRcmds, renderPassLock_t &rPL)
			{
				size_t imageIndex = rPL.getImageIndex();
				mainLogger.logMutexLock("locking _cmdBuffersM of depthCopier");
				std::lock_guard<std::mutex> cmdBufferLk(this->_cmdBuffersM);
				this->_updateSwapchainIfNeeded(rPL);
				this->_recordCommandBuffer(x,y,imageIndex);
				postRcmds.push_back(this->_cmdBuffers->_cmdBuffers[imageIndex]);
			};
			_isWaiting = true;
			mainLogger.logMutexLock("unlocking _iwM of depthCopier");
			iwlk.unlock();
			indexedData_t id;
			try
			{
				id._index = puller->queueAndWaitPostRenderCopyRequest(0, copyRequest);

			}
			catch(rTimeoutException &err)
			{
				iwlk.lock();
				_isWaiting = false;
				iwlk.unlock();
				_iwCV.notify_all();
				return;
			}

			{
				// nothing else is locked
				//std::lock_guard<std::mutex> _lbvlk(_lbvm);
				mainLogger.logMutexLock("locking _scoptsM of depthCopier");
				std::lock_guard<std::mutex> _sclk(_scoptsM);

				uint32_t data;
				memcpy(&data, _buffer._pData, sizeof(data));
#ifndef NDEBUG
				if(!vkl::getDepthProcs[_lastScopts._depthAndStencilImageFormat])
				{

					throw graphicalError("unimplemented depth image format used");
				}		
#endif
				id._data.depth = vkl::getDepthProcs[_lastScopts._depthAndStencilImageFormat]
					(data);
				id._data.clickCoords.x = x;
				id._data.clickCoords.y = y;


				dataSyncher_t &ds = *_dataSyncher.get();
				_dataSyncher->template push_back<dSynchI>(id);
				mainLogger.logMutexLock("releasing _scoptsM of depthCopier");
			}
			mainLogger.logMutexLock("locking _iwM of depthCopier");
			iwlk.lock();
			_isWaiting = false;

			//std::lock_guard<std::mutex> pQLk(_pQM); // to prevent creating new threads before the easily joinable ones are joined.
			iwlk.unlock();
			_iwCV.notify_all();

		}
		mainLogger.logMutexLock("unlocking _iwM of depthCopier");
		/*if(_isWaiting)
		{
			_iwCV.wait(iwlk);
			return;
		}*/
	}
	catch(...)
	{
		printf("another thread thrown an exception. implement an exception handeling for multithreading");
		mainLogger.logText("another thread thrown an exception. implement an exception handeling for multithreading");
		exit(-1);
	}
}
template<typename appInstanceT, typename dataSyncherT, size_t dataSyncherIndex>
template<size_t dSI, typename grPullerT, typename depthCT>
inline void depthCopier<appInstanceT,dataSyncherT,dataSyncherIndex>::
	queueReadPull(const std::shared_ptr<depthCT> &dc,
const std::shared_ptr<grPullerT> &puller, int x, int y)
{
	try
	{
		mainLogger.logMutexLock("locking _pQM of depthCopier");
		std::lock_guard<std::mutex> _pQLk(dc->_pQM);
		std::thread th(
			[x, y](std::shared_ptr<depthCT> dc,
			std::shared_ptr<grPullerT> puller){
				try{
					dc->readPull(puller,x,y);
				}
				catch(...)
				{
					std::rethrow_exception(std::current_exception());
				}
				dc.reset();
				puller.reset();

			},
			std::shared_ptr<depthCT>(dc),
			std::shared_ptr<grPullerT>(puller));

		dc->_pullQueue.push_back(std::move(th));
		mainLogger.logMutexLock("unlocking _pQM of depthCopier");
	}
	catch(...)
	{
		std::rethrow_exception(std::current_exception());
	}
}

template<typename appInstanceT, typename dataSyncherT, size_t dataSyncherIndex>
inline depthCopier<appInstanceT,dataSyncherT,dataSyncherIndex>::~depthCopier()
{
	_cleanup();
}

template<typename appInstanceT, typename dataSyncherT, size_t dataSyncherIndex>
template<typename rPLT>
inline void depthCopier<appInstanceT,dataSyncherT,dataSyncherIndex>::
	_updateSwapchainIfNeeded(rPLT &rPL)
{
#ifndef NDEBUG
	if(!rPL.getLock().owns_lock())
		throw graphicalError("depthCopier proc called with unlocked lock");
#endif

	std::lock_guard<std::mutex> scoptslk(_scoptsM);
	if(_swapchainUpdateFeed != rPL.getScFeed())
	{
		_lastScopts = _appInstance->getLastSwapchainOptions();
		_recreate();
		_swapchainUpdateFeed = rPL.getScFeed();
	}
}



// lock _cmdBuffersM before calling this.
template<typename appInstanceT, typename dataSyncherT, size_t dataSyncherIndex>
inline void depthCopier<appInstanceT,dataSyncherT,dataSyncherIndex>::
	_recreate()
{
	_cmdPool->recreate();
	_cmdBuffers->recreate<false>(_cmdPool, _lastScopts._imageCount);
}

// only call when swapchain is locked.
template<typename appInstanceT, typename dataSyncherT, size_t dataSyncherIndex>
inline void depthCopier<appInstanceT,dataSyncherT,dataSyncherIndex>::
	_recordCommandBuffer(int x, int y, uint32_t imageIndex)
{
	const auto &depthImgs = _appInstance->getLastSwapchainDepthImgs();
	x = std::clamp<int>(x,0,depthImgs[imageIndex].getWidth());
	y = std::clamp<int>(y,0,depthImgs[imageIndex].getHeight());
	_cmdBuffers->beginI(imageIndex, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
	depthImgs[imageIndex].cmdCopyDepthToBuffer(_cmdBuffers->_cmdBuffers[imageIndex],
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, _buffer._hBuffer,
		x,y);
	_cmdBuffers->endI(imageIndex);
}

template<typename appInstanceT, typename dataSyncherT, size_t dataSyncherIndex>
inline void depthCopier<appInstanceT,dataSyncherT,dataSyncherIndex>::_cleanup()
{
	{
		std::lock_guard<std::mutex> qlk(_pQM);
		_finishGc = true;
	}
	_garbageCollector.join();
}

// this function should throw rNotYetException if it fails. it is thread safe.
// tries to update the world coordinates if the proper data is available.
// it is best to call this when attempting to call getLatestData().
inline void rpointer_core::tryUpdateWorldAndVpCoords()
{
	std::lock_guard<std::mutex> lk(_latest_full_data_M);
	onTryUpdateWorldAndVpCoords(_latest_full_data);
}

inline void rpointer_core::setLatestFullData(const std::optional<fullVp_data> &new_full_data)
{
	std::lock_guard<std::mutex> lk(_latest_full_data_M);
	_latest_full_data = new_full_data;
}

// queues a depth read. should not block. it is thread safe.
inline void rpointer_core::queueDepthRead()
{
	mainLogger.logMutexLock("locking _current_coords_M");
	std::lock_guard<std::mutex> lk(_current_coords_M);
	onQueueDepthRead(_viewX,_viewY);
	mainLogger.logMutexLock("unlocking _current_coords_M");
}

// updates the data that is going to be used by the next queueDepthRead() callS
inline void rpointer_core::updatePointer(double x, double y)
{
	std::lock_guard<std::mutex> lk(_current_coords_M);
	_viewX = x;
	_viewY = y;
}

// adds the view data used for a specific render to calculate world coordinates.
inline void rpointer_core::addViewData(const indexedData<fullVp_view_data> &viewData)
{
	std::lock_guard<std::mutex> lk(_latest_submitted_dataM);
	onAddViewData(viewData);
	_latest_submitted_data = viewData._data;
}

// gets the latest available data. it is thread safe.
inline auto rpointer_core::getLatestData() { 
	std::lock_guard<std::mutex> lk(_latest_full_data_M);
	if(_latest_full_data.has_value())
		return _latest_full_data.value(); 
	else
	{
		throw rNotYetException("there is no available view data, it is possible that rviewPort_core::pollEvents() needs to be called at least once");
	}
}

inline auto rpointer_core::getLatestSubmittedViewData()
{
	std::lock_guard<std::mutex> lk(_latest_submitted_dataM);
	if(_latest_submitted_data.has_value())
		return _latest_submitted_data.value();
	else
	{
		throw rNotYetException("no view data has been submitted, there is no available view data");
	}
}

//template<double angleFactor = 300.0>
inline void rpointer_core::RotateLatestView(double x, double y)
{
	std::lock_guard<std::mutex> lk(_latest_full_data_M);
	if(_latest_full_data.has_value())
		RotateVp(_latest_full_data.value(), x, y);
	else
	{
		throw invalidCallState("there is no available view data, it is possible that rviewPort_core::pollEvents() needs to be called at least once");
	}
}

inline void rpointer_core::discardQueue()
{
	onDiscardQueue();
}
	
template<typename appInstanceT, typename grPullerT>
rpointer_basic<appInstanceT,grPullerT>::
	rpointer_basic( const std::shared_ptr<appInstance_t> &appInstance, const std::shared_ptr<graphicsPuller_t> &grPuller )
	: _appInstance(appInstance), _depthC(std::make_shared<depthCopier_t>(appInstance, _dSyncher/*already initialized*/)), _grPuller(grPuller)
{}

template<typename appInstanceT, typename grPullerT>
inline void rpointer_basic<appInstanceT,grPullerT>::
	onQueueDepthRead(const double& x, const double& y)
{
	mainLogger.logMutexLock("locking _gm of rpointer_basic");
	std::unique_lock<std::mutex> lk(_gm);
	_queueDepthRead(x,y);
	mainLogger.logMutexLock("unlocking _gm of rpointer_basic");
}

template<typename appInstanceT, typename grPullerT>
inline void rpointer_basic<appInstanceT,grPullerT>::
	onTryUpdateWorldAndVpCoords(std::optional<fullVp_data> &latest_full_data)
{
	std::unique_lock<std::mutex> lk(_gm);
	_onTryUpdateWorldAndVpCoords(latest_full_data);
}

template<typename appInstanceT, typename grPullerT>
inline void rpointer_basic<appInstanceT,grPullerT>::
	onAddViewData(const indexedData<fullVp_view_data>& viewData)
{
	std::unique_lock<std::mutex> lk(_gm);
	_onAddViewData(viewData);
}

template<typename appInstanceT, typename grPullerT>
inline void rpointer_basic<appInstanceT,grPullerT>::
	onDiscardQueue()
{
	std::lock_guard<std::mutex> lk(_gm);
	_replaceQueueMembers();
}

template<typename appInstanceT, typename grPullerT>
inline void rpointer_basic<appInstanceT,grPullerT>::
	_queueDepthRead(const double &x,const double &y)
{
	depthCopier_t::template queueReadPull<depthDSynchI>(_depthC,_grPuller, x, y);
}

template<typename appInstanceT, typename grPullerT>
inline void rpointer_basic<appInstanceT,grPullerT>::
	_onTryUpdateWorldAndVpCoords(std::optional<fullVp_data> &latest_full_data)
{
	auto dSyncher = _dSyncher;
	//lk.unlock();
	dataSyncher_t::indexedSet_t synchedData = dSyncher->popLatestSynched();
	const viewPort_point &target = std::get<depthDSynchI>(synchedData._data);
	const fullVp_view_data &view_data = std::get<viewDSynchI>(synchedData._data);

	auto signedWidth = view_data.swapchainExtent.width/2; 
	auto signedHeight = view_data.swapchainExtent.height/2;

	auto normalizedX = (((float)target.clickCoords.x - signedWidth)/(float)signedWidth);
	auto normalizedY = (((float)target.clickCoords.y - signedHeight)/(float)signedHeight);
	glm::vec4 cursorLoc = view_data.p_x_v_inverse*glm::vec4(normalizedX, normalizedY, target.depth, 1.0f);
	if(!latest_full_data.has_value())
		latest_full_data = fullVp_data{};
	auto &lfd = latest_full_data.value();
	lfd.view = view_data;
	lfd.target.vp_coords = target;
	lfd.target.world_coords.x = cursorLoc.x;
	lfd.target.world_coords.y = cursorLoc.y;
	lfd.target.world_coords.z = cursorLoc.z;
}

template<typename appInstanceT, typename grPullerT>
inline void rpointer_basic<appInstanceT,grPullerT>::
	_onAddViewData(const indexedData<fullVp_view_data> &viewData)
{
	_dSyncher->template push_back<viewDSynchI>(viewData);
}

template<typename appInstanceT, typename grPullerT>
inline void rpointer_basic<appInstanceT,grPullerT>::
	_replaceQueueMembers()
{
	_dSyncher = std::make_shared<dataSyncher_t> (20);
	_depthC = std::make_shared<depthCopier_t>(_appInstance, _dSyncher);
}


inline void rviewPort_core::configureInputs()
{
	onConfigureInputs();
}

// polls input processing related events.
inline void rviewPort_core::pollEvents()
{
	onPollEvents();
}

inline void rviewPort_core::getMovePars(double &moveRight, double &moveUp)
{
	std::lock_guard<std::mutex> lk(_viewPortParsM);
	moveUp = _moveUpPar;
	moveRight = _moveRightPar;
}

inline void rviewPort_core::addViewData(size_t renderIndex)
{
	onAddViewData(renderIndex);
}

inline auto rviewPort_core::getLatestSubmittedViewData()
{
	return _pointers[0]->getLatestSubmittedViewData();
}

template<typename appInstanceT, typename grPullerT>
inline rviewPort_desktop<appInstanceT,grPullerT>::
	rviewPort_desktop( const std::shared_ptr<appInstance_t> &appInstance, const std::shared_ptr<graphicsPuller_t> &grPuller,
	bool useRawDrag,
	int width, int height, int xoffset, int yoffset)
	: _appInstance(appInstance),
	_mainDragFeature(_appInstance, typename dragFeature_t::dragCallback_t(),useRawDrag)
{
	_pointers.emplace_back(
		std::make_unique<rpointer_basic<appInstance_t,graphicsPuller_t>>(_appInstance, grPuller));
	_width = width;
	_height = height;
	_xoffset = xoffset;
	_yoffset = yoffset;


	_mainDragFeature.configure(*_appInstance->getWindow().get(), uinl::userInputOptions_core::buttonInputIndexes::mainPointer_button1,
		uinl::userInputOptions_core::ch2DInputIndexes::mainPointer);
}

template<typename appInstanceT, typename grPullerT>
inline void rviewPort_desktop<appInstanceT,grPullerT>::
	setAdditiveDragCallback(const dragCallback_t &cb)
{
	typename dragFeature_t::dragCallback_t newCb = [this,cb](std::array<double,2> startPos, 
		std::array<double,2> drag, uinl::rtrc_user_input_trigger_types trigger_type)
	{
		mainLogger.logIo("calling additive drag callback");
		if(trigger_type == 
			uinl::rtrc_user_input_trigger_types::rtrc_user_input_release)
			this->_pointers[0]->discardQueue();
		cb(*this, startPos,drag, trigger_type);
		mainLogger.logIo("additive drag callback called");
	};
	_mainDragFeature.setAdditiveCallback(newCb);
}

// unused
/*template<typename appInstanceT, typename grPullerT>
inline void rviewPort_desktop<appInstanceT,grPullerT>::
	addViewData(const indexedData<fullVp_view_data> &viewData)
{
	mainLogger.logMutexLock("caling addViewData");
	bool isDragging;
	mainLogger.logMutexLock("locking _mainDragStateM");
	std::lock_guard<std::mutex> lk(_mainDragStateM);
	auto dragLk = _mainDragFeature.getIsDragging(isDragging);
	if(!(_useMainDragForRotate&& isDragging))
	{
		_pointers[0]->addViewData(viewData);
	}
	mainLogger.logMutexLock("unlocking _gm of dragFeature_core in addViewData");
	mainLogger.logMutexLock("unlocking _mainDragStateM");
	// this call is thread safe
}*/

// this does not cause deadlock when called from a callback in dragCallback.
template<typename appInstanceT, typename grPullerT>
inline void rviewPort_desktop<appInstanceT,grPullerT>::
	pointers_RotateView(double x, double y)
{
	// these are commented to prevent deadlock.
	//std::lock_guard<std::mutex> lk(_mainDragStateM);
	//if(_useMainDragForRotate)
		_pointers[0]->RotateLatestView(x,y);
}

template<typename appInstanceT, typename grPullerT>
inline auto rviewPort_desktop<appInstanceT,grPullerT>::getLatestFullData()
{
	try
	{
		bool isDragging;
		std::lock_guard<std::mutex> lk(_mainDragStateM);
		auto dragLk = _mainDragFeature.getIsDragging(isDragging);
		if(!(_useMainDragForRotate&& isDragging))
			_pointers[0]->tryUpdateWorldAndVpCoords();
	}
	catch(rNotYetException &)
	{
		// do nothing
	}

	return _pointers[0]->getLatestData();
}

template<typename appInstanceT, typename grPullerT>
inline void rviewPort_desktop<appInstanceT, grPullerT>::onAddViewData
	(size_t renderIndex)
{
	/* steps :
	 * 1.specify viewport width and height
	 * 2.calculate projection matrix
	 * 3.calculate other data
	 * 4. add that data for the specified rendering index.
	 */
	
	using namespace rtrc::vkl;
	
	// specift view port width and height
	const swapchainProperties &scopts = _appInstance->getLastSwapchainOptions();
	int width = scopts._extent.width;
	int height = scopts._extent.height;
	float ratio = (float)width/height;
	
	// calculate projection matrix
	glm::mat4x4 p;
	glm::mat4x4 v = glm::identity<glm::mat4x4>();
	// prevent stretch, zoom out to fit instead.
	if(ratio < 1.0f)
	{
		p = glm::ortho(-1.0f,1.0f, 1.0f/ratio, -1.0f/ratio, 0.0f, 100.0f);
	}
	else
	{
		p = glm::ortho(-ratio, ratio, 1.0f, -1.0f, 0.0f, 100.0f);
	}
	
	//rtrc::layoutl::fullVp_data fd{};
	rtrc::indexedData<rtrc::layoutl::fullVp_view_data> vd{};
	
	{
		mainLogger.logMutexLock("caling addViewData");
		bool isDragging;
		mainLogger.logMutexLock("locking _mainDragStateM");
		std::lock_guard<std::mutex> lk(_mainDragStateM);
		// similar to getLatestFullData() call
		
		try
		{
			//fd = mainVP->getLatestFullData();
			vd._data = _pointers[0]->getLatestSubmittedViewData();
			//vd._data = fd.view;
		}
		catch(rtrc::rNotYetException &)
		{
			vd._data = createDefaultViewData();
		}
		vd._index = renderIndex;
		vd._data.p_x_v = p * vd._data.view_mat;
		vd._data.p_x_v_inverse = glm::inverse(vd._data.p_x_v);
		vd._data.projection_mat = p;
		vd._data.swapchainExtent.width = width;
		vd._data.swapchainExtent.height = height;

		auto dragLk = _mainDragFeature.getIsDragging(isDragging);
		if(!(_useMainDragForRotate&& isDragging))
		{
			_pointers[0]->addViewData(vd);
		}
		mainLogger.logMutexLock("unlocking _gm of dragFeature_core in addViewData");
		mainLogger.logMutexLock("unlocking _mainDragStateM");
	}
	// this call is thread safe
}

template<typename appInstanceT, typename grPullerT>
inline rviewPort_desktop<appInstanceT,grPullerT>::~rviewPort_desktop() {
	_mainDragFeature.cleanup();
}



template<typename appInstanceT, typename grPullerT>
inline void rviewPort_desktop<appInstanceT,grPullerT>::
	onConfigureInputs()
{
	_onConfigureInputs();
}

template<typename appInstanceT, typename grPullerT>
inline void rviewPort_desktop<appInstanceT,grPullerT>::onPollEvents()
{
	_onPollEvents();
}



template<typename appInstanceT, typename grPullerT>
inline auto & rviewPort_desktop<appInstanceT,grPullerT>::_getInputObj()
{
	return _appInstance->getWindow();
}

template<typename appInstanceT, typename grPullerT>
inline void rviewPort_desktop<appInstanceT,grPullerT>::_onPollEvents()
{
	mainLogger.logMutexLock("calling _onPollEvents");
	mainLogger.logMutexLock("locking _mainDragStateM");
	std::lock_guard<std::mutex> isRawlk(_mainDragStateM);
	bool isDragging{};
	auto isDraggingLk = _mainDragFeature.getIsDragging(isDragging);
	if(!(_useMainDragForRotate&&isDragging))
	{
		for(auto &pointer : _pointers)
		{
			// thread safe call.
			pointer->queueDepthRead();
		}
	}
	mainLogger.logMutexLock("unlocking _gm of dragFeature_core in _onPollEvents()");
	mainLogger.logMutexLock("unlocking _mainDragStateM");
}

template<typename appInstanceT, typename grPullerT>
inline void rviewPort_desktop<appInstanceT,grPullerT>::_onConfigureInputs()
{
	auto &inputObj = _getInputObj();
	// dev note : making a function to make all these calls with a single mutex lock is more performant.
	inputObj->setCallback_ch2D(this->_default_cursorCb, uinl::userInputOptions_core::ch2DInputIndexes::mainPointer);
	inputObj->setCallback_ch1D(this->_default_upCallback, uinl::userInputOptions_core::ch1DInputIndexes::moveUp);
	inputObj->setCallback_ch1D(this->_default_rightCallback, uinl::userInputOptions_core::ch1DInputIndexes::moveRight);
}



	
	
}
}

