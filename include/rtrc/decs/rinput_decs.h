
#pragma once
#include <functional>
#include <optional>
#include <mutex>
#include <type_traits>
#include <array>
#include "../rtrc.h"
#include "../rexception.h"

namespace rtrc
{
namespace uinl
{
	
enum rtrc_user_input_trigger_types
{
	rtrc_user_input_undefined = 0,
	rtrc_user_input_press,
	rtrc_user_input_hold, // needs multithreading.
	rtrc_user_input_release,
};

enum rtrc_user_input_value_types
{
	rtrc_user_input_type_raw = 0,
	rtrc_user_input_type_press_state,
	rtrc_user_input_type_channel_1D_coordinates,
	rtrc_user_input_type_channel_2D_coordinates
};

enum rtrc_channel_modes
{
	rtrc_channel_mode_undefined = 0,
	rtrc_channel_mode_normal,
	rtrc_channel_mode_raw
};

struct rtrcUInRawActivator
{
	//rtrc_user_input_value_types sType = rtrc_user_input_type_raw;
	bool active = false;
};

struct rtrcUInPressState
{
	
	//rtrc_user_input_value_types sType = rtrc_user_input_type_press_state;
	rtrc_user_input_trigger_types trigger_type{};
	bool pressed = false;
	bool callCallback = false;
	bool presistent = false;
};

template<typename IT>
struct combinedUserInput
{
	IT mainInput;
	std::vector<rtrcUInPressState> activators;
	std::vector<rtrcUInPressState> deactivators;
	std::vector<rtrcUInRawActivator> rawActivators;
	bool isActive() const;
};

struct rtrcUInChannel1DCoords
{
	static constexpr size_t Dcount = 1;
	//rtrc_user_input_value_types sType = rtrc_user_input_type_channel_1D_coordinates;
	std::array<std::optional<combinedUserInput<rtrcUInPressState>>,1> Npress; // alternative inputs, negative input
	std::array<std::optional<combinedUserInput<rtrcUInPressState>>,1> Ppress; // alternative inputs, negative input
	// changed amount = pressChangeSpeed*lastUpdateTime;
	std::array<double,1> pressChangeSpeeds = { 1.0 }; // speed of which holding a key changes channels
	std::array<std::optional<double>,1> lastUpdateTime = {}; // last update time of each channel
	std::array<double,1> values = {0.0};
	std::array<double,1> effectSpeeds = {1.0};
	bool callCallback = false;
	bool presistent = false; // if callCallback will be set to false after a call.
	rtrc_channel_modes channel_mode{};
	
	void pollEvents(double time);
};

struct rtrcUInChannel2DCoords
{
	static constexpr size_t Dcount = 2;
	//const rtrc_user_input_value_types sType = rtrc_user_input_type_channel_2D_coordinates;
	std::array<std::optional<combinedUserInput<rtrcUInPressState>>,2> Npress; // alternative inputs, negatie input
	std::array<std::optional<combinedUserInput<rtrcUInPressState>>,2> Ppress; // alternative inputs, positive input
	// changed amount = pressChangeSpeed*lastUpdateTime;
	std::array<double,2> pressChangeSpeeds = { 1.0, 1.0 }; // speed of which holding a key changes channels
	std::array<std::optional<double>,2> lastUpdateTime = {}; // last update time of each channel
	std::array<double,2> values = {0.0,0.0};
	std::array<double,2> effectSspeeds = {1.0,1.0};
	bool callCallback = false;
	bool presistent = false; // if callCallback will be set to false after a call.
	rtrc_channel_modes channel_mode{};
	void pollEvents(double time);
};
	

/* when calling pollEvents, make sure callbacks don't require locking any mutex
 * that is already locked by the same thread.
 * also make sure the callbacks don't cause a deadlock in other threads.
 */
struct userInputOptions_core
{
	typedef std::optional<combinedUserInput<rtrcUInPressState>> pti_t;
	typedef std::optional<combinedUserInput<rtrcUInChannel1DCoords>> c1dc_t;
	typedef std::optional<combinedUserInput<rtrcUInChannel2DCoords>> c2dc_t;
	
	typedef std::function<void(bool/*if the input is active and triggered or not*/, double /*call time*/)> button_callback_t;
	typedef std::function<void(double /*main input*/, double/*effecSpeed*/, double /* call time*/,
	rtrc_channel_modes channel_mode)> channel1D_callback_t;
	typedef std::function<void(std::array<double,2> /*main input*/, std::array<double,2>/*effecSpeeds*/, double /* call time*/,
	rtrc_channel_modes channel_mode)> channel2D_callback_t;
	
	enum ch1DInputIndexes
	{
		moveUp = 0, moveRight,
		rotateAboutY, rotateAboutX,
		ch1DInputCount
	};
	
	enum ch2DInputIndexes
	{
		mainPointer = 0,
		ch2DInputCount
	};
	
	enum buttonInputIndexes
	{
		mainPointer_button1 = 0,
		buttonInputCount
	};
	
	// safe to call from pollEvents by callbacks
	void updateChannel1D(const c1dc_t &c1dc, ch1DInputIndexes select);
	
	void updateChannel2D(const c2dc_t &c2dc, ch2DInputIndexes select);
	
	c1dc_t getChannel1D(ch1DInputIndexes select);
	
	void updateNpressChannel1D(double updateTime, const combinedUserInput<rtrcUInPressState> &Npress, ch1DInputIndexes select);
	
	void updatePpressChannel1D(double updateTime, const combinedUserInput<rtrcUInPressState> &Ppress, ch1DInputIndexes select);
	
	void updateButtonPress(const pti_t &inputState, buttonInputIndexes select);
	
	void setCallback_button(const button_callback_t &cb, buttonInputIndexes select);
	
	void setCallback_ch1D(const channel1D_callback_t &cb, ch1DInputIndexes select);
	
	void setCallback_ch2D(const channel2D_callback_t &cb, ch2DInputIndexes select);
	
	// for lower latency, and lower overhead for low latency calls.
	void updateAndPollChannel1D(double time, const c1dc_t &c1dc, ch1DInputIndexes select);
	
	
	
	// for lower latency, and lower overhead for low latency calls.
	void pollCh1DEvent(double time, ch1DInputIndexes select);
	
	/* thread safety :
	 * assigned callbacks are responsible for synchronization of external resources.
	 */
	void pollInputEvents(double time/*used as input for callbacks*/);
	
	void clearFunctionalCallbacks() noexcept;
	
	void getCh2D(ch2DInputIndexes select, double &d1, double &d2);
	
private:
	
	void _pollCh1DEvent(double time, ch1DInputIndexes select, const c1dc_t &chInput);
	
	void _pollCh2DEvent(double time, ch2DInputIndexes select, const c2dc_t &chInput);
	
	void _pollButtonEvent(double time, buttonInputIndexes select, const pti_t &chInput);
	
protected :	
	
	/* locking order :
	 * 1. _callbacksM.
	 * 2. _inputValuesM.
	 */
	
	struct inputValues_t
	{
		std::array<c1dc_t,ch1DInputCount> ch1DInputs;
		std::array<c2dc_t,ch2DInputCount> ch2DInputs;
		std::array<pti_t,buttonInputCount> pressInputs;
	};
	
	mutable std::mutex _callbacksM;
	std::array<channel1D_callback_t,ch1DInputCount> _ch1DInputCallbacks;
	std::array<channel2D_callback_t,ch2DInputCount> _ch2DInputCallbacks;
	std::array<button_callback_t, buttonInputCount> _buttonInputCallbacks;
	
	/* use to modify values. 
	 * it will be automatically included in internal callbacks.
	 */
	mutable std::mutex _inputValuesM;
	inputValues_t _inputValues;
	
};

/* if dragFeature_core is being freed before the userInputOptions_core that 
 * depends on it, then dragFeature_core::cleanup() needs to be called to ensure
 * callbacks don't have invalid captures.
 * 
 * use setCallback for the callback that doesn't care about the path.
 * use setAdditiveCallback for the callback that cares about the path.
 * use presistent = true for button for a live calling of dragCallback_t.
 * 
 * note for callback :
 * during a release trigger, _isDragging is always true. thus a release trigger
 * can be used for preparation for the after Dragging stages where 
 * getIsDragging() is called.
 */

template<typename appInstanceT>
struct dragFeature_core
{
	typedef std::function<void(std::array<double,2> /*startPos*/, 
	std::array<double,2> /*drag*/, rtrc_user_input_trigger_types /*trigger type*/)> dragCallback_t;
	// for additive drag startPos will be the last position that the previous callback was called.
	
	typedef appInstanceT appInstance_t;
	
	dragFeature_core(const dragFeature_core &) = delete;
	dragFeature_core(dragFeature_core &&) = delete;
	dragFeature_core &operator=(const dragFeature_core &) = delete;
	/* tryUseRawMouseMotion :
	 *	ties to use raw mouse motion if supported.
	 * 
	 */
	dragFeature_core(const std::shared_ptr<appInstance_t> &appInstance, dragCallback_t cb = {}, bool tryUseRawMouseMotion = false) noexcept;
	
	// calling configure more than one time, will clear the previous callbacks.
	void configure(userInputOptions_core &uio,
	userInputOptions_core::buttonInputIndexes buttonSelect, 
	userInputOptions_core::ch2DInputIndexes ch2DSelect);
	
	void cleanup() noexcept;
	
	/* there are two types of callbacks. a callback that takes the final location
	 * and a another (additive) that takes the difference in location from the 
	 * last call. this is very useful to make the callback function simpler.
	 */
	void setCallback(const dragCallback_t &cb);
	void setAdditiveCallback(const dragCallback_t &cb);
	
	~dragFeature_core();
	
	std::unique_lock<std::mutex> getIsDragging(bool &out);
private :
	
	void _cleanup() noexcept;
	
	userInputOptions_core::button_callback_t _defaultButtonCb =
	[this](bool pressed, 
		double /*call time*/)
	{
		mainLogger.logMutexLock("locking _gm of dragFeature_core in _defaultButtonCb");
		std::lock_guard<std::mutex> lk(this->_gm);
		mainLogger.logMutexLock("_gm of dragFeature_core in _defaultButtonCb locked");
		std::array<double,2> drag{};
		rtrc_user_input_trigger_types triggerType;
		double curX{};
		double curY{};
		this->_owner->getCh2D(_ch2D_select, curX,curY);
		if(this->_isDragging)
		{
			drag[0] = curX - this->_startX;
			drag[1] = curY - this->_startY;
			if(pressed)
			{
				triggerType = rtrc_user_input_trigger_types::rtrc_user_input_hold;
				// hold trigger
				
			}
			else // release trigger
			{
#ifndef NDEBUG
				mainLogger.logIo("drag release trigger");
#endif
				if(this->_useRawMotion)
					this->_appInstance->getWindow()->deactivateRawPointer();
				triggerType = rtrc_user_input_trigger_types::rtrc_user_input_release;
				// a event poll while dragging.
			}
		}
		else if(pressed)
		{
#ifndef NDEBUG
			mainLogger.logIo("drag key down trigger");
#endif
			// starting drag.
			triggerType = rtrc_user_input_trigger_types::rtrc_user_input_press;
			this->_startX = curX;
			this->_startY = curY;
			this->_prevDragCurX = curX;
			this->_prevDragCurY = curY;
			if(this->_useRawMotion)
			{
				this->_appInstance->getWindow()->activateRawPointer();
			}
			// drag will be zero.
		}
		mainLogger.logMutexLock("calling callbacks while holding _gm of dragFeature_core in _defaultButtonCb");
		if(this->_dragCallback)
		{
			this->_dragCallback({this->_startX,this->_startY},
			drag,
			triggerType);
		}
		if(this->_additiveDragCallback)
		{
			// bug fix, some softwares have a jumpy raw mode for the cursor.
			const double additiveLimit = 20.0;
			auto additiveDragAmountX = curX-this->_prevDragCurX;
			auto additiveDragAmountY = curY-this->_prevDragCurY;
			if(additiveDragAmountX > additiveLimit || additiveDragAmountX < -additiveLimit || 
					additiveDragAmountY > additiveLimit || additiveDragAmountY < -additiveLimit)
			{
				additiveDragAmountX = 0;
				additiveDragAmountY = 0;
			}
			this->_additiveDragCallback({this->_prevDragCurX, this->_prevDragCurY},
			{additiveDragAmountX, additiveDragAmountY}, triggerType);
		}
		this->_prevDragCurX = curX;
		this->_prevDragCurY = curY;
		this->_isDragging = pressed;
		mainLogger.logMutexLock("unlocking _gm of dragFeature_core in _defaultButtonCb");
	};
	
	std::shared_ptr<appInstance_t> _appInstance;
	std::mutex _gm; // general mutex
	dragCallback_t _dragCallback;
	dragCallback_t _additiveDragCallback;
	bool _isDragging = false; // detect the transition.
	bool _useRawMotion = false;
	bool _autoReset = false;
	double _startX = 0, _startY = 0; // represent the starting point.
	double _prevDragCurX = 0, _prevDragCurY = 0;
	userInputOptions_core *_owner = nullptr;
	userInputOptions_core::buttonInputIndexes _button_select;
	userInputOptions_core::ch2DInputIndexes _ch2D_select;
};
}
}
