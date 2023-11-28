#pragma once
#include "decs/rglfw_decs.h"
#include "graphical/rvulkan.h"
#include <memory>
#include <GLFW/glfw3.h>
#include "rtrc.h"
#include <thread>
#include "rexception.h"
#include "rinput.h"
#include <tuple>



namespace rtrc
{
namespace rglfwl
{

static inline void pollEvents()
{
	if(!isThreadMain())
		throw invalidCallState("trying to poll glfw events outside of main thread");
	glfwPollEvents();
}

static inline void rglfwEnableRawMouse(GLFWwindow *window)
{
#ifndef NDEBUG
	if(!isThreadMain())
		throw invalidCallState("trying to enable raw mouse input mode outside of main thread");
#endif
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
}

static inline void rglfwDisableRawMouse(GLFWwindow *window)
{
#ifndef NDEBUG
	if(!isThreadMain())
		throw invalidCallState("trying to disable raw mouse input mode outside of main thread");
#endif
	glfwSetInputMode(window,GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
	glfwSetInputMode(window,GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

inline rglfwWindow::windowUserData::windowUserData(const size_t& width, const size_t& height)
	: _width(width), _height(height)
{
	
}

inline void rglfwWindow::windowUserData::setMainCursorButtonCallback(mainCursorButtonCallback_t cb)
{
	std::lock_guard<std::mutex> lk(_gm);
	_mainCursorButtonCallback = cb;
}

inline void rglfwWindow::windowUserData::setMainCursorPosCallback(mainCursorPosCallback_t cb)
{
	std::lock_guard<std::mutex> lk(_gm);
	_mainCursorPosCallback = cb;
}

inline void rglfwWindow::windowUserData::setMainKeyCallback(mainKeyCallback_t cb)
{
	std::lock_guard<std::mutex> lk(_gm);
	_mainKeyCallback = cb;
}

inline void rglfwWindow::windowUserData::setResizeCallback(resizeCallback_t cb)
{
	std::lock_guard<std::mutex> lk(_gm);
	_resizeCallback = cb;
}

template<typename ...Args>
inline void rglfwWindow::windowUserData::invokeMainCursorButtonCallback(const Args&... args)
{
	std::lock_guard<std::mutex> lk(_gm);
	if(_mainCursorButtonCallback)
		_mainCursorButtonCallback(args...);
}

template<typename ...Args>
inline void rglfwWindow::windowUserData::invokeMainCursorPosCallback(const Args&... args)
{
	std::lock_guard<std::mutex> lk(_gm);
	if(_mainCursorPosCallback)
		_mainCursorPosCallback(args...);
}

template<typename ...Args>
inline void rglfwWindow::windowUserData::invokeMainKeyCallback(const Args&... args)
{
	std::lock_guard<std::mutex> lk(_gm);
	if(_mainKeyCallback)
		_mainKeyCallback(args...);
}

inline void rglfwWindow::windowUserData::invokeResizeCallback(const int &width, const int &height)
{	
	std::lock_guard<std::mutex> lk(_gm);
	_width = width < 0 ? 0:width;
	_height = height < 0 ? 0:height;
	if(_resizeCallback)
		_resizeCallback(_width, _height);
	
}

inline void rglfwWindow::windowUserData::invokeResizeCallback()
{
	std::lock_guard<std::mutex> lk(_gm);
	if(_resizeCallback)
		_resizeCallback(_width, _height);
}
	
inline rglfwWindow::rglfwWindow(int width, int height, const char* title)
	: userInputOptions_core{}, _windowUD(width, height)
{
#ifndef NDEBUG
	if(!isThreadMain())
		throw mainAppObjInitializationFailure(" attempt to create glfw window outside of main thread");
#endif
	_hwindow = glfwCreateWindow(width,height,"fff",NULL,NULL);
	if(!_hwindow)
		throw mainAppObjInitializationFailure("could not create glfw window");
	// these set functions can be summarized in a constructor.
	_windowUD.setMainKeyCallback([this](int key, int scanCode, int action, int mod)
	{
		keyCallback_t &func = this->_keyCallbacks[GLFW_KEY_to_array_index(key)];
		if(func) func(glfwGetTime(),this,action,mod);
	});

	_windowUD.setMainCursorPosCallback([this](double time, double x, double y)
	{
		std::lock_guard<std::mutex> lk(_optsM);
		uinl::rtrc_channel_modes channel_mode{};
		if(_prevIsPointerRaw)
			channel_mode = uinl::rtrc_channel_modes::rtrc_channel_mode_raw;
		else
			channel_mode = uinl::rtrc_channel_modes::rtrc_channel_mode_normal;
		if(this->_cursorPosCalback) this->_cursorPosCalback(time, this, x, y, channel_mode);
	});

	_windowUD.setMainCursorButtonCallback([this](double time,int button, int action, 
int mods)
	{
		cursorButtonCallback_t &func = 
			this->_cursorButtonCallbacks[GLFW_MOUSE_BUTTON_to_array_index(button)];
		if(func) func(time, this, action, mods );
	});

	glfwSetWindowUserPointer(_hwindow, &_windowUD);

	_setGlfwKeyCallback();
	_setGlfwResizeCallback();
	_setGlfwCursorPosCallback();
	_setGlfwCursorButtonCallback();

	uinl::combinedUserInput<uinl::rtrcUInChannel1DCoords> ch1d{};
	ch1d.mainInput.Npress[0] = uinl::combinedUserInput<uinl::rtrcUInPressState>{};
	ch1d.mainInput.Ppress[0] = uinl::combinedUserInput<uinl::rtrcUInPressState>{};
	updateChannel1D({ch1d}, moveUp);
}

inline void rglfwWindow::_setGlfwResizeCallback()//GLFWframebuffersizefun glfwResizeCallback)
{
	//_glfwResizeCallback = glfwResizeCallback;
	glfwSetFramebufferSizeCallback(_hwindow, _defaultResizeCallback);
}

inline void rglfwWindow::_defaultResizeCallback(GLFWwindow *window, int width, int height)
{
	userData_t *ud = (userData_t*)glfwGetWindowUserPointer(window);
	ud->invokeResizeCallback(width, height);
}

inline void rglfwWindow::setKeyCallback(const keyCallback_t &cb, int key)
{
	if(GLFW_KEY_to_array_index(key) < _keyCallbacks.size())
		_keyCallbacks[GLFW_KEY_to_array_index(key)] = cb;
}

inline void rglfwWindow::setMouseButtonCallback( const cursorButtonCallback_t &cb, int key)
{
	if(GLFW_MOUSE_BUTTON_to_array_index(key) < _cursorButtonCallbacks.size())
		_cursorButtonCallbacks[GLFW_MOUSE_BUTTON_to_array_index(key)] = cb;
}

inline void rglfwWindow::setCursorPosCallback(const cursorPosCallback_t &cb)
{
	_cursorPosCalback = cb;
}

inline void rglfwWindow::setResizeCallback(resizeCallback_t cb, bool callonce)
{
	_windowUD.setResizeCallback(cb);
	if(callonce)_windowUD.invokeResizeCallback();
}

// may be called from any thread, but should be called once only.
inline VkResult rglfwWindow::createSurface(VkInstance instance, VkSurfaceKHR *surface, VkAllocationCallbacks *pAllocator)
{
	std::lock_guard<std::mutex> lk(_optsM);
	if(_surfaceIsCreated)
		throw mainAppObjInitializationFailure(" attempt to create VkSurfaceKHR more than once for the same window");
	// may be called from any thread
	VkResult ret = glfwCreateWindowSurface( instance, _hwindow, pAllocator, surface);
	_surfaceIsCreated = true;
	return ret;
}

inline void rglfwWindow::show() noexcept
{
#ifndef NDEBUG
	if(!isThreadMain())
		throw mainAppObjInitializationFailure(" attempt to call rglfwWindow::show() outside of main thread");
#endif
	std::lock_guard<std::mutex> lk(_optsM);
	_isVisible = true;
}

inline bool rglfwWindow::shouldClose() noexcept
{
	// may be called from any thread
	return glfwWindowShouldClose(_hwindow);
}

// safe to call outside of main thread.
inline rglfwWindow::~rglfwWindow()
{
	if(!isThreadMain())
	{
		main_thread_queue_handler.appendCommand([_hwindow = _hwindow](){glfwDestroyWindow(_hwindow);});
	}
	else
		glfwDestroyWindow(_hwindow);
}


// this call is thread safe. it is delayed by pollManualWindowEvents()
inline void rglfwWindow::activateRawPointer() noexcept
{
	std::lock_guard<std::mutex> lk(_optsM);
	_isPointerRaw = true;
}

// this call is thread safe. it is delayed by pollManualWindowEvents()
inline void rglfwWindow::deactivateRawPointer() noexcept
{
	std::lock_guard<std::mutex> lk(_optsM);
	_isPointerRaw = false;
}

// this function should be called in the main thread's polling loop, preferably after other poll functions. 
inline void rglfwWindow::pollManualWindowEvents()
{
	mainLogger.logPerformance("calling rglfwWindow::pollManualWindowEvents");
#ifndef NDEBUG
	if(!isThreadMain())
		throw invalidCallState("trying to call pollManualWindowEvents outside of main thread");
#endif
	{
		std::lock_guard<std::mutex> israwlk(_optsM);
		if(_isPointerRaw && !_prevIsPointerRaw)
			rglfwEnableRawMouse(_hwindow);
		else if(!_isPointerRaw && _prevIsPointerRaw)
			rglfwDisableRawMouse(_hwindow);
		if(_isVisible && !_prevIsVisible)
			glfwShowWindow(_hwindow);
		else if(!_isVisible && _prevIsVisible)
			glfwHideWindow(_hwindow);
		_prevIsPointerRaw = _isPointerRaw;
		_prevIsVisible = _isVisible;
	}
	mainLogger.logPerformance("rglfwWindow::pollManualWindowEvents called");
}


inline void rglfwWindow::_setGlfwKeyCallback()
{
	//_glfwKeyCallback = kcb;
	glfwSetKeyCallback(_hwindow, _defaultKeyCallback);
}

inline void rglfwWindow::_defaultCursorPosCallback(GLFWwindow *window, double x, double y)
{
	userData_t *userData = (userData_t *)glfwGetWindowUserPointer(window);
	userData->invokeMainCursorPosCallback(glfwGetTime(),x,y);
}

inline void rglfwWindow::_setGlfwCursorPosCallback()
{
	glfwSetCursorPosCallback(_hwindow, _defaultCursorPosCallback);
}

inline void rglfwWindow::_defaultCursorButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	userData_t *userData = (userData_t *)glfwGetWindowUserPointer(window);
	userData->invokeMainCursorButtonCallback(glfwGetTime(), button, action, mods);
}

inline void rglfwWindow::_setGlfwCursorButtonCallback()
{
	glfwSetMouseButtonCallback(_hwindow, _defaultCursorButtonCallback);
}


inline void rglfwWindow::_defaultKeyCallback(GLFWwindow *window,int key,int scanCode,int action,int mod)
{
	userData_t *userData = (userData_t *)glfwGetWindowUserPointer(window);
	userData->invokeMainKeyCallback(key, scanCode, action,mod);
}



inline void commonKeyCallbacks::leftCallback(
	double time, uinl::userInputOptions_core *inputOptionsCore,
	int action,int mod)
{
	uinl::combinedUserInput<uinl::rtrcUInPressState> npress{};
	npress.mainInput.trigger_type = uinl::rtrc_user_input_hold;
	if(action == GLFW_PRESS)
	{
		npress.mainInput.pressed = true;
	}
	inputOptionsCore->updateNpressChannel1D(time,npress,uinl::userInputOptions_core::ch1DInputIndexes::moveRight);
}

inline void commonKeyCallbacks::rightCallback(
	double time, uinl::userInputOptions_core *inputOptionsCore,
	int action,int mod)
{
	uinl::combinedUserInput<uinl::rtrcUInPressState> ppress{};
	ppress.mainInput.trigger_type = uinl::rtrc_user_input_hold;
	if(action == GLFW_PRESS)
	{
		ppress.mainInput.pressed = true;
	}
	inputOptionsCore->updatePpressChannel1D(time,ppress,uinl::userInputOptions_core::ch1DInputIndexes::moveRight);
}

inline void commonKeyCallbacks::upCallback(
	double time, uinl::userInputOptions_core *inputOptionsCore,
	int action,int mod)
{
	uinl::combinedUserInput<uinl::rtrcUInPressState> ppress{};
	ppress.mainInput.trigger_type = uinl::rtrc_user_input_hold;
	if(action == GLFW_PRESS || action == GLFW_REPEAT)
	{
		ppress.mainInput.pressed = true;
	}
	inputOptionsCore->updatePpressChannel1D(time,ppress,uinl::userInputOptions_core::ch1DInputIndexes::moveUp);
}

inline void commonKeyCallbacks::downCallback(
	double time, uinl::userInputOptions_core *inputOptionsCore,
	int action,int mod)
{
	uinl::combinedUserInput<uinl::rtrcUInPressState> npress{};
	npress.mainInput.trigger_type = uinl::rtrc_user_input_hold;
	if(action == GLFW_PRESS || action == GLFW_REPEAT)
	{
		npress.mainInput.pressed = true;
	}
	inputOptionsCore->updateNpressChannel1D(time,npress,uinl::userInputOptions_core::ch1DInputIndexes::moveUp);
}

inline void commonKeyCallbacks::mouseLeftClickCallback(double time, uinl::userInputOptions_core *ioc,
int action, int mod)
{
	uinl::combinedUserInput<uinl::rtrcUInPressState> mousePress{};
	mousePress.mainInput.trigger_type = uinl::rtrc_user_input_hold;
	mousePress.mainInput.pressed = action == GLFW_PRESS;
	mousePress.mainInput.callCallback = true;
	mousePress.mainInput.presistent = mousePress.mainInput.pressed;
	ioc->updateButtonPress({mousePress},uinl::userInputOptions_core::buttonInputIndexes::mainPointer_button1);

}

inline void commonKeyCallbacks::mousePosCallback(
double time, uinl::userInputOptions_core *inputOptionsCore,
int x, int y, uinl::rtrc_channel_modes channel_mode)
{
	uinl::combinedUserInput<uinl::rtrcUInChannel2DCoords> input{};
	input.mainInput.values[0] = x;
	input.mainInput.values[1] = y;
	input.mainInput.lastUpdateTime[0] = time;
	input.mainInput.lastUpdateTime[1] = time;
	input.mainInput.callCallback = true;
	input.mainInput.presistent = true;
	input.mainInput.channel_mode = channel_mode;
	inputOptionsCore->updateChannel2D(input, uinl::userInputOptions_core::ch2DInputIndexes::mainPointer);
}

template<typename windowT>
inline void commonKeyCallbacks::configure(windowT &window)
{
	typedef typename windowT::keyCallback_t keyCallback_t;
	window.setKeyCallback(keyCallback_t(upCallback),GLFW_KEY_UP);
	window.setKeyCallback(keyCallback_t(downCallback),GLFW_KEY_DOWN);
	window.setKeyCallback(keyCallback_t(leftCallback),GLFW_KEY_LEFT);
	window.setKeyCallback(keyCallback_t(rightCallback),GLFW_KEY_RIGHT);

	typedef typename windowT::cursorButtonCallback_t cursorButtonCallback_t;
	window.setMouseButtonCallback(cursorButtonCallback_t(mouseLeftClickCallback),
		GLFW_MOUSE_BUTTON_1);

	typedef typename windowT::cursorPosCallback_t cursorPosCallback_t; 
	window.setCursorPosCallback(cursorPosCallback_t(mousePosCallback));
}



}
}
