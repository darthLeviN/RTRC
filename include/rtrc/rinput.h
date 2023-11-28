/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   rinput.h
 * Author: roohi
 *
 * Created on September 29, 2019, 6:38 AM
 */
#pragma once
#include "decs/rinput_decs.h"
#include <functional>
#include <optional>
#include <mutex>
#include <type_traits>
#include <array>
#include "rtrc.h"
#include "rexception.h"

namespace rtrc
{
namespace uinl
{
	


template<typename IT>
inline bool combinedUserInput<IT>::isActive() const
{
	for( auto &i : rawActivators )
	{
		if(i.active == false) return false;
	}
	for( auto &i : activators )
	{
		if(i.pressed == false) return false;
	}
	for( auto &i : deactivators )
	{
		if(i.pressed ) return false;
	}
	return true;
}



inline void rtrcUInChannel1DCoords::pollEvents(double time)
{
	for(size_t i = 0; i < Dcount; ++i)
	{
		if( Npress[i].has_value() && Ppress[i].has_value())
		{
			callCallback = true;
			combinedUserInput<rtrcUInPressState> &np = Npress[i].value();
			combinedUserInput<rtrcUInPressState> &pp = Ppress[i].value();

			bool neg = np.isActive() && np.mainInput.pressed;
			bool pos = pp.isActive() && pp.mainInput.pressed;

			if(lastUpdateTime[i].has_value())
			{
				if(neg && !pos)
				{
					values[i] -= (time - lastUpdateTime[i].value())*pressChangeSpeeds[i];
					callCallback = true;
				}
				else if (!neg && pos)
				{
					values[i] += (time - lastUpdateTime[i].value())*pressChangeSpeeds[i];
					callCallback = true;
				}
			}
			lastUpdateTime[i] = time;

		}
	}
}



inline void rtrcUInChannel2DCoords::pollEvents(double time)
{
	for(size_t i = 0; i < Dcount; ++i)
	{
		if( Npress[i].has_value() && Ppress[i].has_value())
		{
			combinedUserInput<rtrcUInPressState> &np = Npress[i].value();
			combinedUserInput<rtrcUInPressState> &pp = Ppress[i].value();
			bool neg = np.isActive() && np.mainInput.pressed;
			bool pos = pp.isActive() && pp.mainInput.pressed;
			if(lastUpdateTime[i].has_value())
			{
				if(neg && !pos)
				{
					values[i] -= (time - lastUpdateTime[i].value())*pressChangeSpeeds[i];
					callCallback = true;
				}
				else if (!neg && pos)
				{
					values[i] += (time - lastUpdateTime[i].value())*pressChangeSpeeds[i];
					callCallback = true;
				}

			}
			lastUpdateTime[i] = time;
		}
	}
}

	


	
// safe to call from pollEvents by callbacks
inline void userInputOptions_core::updateChannel1D(const c1dc_t &c1dc, ch1DInputIndexes select)
{
	std::lock_guard<std::mutex> ilk(_inputValuesM);
	//static_assert(std::is_copy_constructible<c1dc_t::value_type>::value);
	//static_assert(std::is_copy_assignable<c1dc_t::value_type>::value);
	_inputValues.ch1DInputs[select] = c1dc;
}

inline void userInputOptions_core::updateChannel2D(const c2dc_t &c2dc, ch2DInputIndexes select)
{
	std::lock_guard<std::mutex> ilk(_inputValuesM);
	_inputValues.ch2DInputs[select] = c2dc;
}

inline userInputOptions_core::c1dc_t userInputOptions_core::getChannel1D(ch1DInputIndexes select)
{
	std::lock_guard<std::mutex> ilk(_inputValuesM);
	return _inputValues.ch1DInputs[select];
}

inline void userInputOptions_core::updateNpressChannel1D(double updateTime, const combinedUserInput<rtrcUInPressState> &Npress, ch1DInputIndexes select)
{
	std::lock_guard<std::mutex> ilk(_inputValuesM);
	if(_inputValues.ch1DInputs[select].has_value())
	{
		if(_inputValues.ch1DInputs[select].value().mainInput.Npress[0].has_value())
		{
			auto &ni = _inputValues.ch1DInputs[select].value().mainInput.Npress[0].value();
			bool activationChange = ni.isActive() != Npress.isActive();
			bool pressChange = ni.mainInput.pressed != Npress.mainInput.pressed;
			if(activationChange || pressChange)
			{
				_inputValues.ch1DInputs[select].value().mainInput.lastUpdateTime[0] = updateTime;
			}
		}
		_inputValues.ch1DInputs[select].value().mainInput.Npress[0] = Npress;

	}
}

inline void userInputOptions_core::updatePpressChannel1D(double updateTime, const combinedUserInput<rtrcUInPressState> &Ppress, ch1DInputIndexes select)
{
	std::lock_guard<std::mutex> ilk(_inputValuesM);
	if(_inputValues.ch1DInputs[select].has_value())
	{
		if(_inputValues.ch1DInputs[select].value().mainInput.Ppress[0].has_value())
		{
			auto &pi = _inputValues.ch1DInputs[select].value().mainInput.Ppress[0].value();
			bool activationChange = pi.isActive() != Ppress.isActive();
			bool pressChange = pi.mainInput.pressed != Ppress.mainInput.pressed;
			if(activationChange || pressChange)
			{
				_inputValues.ch1DInputs[select].value().mainInput.lastUpdateTime[0] = updateTime;
			}
		}
		_inputValues.ch1DInputs[select].value().mainInput.Ppress[0] = Ppress;

	}
}

inline void userInputOptions_core::updateButtonPress(const pti_t &inputState, buttonInputIndexes select)
{
	std::lock_guard<std::mutex> ilk(_inputValuesM);
	_inputValues.pressInputs[select] = inputState;
}

inline void userInputOptions_core::setCallback_button(const button_callback_t &cb, buttonInputIndexes select)
{
	std::lock_guard<std::mutex> ccblk(_callbacksM);
	_buttonInputCallbacks[select] = cb;
}

inline void userInputOptions_core::setCallback_ch1D(const channel1D_callback_t &cb, ch1DInputIndexes select)
{
	std::lock_guard<std::mutex> ccblk(_callbacksM);
	_ch1DInputCallbacks[select] = cb;
}

inline void userInputOptions_core::setCallback_ch2D(const channel2D_callback_t &cb, ch2DInputIndexes select)
{
	std::lock_guard<std::mutex> ccblk(_callbacksM);
	_ch2DInputCallbacks[select] = cb;
}

// for lower latency, and lower overhead for low latency calls.
inline void userInputOptions_core::updateAndPollChannel1D(double time, const c1dc_t &c1dc, ch1DInputIndexes select)
{
	std::lock_guard<std::mutex> ccblk(_callbacksM);
	std::unique_lock<std::mutex> ilk(_inputValuesM);
	_inputValues.ch1DInputs[select] = c1dc;
	ilk.unlock();
	auto copy = c1dc;
	_pollCh1DEvent(time,select,copy);
}



// for lower latency, and lower overhead for low latency calls.
inline void userInputOptions_core::pollCh1DEvent(double time, ch1DInputIndexes select)
{
	std::lock_guard<std::mutex> ccblk(_callbacksM);
	std::unique_lock<std::mutex> ilk(_inputValuesM);
	auto icopy = _inputValues.ch1DInputs[select];
	ilk.unlock();
	_pollCh1DEvent(time,select,icopy);
}

/* thread safety :
 * assigned callbacks are responsible for synchronization of external resources.
 */
inline void userInputOptions_core::pollInputEvents(double time/*used as input for callbacks*/)
{
	mainLogger.logPerformance("calling userInputOptions_core::pollInputEvents");
	std::lock_guard<std::mutex> ccblk(_callbacksM);
	std::unique_lock<std::mutex> ilk(_inputValuesM);
	auto icopy = _inputValues;
	// setting the new values.
	for(size_t i = 0; i < ch1DInputCount; ++i)
	{
		if(_inputValues.ch1DInputs[i].has_value())
		{
			auto &mainInput = _inputValues.ch1DInputs[i].value().mainInput;
			mainInput.callCallback = false || mainInput.presistent;
			mainInput.pollEvents(time);
		}
	}
	for(size_t i = 0; i < ch2DInputCount; ++i)
	{
		if(_inputValues.ch2DInputs[i].has_value())
		{
			auto &mainInput = _inputValues.ch2DInputs[i].value().mainInput;
			mainInput.callCallback = false || mainInput.presistent;
			mainInput.pollEvents(time);
		}
	}
	for(size_t i = 0; i < buttonInputCount; ++i)
	{
		if(_inputValues.pressInputs[i].has_value())
		{
			auto &mainInput = _inputValues.pressInputs[i].value().mainInput;
			mainInput.callCallback = false || mainInput.presistent;
		}
	}
	ilk.unlock();

	for(size_t i = 0; i < ch2DInputCount; ++i)
	{
		_pollCh2DEvent(time,(ch2DInputIndexes)i, icopy.ch2DInputs[i]);
	}

	// polling based on old values
	for(size_t i = 0; i < ch1DInputCount; ++i)
	{
		_pollCh1DEvent(time,(ch1DInputIndexes)i,icopy.ch1DInputs[i]);
	};

	for(size_t i = 0; i < buttonInputCount; ++i)
	{
		_pollButtonEvent(time, (buttonInputIndexes)i, icopy.pressInputs[i]);
	}
	mainLogger.logPerformance("userInputOptions_core::pollInputEvents called");
}

inline void userInputOptions_core::clearFunctionalCallbacks() noexcept
{
	std::lock_guard<std::mutex> lk(_callbacksM);
	button_callback_t bc{};
	channel1D_callback_t c1dc{};
	channel2D_callback_t c2dc{};
	for(int i = 0; i < ch1DInputCount; ++i)
	{
		_ch1DInputCallbacks[i] = c1dc;
	}
	for(int i = 0; i < ch2DInputCount; ++i)
	{
		_ch2DInputCallbacks[i] = c2dc;
	}
}

inline void userInputOptions_core::getCh2D(ch2DInputIndexes select, double &d1, double &d2)
{
	std::lock_guard<std::mutex> lk(_inputValuesM);
	if(_inputValues.ch2DInputs[select].has_value())
	{
		const auto &mainInput = _inputValues.ch2DInputs[select].value().mainInput;
		d1 = mainInput.values[0];
		d2 = mainInput.values[1];
	}
}



inline void userInputOptions_core::_pollCh1DEvent(double time, ch1DInputIndexes select, const c1dc_t &chInput)
{
	if(_ch1DInputCallbacks[select] && chInput.has_value() && chInput.value().isActive())
	{
		const rtrcUInChannel1DCoords &input = chInput.value().mainInput;
		if(input.callCallback)
		{

			double mainValue = input.values[0];

			/* invoking callback 
			 * void(std::optional<bool> alternative Ninput, , std::optional<bool> Pinput, std::optional<double> main input, double pressChangeSpeed, double effecSpeed,
			 *  doublcall_time)
			 */

			_ch1DInputCallbacks[select](mainValue, input.effectSpeeds[0], time,
				input.channel_mode);
		}
	}
}

inline void userInputOptions_core::_pollCh2DEvent(double time, ch2DInputIndexes select, const c2dc_t &chInput)
{
	if(_ch2DInputCallbacks[select] && chInput.has_value() && chInput.value().isActive())
	{
		const rtrcUInChannel2DCoords &input = chInput.value().mainInput;
		if(input.callCallback)
		{

			const auto &mainValue = input.values;
			_ch2DInputCallbacks[select](mainValue, input.effectSspeeds, time,
				input.channel_mode);
		}
	}
}

inline void userInputOptions_core::_pollButtonEvent(double time, buttonInputIndexes select, const pti_t &chInput)
{
	if(_buttonInputCallbacks[select] && chInput.has_value() && chInput.value().isActive())
	{
		const rtrcUInPressState &input = chInput.value().mainInput;
		if(input.callCallback)
		{
			if(input.trigger_type == rtrc_user_input_trigger_types::rtrc_user_input_hold)
			{
				_buttonInputCallbacks[select](input.pressed, time);
			}
			else
				throw rImplementationError("unimplemented press input trigger type");
		}
	}
}


/* tryUseRawMouseMotion :
 *	ties to use raw mouse motion if supported.
 * 
 */
template<typename appInstanceT>
inline dragFeature_core<appInstanceT>::dragFeature_core(const std::shared_ptr<appInstance_t> &appInstance, dragCallback_t cb, bool tryUseRawMouseMotion) noexcept
	: _appInstance(appInstance), _dragCallback(cb), _useRawMotion(false)
{
	if(tryUseRawMouseMotion)
	{
		_useRawMotion = rtrcRawMouseMotionIsSupported();
	}
}

// calling configure more than one time, will clear the previous callbacks.
template<typename appInstanceT>
inline void dragFeature_core<appInstanceT>::configure(userInputOptions_core &uio,
userInputOptions_core::buttonInputIndexes buttonSelect, 
userInputOptions_core::ch2DInputIndexes ch2DSelect)
{
	std::lock_guard<std::mutex> lk(_gm);
	if(_owner)
	{
		_cleanup();
	}
	_owner = &uio;
	_ch2D_select = ch2DSelect;
	_button_select = buttonSelect;
	//uio.setCallback_ch2D(_defaultCh2DCb, _ch2D_select);
	uio.setCallback_button(_defaultButtonCb, _button_select);
}

template<typename appInstanceT>
inline void dragFeature_core<appInstanceT>::cleanup() noexcept
{
	std::lock_guard<std::mutex> lk(_gm);
	if(_owner)
	{
		_cleanup();
	}
	_owner = nullptr;
}

/* there are two types of callbacks. a callback that takes the final location
 * and a another (additive) that takes the difference in location from the 
 * last call. this is very useful to make the callback function simpler.
 */
template<typename appInstanceT>
inline void dragFeature_core<appInstanceT>::setCallback(const dragCallback_t &cb)
{
	std::lock_guard<std::mutex> lk(_gm);
	_dragCallback = cb;
}

template<typename appInstanceT>
inline void dragFeature_core<appInstanceT>::setAdditiveCallback(const dragCallback_t &cb)
{
	std::lock_guard<std::mutex> lk(_gm);
	_additiveDragCallback = cb;
}

template<typename appInstanceT>
inline dragFeature_core<appInstanceT>::~dragFeature_core()
{
	cleanup();
}

template<typename appInstanceT>
inline std::unique_lock<std::mutex> dragFeature_core<appInstanceT>::getIsDragging(bool &out)
{
	mainLogger.logMutexLock("locking _gm of dragFeature_core in getIsDragging");
	std::unique_lock<std::mutex> lk(_gm);
	mainLogger.logMutexLock("_gm of dragFeature_core locked in getIsDragging");
	out = _isDragging;
	mainLogger.logMutexLock("returning _gm of dragFeature_core in getIsDragging");
	return std::move(lk);
}

template<typename appInstanceT>
inline void dragFeature_core<appInstanceT>::_cleanup() noexcept
{
	//_owner->setCallback_ch2D({},_ch2D_select);
	_owner->setCallback_button({},_button_select);
}

	
	
}
}
