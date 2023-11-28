#pragma once
#include "../graphical/rvulkan.h"
#include <memory>
#include <GLFW/glfw3.h>
#include "../rtrc.h"
#include <thread>
#include "../rexception.h"
#include "../rinput.h"
#include <tuple>
#include "pre/rglfw_pre_decs.h"



namespace rtrc
{
namespace rglfwl
{


static inline void pollEvents();

static inline void rglfwEnableRawMouse(GLFWwindow *window);

static inline void rglfwDisableRawMouse(GLFWwindow *window);

/* thread safety : because of glfw limits, this struct is not designed to be 
 * owned outside of the main thread, although userInputOptions_core is designed 
 * to be able to handle multithreading, thus use of userInputOptions_core in 
 * other threads is allowed.
 * in other means, you can change the values, activations and callbacks for a specific 
 * "functionality" in another thread, but changing the "key" callbacks outside of the 
 * main thread is not allowed.
 * 
 * resource management considerations :
 * the hierarchy that is above the window is responsible to manage the 
 * windowUserData::extraData because it may contain circular references.
 */
struct rglfwWindow : uinl::userInputOptions_core
{
	typedef std::function<void(int /*GLFW_KEY_XXX*/,int /*scancode*/,
		int /*GLFW_PRESS/RELEASE/REPEAT*/,int /*GLFW_MOD_XXX*/)> mainKeyCallback_t;
	typedef std::function<void(double time, userInputOptions_core */*inputOptionsCore*/,
		int /*GLFW_PRESS/RELEASE/REPEAT*/,int /*GLFW_MOD_XXX*/)> keyCallback_t;
	typedef std::function<void(double time,double,double)> mainCursorPosCallback_t;
	typedef std::function<void(double time, userInputOptions_core */*inputOptionsCore*/
	,double,double, uinl::rtrc_channel_modes channel_mode)> cursorPosCallback_t;
	typedef std::function<void(double time, int /*button*/, int /*action*/, 
	int /*mods*/)> mainCursorButtonCallback_t;
	typedef std::function<void(double time, userInputOptions_core */*inputOptionsCore*/,
	int /*GLFW_PRESS/RELEASE*/, int /*GLFW_MOD_XX*/)> cursorButtonCallback_t;
	typedef std::function<void(int/*width*/, int/*height*/)> resizeCallback_t;
	//typedef std::function<void()> windowDestroyer_t;
	typedef std::function<void(/*it's not a const but don't change it*/resizeCallback_t *)> setResizeCallback_t;
	typedef void *extraData_t;
	/* 
	 * 
	 * the internally set default callbacks are responsible to call the main callbacks.
	 * all functions of this struct may be called from any thread.
	 */
	struct windowUserData
	{	
		windowUserData(const size_t &width = 0, const size_t &height = 0);
		
		void setResizeCallback(resizeCallback_t cb);
		void setMainKeyCallback(mainKeyCallback_t cb);
		void setMainCursorPosCallback(mainCursorPosCallback_t cb);
		void setMainCursorButtonCallback(mainCursorButtonCallback_t cb);
		
		
		void invokeResizeCallback(const int &width, const int &height);
		void invokeResizeCallback(); // use previous width and height.
		// templates are considered to simplify any changes.
		
		template<typename ...Args>
		void invokeMainKeyCallback(const Args &... args);
		
		template<typename ...Args>
		void invokeMainCursorPosCallback(const Args &... args);
		
		template<typename ...Args>
		void invokeMainCursorButtonCallback(const Args &... args);
		
	private:
		
		
		std::mutex _gm;
		resizeCallback_t _resizeCallback; // calls the resizeCallback
		mainKeyCallback_t _mainKeyCallback; // selects from keyCallbacks, calls it with userInputOptions_core.
		mainCursorPosCallback_t _mainCursorPosCallback; // calls the cursorPosCallback and gives it the userInputOptions_core
		mainCursorButtonCallback_t _mainCursorButtonCallback; // selects from cursorButtonCallbacks, calls it with userInputOptions_core
		size_t _width{}, _height{};
	};
	
	rglfwWindow() = delete;
	rglfwWindow(const rglfwWindow &) = delete;
	rglfwWindow(const rglfwWindow &&) = delete;
	
	typedef windowUserData userData_t; // used by externally set callbacks to retrive data.
	rglfwWindow(int width, int height, const char* title);
	
	
	// may be called from any thread
	void setKeyCallback(const keyCallback_t &cb, int key);
	
	// may be called from any thread.
	void setMouseButtonCallback( const cursorButtonCallback_t &cb, int key);
	
	// may be called from any thread.
	void setCursorPosCallback(const cursorPosCallback_t &cb);
	
	// may be called from any thread
	void setResizeCallback(resizeCallback_t cb, bool callonce = false);
	
	// may be called from any thread, but should be called once only.
	VkResult createSurface(VkInstance instance, VkSurfaceKHR *surface, VkAllocationCallbacks *pAllocator);
	
	// must be called from any thread.
	void show() noexcept;
	
	// safe to call outside of main thread.
	bool shouldClose() noexcept;
	
	// safe to call outside of main thread.
	~rglfwWindow();
	
	
	// may be called from any thread. 
	// the effect is delayed till pollManualWindowEvents()
	void activateRawPointer() noexcept;
	
	// may be called from any thread. 
	// the effect is delayed till pollManualWindowEvents()
	void deactivateRawPointer() noexcept;
	
	// this function should be called in the main thread's polling loop, preferably after other poll functions. 
	// executes delayed effects that needed to be executed in the main thread.
	void pollManualWindowEvents();
	
	// use inside the resize callback only
	const auto &_getWindowWidth();
	const auto &_getWindowHeight();
	

private:
	
	void _setGlfwResizeCallback();//GLFWframebuffersizefun glfwResizeCallback)
	
	static void _defaultResizeCallback(GLFWwindow *window, int width, int height);
	
	void _setGlfwKeyCallback();
	
	static void _defaultCursorPosCallback(GLFWwindow *window, double x, double y);
	
	void _setGlfwCursorPosCallback();
	
	static void _defaultCursorButtonCallback(GLFWwindow* window, int button, int action, int mods);
	
	void _setGlfwCursorButtonCallback();
	
	//static void _
	
	static void _defaultKeyCallback(GLFWwindow *window,int key,int scanCode,int action,int mod);
	
	
	/* order of locking mutexes (not well decided yet.):
	 * 1._windowUD internal mutexes
	 * 2._optsM
	 */
	
	userData_t _windowUD{};
	GLFWwindow *_hwindow{};
	//GLFWframebuffersizefun _glfwResizeCallback{};
	//GLFWkeyfun _glfwKeyCallback{};
	//GLFWcursorposfun _glfwCursorPosCallback{};
	
	// input management :
	std::array<keyCallback_t,GLFW_KEY_to_array_index(GLFW_KEY_LAST)+1> _keyCallbacks;
	std::array<cursorButtonCallback_t, GLFW_MOUSE_BUTTON_to_array_index(GLFW_MOUSE_BUTTON_8)+1>
	_cursorButtonCallbacks;
	cursorPosCallback_t _cursorPosCalback;
	
	std::mutex _optsM;
	double _camRotateSpeed = 0.7;
	bool _isPointerRaw = false;
	bool _prevIsPointerRaw = false;
	bool _surfaceIsCreated = false;
	bool _isVisible = false;
	bool _prevIsVisible = false;
	
};


struct commonKeyCallbacks
{
	//typedef rglfwWindow window_t;
	
	static void leftCallback(
		double time, uinl::userInputOptions_core *inputOptionsCore,
		int action,int mod);
	
	static void rightCallback(
		double time, uinl::userInputOptions_core *inputOptionsCore,
		int action,int mod);
	
	static void upCallback(
		double time, uinl::userInputOptions_core *inputOptionsCore,
		int action,int mod);
	
	static void downCallback(
		double time, uinl::userInputOptions_core *inputOptionsCore,
		int action,int mod);
	
	static void mouseLeftClickCallback(double time, uinl::userInputOptions_core *ioc,
	int action, int mod);
	
	static void mousePosCallback(
	double time, uinl::userInputOptions_core *inputOptionsCore,
	int x, int y, uinl::rtrc_channel_modes channel_mode);
	
	template<typename windowT>
	static void configure(windowT &window);
};


}
}
