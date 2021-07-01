

#ifndef WSAL_H
#define WSAL_H

#include <vector>
#include <array>
#include <algorithm>
#include <memory>
//namespace wsal{


#if defined(WSAL_USE_OPEN_GL_ES) && defined(WSAL_USE_OPEN_GL)
#error WSAL can not have both OpenGL and OpenGLes, because libraries interfere
#endif

#if defined(WSAL_USE_OPEN_GL_ES)
#define WSAL_OGL 1
#elif defined(WSAL_USE_OPEN_GL_ES)
#define WSAL_OGL 2
#endif

#ifndef NO_VULKAN

#define WSAL_VULKAN 1

#endif

#ifdef __linux

#ifndef NO_EGL

#define WSAL_EGL 1


#include <EGL/egl.h>
#include <EGL/eglext.h>

#define EGL_ERROR_SWITCH(X)           \
    case X:                           \
        std::cout << #X << std::endl; \
        break;

#define ASSERT_EGL() if(!assertEgl()){std::cout << "In line : " << __LINE__ <<std::endl;}

bool assertEgl() {
	EGLint error = eglGetError();
	switch (error) {
		case EGL_SUCCESS:
			return true;
		EGL_ERROR_SWITCH(EGL_NOT_INITIALIZED)
		EGL_ERROR_SWITCH(EGL_BAD_ACCESS)
		EGL_ERROR_SWITCH(EGL_BAD_ALLOC)
		EGL_ERROR_SWITCH(EGL_BAD_ATTRIBUTE)
		EGL_ERROR_SWITCH(EGL_BAD_CONTEXT)
		EGL_ERROR_SWITCH(EGL_BAD_CONFIG)
		EGL_ERROR_SWITCH(EGL_BAD_CURRENT_SURFACE)
		EGL_ERROR_SWITCH(EGL_BAD_DISPLAY)
		EGL_ERROR_SWITCH(EGL_BAD_SURFACE)
		EGL_ERROR_SWITCH(EGL_BAD_MATCH)
		EGL_ERROR_SWITCH(EGL_BAD_PARAMETER)
		EGL_ERROR_SWITCH(EGL_BAD_NATIVE_PIXMAP)
		EGL_ERROR_SWITCH(EGL_BAD_NATIVE_WINDOW)
		EGL_ERROR_SWITCH(EGL_CONTEXT_LOST)
		default:
			break;
	}
	return false;
}

#undef EGL_ERROR_SWITCH
#endif //EGL


#ifndef NO_GLX

#define WSAL_GLX 2

#include <GL/glew.h>
#include <GL/glxew.h>
#include <GL/glx.h>
#include <GL/glxext.h>
#include <GL/gl.h>
#include <GL/glext.h>

#endif //GLX


#ifndef WSAL_NO_XLIB

#define WSAL_XLIB 2

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#ifdef WSAL_VULKAN

#define VK_USE_PLATFORM_XLIB_KHR

#endif //VULKAN

#endif //XLIB

#ifdef WSAL_VULKAN

#include <vulkan/vulkan.h>
#include <sstream>
#include <optional>

#endif

#endif //LINUX

#ifdef __WIN64__

#include <windows.h>

#define WSAL_WIN 3

#ifndef NO_WGL

#include <KHR/khrplatform.h>
#include "GLEW/glew.h"
#include "GLEW/wglew.h"
#include <GL/gl.h>
#include <GL/wgl.h>
#include <GL/wglext.h>

#include "win32.h"

#endif

#endif

#include "wsal_util.h"

#include "xlib.h"

void printWsalWs(WsalWs ws) {
	switch (ws) {
#if defined(WSAL_XLIB)
		case WSAL_WS_XLIB:
			std::cout << "WSAL_WS_XLIB" << std::endl;
			break;
#endif
#if defined(WSAL_XCB)
			case WSAL_WS_XCB:
				std::cout << "WSAL_WS_XCB" << std::endl;
				break;
#endif
#if defined(WSAL_WAYLAND)
			case WSAL_WS_WAYLAND:
				std::cout << "WSAL_WS_WAYLAND" << std::endl;
				break;
#endif
#if defined(WSAL_WEB)
			case WSAL_WS_WEB:
				std::cout << "WSAL_WS_WEB" << std::endl;
				break;
#endif
#if defined(WSAL_WIN)
			case WSAL_WS_WIN:
				std::cout << "WSAL_WS_WIN" << std::endl;
				break;
#endif
#if defined(WSAL_ANDROID)
			case WSAL_WS_ANDROID:
				std::cout << "WSAL_WS_ANDROID" << std::endl;
				break;
#endif
	}
}

namespace wsal {
	WsalApi globalWsalApiSelection;

	template<class T, const int arrayCount = 0>
	struct VarArray {
		const int count = arrayCount;
		T s[arrayCount];

		inline constexpr VarArray<T, arrayCount + 1> push_back(T val) const {
			VarArray<T, arrayCount + 1> ret;
			for (int i = 0; i < count; i++) {
				ret.s[i] = s[i];
			}
			ret.count = count + 1;
			ret.s[count] = val;
			return ret;
		}

		inline constexpr VarArray<T, arrayCount + 1> push_void() const {
			VarArray<T, arrayCount + 1> ret;
			for (int i = 0; i < count; i++) {
				ret.s[i] = s[i];
			}
			ret.count = count;
			return ret;
		}

		[[nodiscard]] inline constexpr int size() const {
			return count;
		}

		inline constexpr T &operator[](int location) {
			return s[location];
		}

		inline constexpr T *data() {
			return s;
		}
	};

}

bool hasWayland() {
	std::string session = getenv("XDG_SESSION_TYPE");
	std::cout << session << std::endl;
	return getenv("WAYLAND_DISPLAY") != nullptr;
}

bool hasVulkan() {
	VkApplicationInfo testInfo;
	testInfo.apiVersion = VK_VERSION_1_0;
	testInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 0);
	testInfo.engineVersion = VK_MAKE_VERSION(0, 0, 0);
	testInfo.pApplicationName = "";
	testInfo.pEngineName = "";
	testInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	testInfo.pNext = nullptr;
	VkInstance instance;
	VkInstanceCreateInfo instanceCreateInfo;
	instanceCreateInfo.pNext = nullptr;
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.enabledExtensionCount = 0;
	instanceCreateInfo.enabledLayerCount = 0;
	instanceCreateInfo.flags = 0;
	instanceCreateInfo.pApplicationInfo = &testInfo;
	instanceCreateInfo.ppEnabledExtensionNames = nullptr;
	instanceCreateInfo.ppEnabledLayerNames = nullptr;
	VkResult result = vkCreateInstance(&instanceCreateInfo, nullptr, &instance);
	bool returnValue = result == VK_SUCCESS;
	vkDestroyInstance(instance, nullptr);
	return returnValue;
}

/*
 * May be expensive
 */
std::optional<std::vector<WsalWs>> selectApi(WsalApi api) {
	wsal::globalWsalApiSelection = api;
#if defined(WSAL_WIN)
#elif defined(__linux__)
	switch (api) {
		case WSAL_API_GL: {
			std::vector<WsalWs> validWs;
#if defined(WSAL_WAYLAND)
			if (hasWayland()) {
				validWs.push_back(WSAL_WS_WAYLAND);
			}
#endif
			validWs.push_back(WSAL_WS_XLIB);
			return std::optional<std::vector<WsalWs>>{validWs};
		}
		case WSAL_NO_API: {
			std::vector<WsalWs> validWs;
#if defined(WSAL_WAYLAND)
			if (hasWayland()) {
				validWs.push_back(WSAL_WS_WAYLAND);
			}
#endif
#if defined(WSAL_XCB)
			validWs.push_back(WSAL_WS_XCB);
#endif
#if defined(WSAL_XLIB)
			validWs.push_back(WSAL_WS_XLIB);
#endif
			return std::optional<std::vector<WsalWs>>{validWs};
		}
		case WSAL_API_VULKAN:
			if (hasVulkan()) {
				std::vector<WsalWs> validWs;
#if defined(WSAL_WAYLAND)
				if (hasWayland()) {
					validWs.push_back(WSAL_WS_WAYLAND);
				}
#endif
#if defined(WSAL_XCB)
				validWs.push_back(WSAL_WS_XCB);
#endif
#if defined(WSAL_XLIB)
				validWs.push_back(WSAL_WS_XLIB);
#endif
				return std::optional<std::vector<WsalWs>>{validWs};
			}
			[[fallthrough]];
		case WSAL_API_DX11:
		case WSAL_API_DX12:
		case WSAL_API_GPU_WEB:
			return std::nullopt;
	}
#elif defined(__emscripten)
#elif defined(ANDROID)
#endif
	return std::nullopt;
}

template<class T>
class WsalContext {
	WsalWs windowSystem;
	bool isCopy = true;
	void *wsContext = nullptr;

	bool init() {
		switch (windowSystem) {
#ifdef WSAL_XLIB
			case WSAL_WS_XLIB:
				wsContext = (void *) new WsalXlibContext;
				((WsalXlibContext *) wsContext)->display = new XlibDisplay;
				if (!((WsalXlibContext *) wsContext)->display->success()) {
					return false;
				}
				((WsalXlibContext *) wsContext)->screen = new XlibScreen(((WsalXlibContext *) wsContext)->display);
				break;
#endif
			default:
				throw WSAL_Exception(3245);
		}
		return true;
	}

public:

	WsalContext(const std::vector<WsalWs> &windowSystems) {
		isCopy = false;
		for (auto possibleWindowSystem : windowSystems) {
			windowSystem = possibleWindowSystem;
			bool success = init();
			if (success) {
				return;
			}
		}
		throw WSAL_Exception(2359);
	}

	WsalContext(WsalWs ws) : windowSystem(ws) {
		isCopy = false;
		init();
	}

	WsalWs getWindowSystem() {
		return windowSystem;
	}

	~WsalContext() {
		if (!isCopy) {
			switch (windowSystem) {
#ifdef WSAL_XLIB
				case WSAL_WS_XLIB:
					delete ((WsalXlibContext *) wsContext)->screen;
					delete ((WsalXlibContext *) wsContext)->display;
					delete ((WsalXlibContext *) wsContext);
					break;
#endif
			}
		}
	}

	[[nodiscard]] void *getWsContext() const {
		return wsContext;
	}
};

template<class T>
class WsalWindow {
	T eventHandler;
	const WsalWs windowSystem;
	void *window = nullptr;
	WsalContext<T> *wsalContext;

	void init() {

		switch (windowSystem) {
#ifdef WSAL_XLIB
			case WSAL_WS_XLIB:
				window = (void *) new XlibWindow<T>(((WsalXlibContext *) wsalContext->getWsContext())->screen);
				((XlibWindow<T> *) window)->show();
				break;
#endif
			default:
				throw WSAL_Exception(2394);
		}
	}

public:

	WsalWindow(WsalWs ws, WsalContext<T> *context) : windowSystem(ws), wsalContext(context) {
		init();
	}

	explicit WsalWindow(std::vector<WsalWs> ws, WsalContext<T> *context) : windowSystem(ws[0]), wsalContext(context) {
		init();
	}

	~WsalWindow() {
		switch (windowSystem) {
#ifdef WSAL_XLIB
			case WSAL_WS_XLIB:
				delete ((XlibWindow<T> *) window);
				break;
#endif
		}
	}

	const T *getEventHandler() {
		return &eventHandler;
	}

	bool pollEvents() {
		switch (windowSystem) {
#ifdef WSAL_XLIB
			case WSAL_WS_XLIB:
				return ((XlibWindow<T> *) window)->pollEvents();
#endif
		}
		return false;
	}

	bool waitForEvents() {
		switch (windowSystem) {
#ifdef WSAL_XLIB
			case WSAL_WS_XLIB:
				return ((XlibWindow<T> *) window)->waitForEvents();
#endif
		}
		return false;
	}

#ifdef WSAL_EGL

	EGLDisplay eglDisplay;

	void initEglDisplay() {
		switch (windowSystem) {
			case WSAL_WS_XLIB:
				eglDisplay = ((XlibWindow<T> *) window)->getEglDisplay();
				break;
		}
	}

	[[nodiscard]] EGLDisplay getEglDisplay() const {
		return eglDisplay;
	}

	[[nodiscard]] EGLSurface createEglSurface(EGLDisplay display, EGLConfig config) {
		switch (windowSystem) {
			case WSAL_WS_XLIB:
				return ((XlibWindow<T> *) window)->createEglSurface(display, config);
				break;
		}
		throw WSAL_Exception(1434);
	}

#endif

};

template<class T>
std::unique_ptr<WsalWindow<T>> createWindow(WsalContext<T> *context) {
	return std::make_unique<WsalWindow<T>>(context->getWindowSystem(), context);
}


#ifdef WSAL_EGL

namespace WSAL {
	void printEglVersion(EGLDisplay display) {
		const char *versionString = eglQueryString(display, EGL_VERSION);
		if (versionString == nullptr) {
			ASSERT_EGL();
		} else {
			std::cout << versionString << std::endl;
		}
	}

	std::pair<int, int> getEglVersion(EGLDisplay display) {
		const char *versionString = eglQueryString(display, EGL_VERSION);
		if (versionString == nullptr) {
			ASSERT_EGL();
		} else {
			std::vector<int> versionNumber;
			versionNumber.reserve(2);
			std::istringstream version(versionString);
			std::string version_;
			while (std::getline(version, version_, '.')) {
				versionNumber.push_back(stoi(version_));
			}
			return std::make_pair(versionNumber[0], versionNumber[1]);
		}
		return std::make_pair(0, 0);
	}

	void printEglVendor(EGLDisplay display) {
		const char *vendorString = eglQueryString(display, EGL_VENDOR);
		if (vendorString == nullptr) {
			ASSERT_EGL();
		} else {
			std::cout << vendorString << std::endl;
		}
	}

	void printEglApis(EGLDisplay display) {
		const char *apiString = eglQueryString(display, EGL_CLIENT_APIS);
		if (apiString == nullptr) {
			ASSERT_EGL();
		} else {
			std::cout << apiString << std::endl;
		}
	}

	void printEglExtensions(EGLDisplay display) {
		const char *extensionString = eglQueryString(display, EGL_EXTENSIONS);
		if (extensionString == nullptr) {
			ASSERT_EGL();
		} else {
			std::cout << extensionString << std::endl;
		}
	}

	int getSupportedEglApis(EGLDisplay display) {
		int supportedApis = 0;
		const char *apiString = eglQueryString(display, EGL_CLIENT_APIS);
		if (apiString == nullptr) {
			ASSERT_EGL();
		} else {
			std::istringstream apis(apiString);
			std::string api;
			while (std::getline(apis, api, ' ')) {
				if (api == "OpenGL") {
					supportedApis += EGL_OPENGL_BIT;
				} else if (api == "OpenGL_ES") {
					supportedApis += EGL_OPENGL_ES2_BIT;
				} else if (api == "OpenVG") {
					supportedApis += EGL_OPENVG_BIT;
				}
			}
		}
		return supportedApis;
	}

	std::vector<std::string> getSupportedEglExtensions(EGLDisplay display) {
		const char *extensionString = eglQueryString(display, EGL_EXTENSIONS);
		std::vector<std::string> supportedExtensions;
		if (extensionString == nullptr) {
			ASSERT_EGL();
		} else {
			std::istringstream extensions(extensionString);
			std::string extension;
			while (std::getline(extensions, extension, ' ')) {
				supportedExtensions.push_back(extension);
			}
		}
		return supportedExtensions;
	}

#define WSAL_EGL_EXTENSION_COLORSPACE 0
	typedef std::array<bool, 1> WsalEglExtensionId;

	WsalEglExtensionId getSupportedEglExtensionsById(const std::vector<std::string> &extensions) {
		WsalEglExtensionId extensionsIds = {false};
		for (const auto &s : extensions) {
			if (s == "EGL_KHR_gl_colorspace") {
				extensionsIds[WSAL_EGL_EXTENSION_COLORSPACE] = true;
			}
		}
		return extensionsIds;
	}

	void bindEglApi(EGLint api) {
		EGLBoolean error = eglBindAPI(api);
		if (error == EGL_FALSE) {
			ASSERT_EGL();
		}
	}

	static EGLint const attribute_list[] = {
			EGL_RED_SIZE, 1,
			EGL_GREEN_SIZE, 1,
			EGL_BLUE_SIZE, 1,
			EGL_NONE
	};

	std::vector<EGLConfig> getEglConfigs(EGLDisplay display) {
		int configCount;
		EGLBoolean error = eglGetConfigs(display, nullptr, 0, &configCount);
		if (error == EGL_FALSE) {
			ASSERT_EGL();
			return {};
		}
		std::vector<EGLConfig> configs(configCount);
		std::cout << "EGL config count " << configCount << std::endl;
		error = eglGetConfigs(display, configs.data(), configCount, &configCount);
		if (error == EGL_FALSE) {
			ASSERT_EGL();
			return {};
		}
		return configs;

	}

	void cleanupEgl(EGLDisplay display) {
		EGLBoolean error = eglTerminate(display);
		if (error == EGL_FALSE) {
			ASSERT_EGL();
		}
	}

#define WSAL_EGL_GET_ATTRIBUTE(X) EGLint getEglAttribute_ ## X (EGLDisplay display, EGLConfig config){\
                                EGLint value;EGLBoolean error = eglGetConfigAttrib(display,config, X, &value);\
                                if (error == EGL_FALSE) {\
                                    ASSERT_EGL();\
                                }\
                                return value;}

	WSAL_EGL_GET_ATTRIBUTE(EGL_ALPHA_SIZE)

	WSAL_EGL_GET_ATTRIBUTE(EGL_ALPHA_MASK_SIZE)

	WSAL_EGL_GET_ATTRIBUTE(EGL_BIND_TO_TEXTURE_RGB)

	WSAL_EGL_GET_ATTRIBUTE(EGL_BIND_TO_TEXTURE_RGBA)

	WSAL_EGL_GET_ATTRIBUTE(EGL_BLUE_SIZE)

	WSAL_EGL_GET_ATTRIBUTE(EGL_BUFFER_SIZE)

	WSAL_EGL_GET_ATTRIBUTE(EGL_COLOR_BUFFER_TYPE)

	WSAL_EGL_GET_ATTRIBUTE(EGL_CONFIG_CAVEAT)

	WSAL_EGL_GET_ATTRIBUTE(EGL_CONFIG_ID)

	WSAL_EGL_GET_ATTRIBUTE(EGL_CONFORMANT)

	WSAL_EGL_GET_ATTRIBUTE(EGL_DEPTH_SIZE)

	WSAL_EGL_GET_ATTRIBUTE(EGL_GREEN_SIZE)

	WSAL_EGL_GET_ATTRIBUTE(EGL_LEVEL)

	WSAL_EGL_GET_ATTRIBUTE(EGL_LUMINANCE_SIZE)

	WSAL_EGL_GET_ATTRIBUTE(EGL_MAX_PBUFFER_WIDTH)

	WSAL_EGL_GET_ATTRIBUTE(EGL_MAX_PBUFFER_HEIGHT)

	WSAL_EGL_GET_ATTRIBUTE(EGL_MAX_PBUFFER_PIXELS)

	WSAL_EGL_GET_ATTRIBUTE(EGL_MAX_SWAP_INTERVAL)

	WSAL_EGL_GET_ATTRIBUTE(EGL_MIN_SWAP_INTERVAL)

	WSAL_EGL_GET_ATTRIBUTE(EGL_NATIVE_RENDERABLE)

	WSAL_EGL_GET_ATTRIBUTE(EGL_NATIVE_VISUAL_ID)

	WSAL_EGL_GET_ATTRIBUTE(EGL_NATIVE_VISUAL_TYPE)

	WSAL_EGL_GET_ATTRIBUTE(EGL_RED_SIZE)

	WSAL_EGL_GET_ATTRIBUTE(EGL_RENDERABLE_TYPE)

	WSAL_EGL_GET_ATTRIBUTE(EGL_SAMPLE_BUFFERS)

	WSAL_EGL_GET_ATTRIBUTE(EGL_SAMPLES)

	WSAL_EGL_GET_ATTRIBUTE(EGL_STENCIL_SIZE)

	WSAL_EGL_GET_ATTRIBUTE(EGL_SURFACE_TYPE)

	WSAL_EGL_GET_ATTRIBUTE(EGL_TRANSPARENT_TYPE)

	WSAL_EGL_GET_ATTRIBUTE(EGL_TRANSPARENT_RED_VALUE)

	WSAL_EGL_GET_ATTRIBUTE(EGL_TRANSPARENT_GREEN_VALUE)

	WSAL_EGL_GET_ATTRIBUTE(EGL_TRANSPARENT_BLUE_VALUE)

#define WSAL_EGL_PRINT_ATTRIBUTE(DISPLAY, CONFIG, X) std::cout << # X << " : " << WSAL::getEglAttribute_ ## X (DISPLAY, CONFIG) << std::endl

	struct EglColorSize {
		EGLint r;
		EGLint g;
		EGLint b;
		EGLint a;
	};

	EglColorSize fillEglColorSize(EGLDisplay display, EGLConfig config) {
		return {getEglAttribute_EGL_RED_SIZE(display, config),
				getEglAttribute_EGL_GREEN_SIZE(display, config),
				getEglAttribute_EGL_BLUE_SIZE(display, config),
				getEglAttribute_EGL_ALPHA_SIZE(display, config)};
	}

	std::vector<EGLint> createEglConfigRestrictions() {
		return std::vector<EGLint>();
	};


	void addEglColorSizeRestriction(std::vector<EGLint> *restrictions, const EglColorSize color) {
		restrictions->push_back(EGL_RED_SIZE);
		restrictions->push_back(color.r);
		restrictions->push_back(EGL_GREEN_SIZE);
		restrictions->push_back(color.g);
		restrictions->push_back(EGL_BLUE_SIZE);
		restrictions->push_back(color.b);
		restrictions->push_back(EGL_ALPHA_SIZE);
		restrictions->push_back(color.a);
	}

	void addEglApiRestriction(std::vector<EGLint> *restrictions, const EGLint apis) {
		restrictions->push_back(EGL_CONFORMANT);
		restrictions->push_back(apis);
	}

	void addEglDepthBufferRestriction(std::vector<EGLint> *restrictions, const EGLint bits) {
		restrictions->push_back(EGL_CONFORMANT);
		restrictions->push_back(bits);
	}


	void finalizeEglRestriction(std::vector<EGLint> *restrictions) {
		restrictions->push_back(EGL_NONE);
	}

	EGLConfig chooseOptimalEglConfig(EGLDisplay display, const std::vector<EGLint> *restrictions) {
		EGLint numConfigs;
		EGLConfig retConfig;
		EGLBoolean error = eglChooseConfig(display, restrictions->data(), &retConfig, 1, &numConfigs);
		std::cout << "Usable configs : " << numConfigs << std::endl;
		if (error == EGL_FALSE) {
			ASSERT_EGL();
		}
		return retConfig;
	}

	struct EglVersion {
		int major = 0;
		int minor = 0;

		EglVersion() = default;

		EglVersion(int major_, int minor_) : major(major_), minor(minor_) {}

		[[nodiscard]] std::vector<int> toAttributeVersion() const {
			std::vector<int> ret;
			ret.push_back(EGL_CONTEXT_MAJOR_VERSION);
			ret.push_back(major);
			ret.push_back(EGL_CONTEXT_MINOR_VERSION);
			ret.push_back(minor);
			return ret;
		}

		bool operator==(const EglVersion &other) const {
			return (major == other.major) && (minor == other.minor);
		}

		std::strong_ordering operator<=>(const EglVersion &other) const {
			if ((major == other.major) && (minor == other.minor)) {
				return std::strong_ordering::equal;
			}
			if ((major < other.major) || ((major == other.major) && (minor < other.minor))) {
				return std::strong_ordering::less;
			}
			return std::strong_ordering::greater;
		}

	};

	EGLContext
	createEglOpenGlEsContext(EGLDisplay display, EGLConfig config, EglVersion *maxVersion, bool debug = false) {
		std::array<EglVersion, 5> versions{{{3, 2}, {3, 1}, {3, 0}, {2, 0}, {1, 1}}};
		for (EglVersion selectedVersion:versions) {
			if (*maxVersion < selectedVersion) {
				continue;
			}
			std::cout << " _ " << selectedVersion.major << "." << selectedVersion.minor << std::endl;
			auto attributes = selectedVersion.toAttributeVersion();
			if (debug) {
				attributes.push_back(EGL_CONTEXT_OPENGL_DEBUG);
				attributes.push_back(EGL_TRUE);
			}
			attributes.push_back(EGL_NONE);
			EGLContext context = eglCreateContext(display, config, EGL_NO_CONTEXT, attributes.data());
			if (context != EGL_NO_CONTEXT) {
				*maxVersion = selectedVersion;
				return context;
			}
		}
		throw WSAL_Exception(13);
	}

	void destroyEglSurface(EGLDisplay display, EGLSurface surface) {
		EGLBoolean error = eglDestroySurface(display, surface);
		if (error == EGL_TRUE) {
			ASSERT_EGL();
		}
	}

	void destroyEglContext(EGLDisplay display, EGLContext context) {
		EGLBoolean error = eglDestroyContext(display, context);
		if (error == EGL_TRUE) {
			ASSERT_EGL();
		}
	}

	void bindEglSurfaceToContext(EGLDisplay display, EGLSurface surface, EGLContext context) {
		EGLBoolean error = eglMakeCurrent(display, surface, surface, context);
		if (error == EGL_TRUE) {
			ASSERT_EGL();
		}
	}

	void draw(EGLDisplay display, EGLSurface surface) {
		EGLBoolean error = eglWaitClient();
		if (error == EGL_TRUE) {
			ASSERT_EGL();
		}
		error = eglSwapBuffers(display, surface);
		if (error == EGL_TRUE) {
			ASSERT_EGL();
		}
	}

	int getEglOpenGlEsVersion(EGLDisplay display, EGLContext context) {
		int value;
		EGLBoolean error = eglQueryContext(display, context, EGL_CONTEXT_CLIENT_TYPE, &value);
		if (error == EGL_FALSE) {
			ASSERT_EGL();
			return 0;
		}
		if (value != EGL_OPENGL_ES_API) {
			return 0;
		}

		error = eglQueryContext(display, context, EGL_CONTEXT_CLIENT_VERSION, &value);
		if (error == EGL_FALSE) {
			ASSERT_EGL();
			return 0;
		}
		return value;
	}

	class EglContext {
		int eglVersionMajor;
		int eglVersionMinor;
		EglVersion openGlEsVersion = {3, 2};
		EGLDisplay display;
		EGLSurface surface = nullptr;
		EGLContext context = nullptr;
		bool isCopy = true;
		std::vector<std::string> extensions;
		WsalEglExtensionId extensionIds;
		std::vector<EGLint> restrictions;
		EGLConfig usedConfig = nullptr;
		bool isDebugContext = false;
	public:
		template<class T>
		EglContext(WsalWindow<T> *window) {
			window->initEglDisplay();
			display = window->getEglDisplay();
			isCopy = false;
			printEglVersion(display);
			auto version = getEglVersion(display);
			eglVersionMajor = version.first;
			eglVersionMinor = version.second;
			extensions = getSupportedEglExtensions(display);
			extensionIds = getSupportedEglExtensionsById(extensions);
		}

		void printEglExtensions() {
			for (const auto &extension : extensions) {
				std::cout << extension << std::endl;
			}
		}

		void addDebugCapabilities() {
			isDebugContext = true;
		}

		void addColorSizeRestriction(const EglColorSize color) {
			addEglColorSizeRestriction(&restrictions, color);
		}

		void addApiRestriction() {
#if WSAL_OGL == 1
			addEglApiRestriction(&restrictions, EGL_OPENGL_ES_API);
#elif WSAL_OGL == 2
			addEglApiRestriction(&restrictions, EGL_OPENGL_API);
#endif
		}

		void addDepthBufferRestriction(const EGLint bits) {
			addEglDepthBufferRestriction(&restrictions, bits);
		}

		void addApiPreference(const EglVersion api) {
			openGlEsVersion = api;
		}

		void createContext() {
			finalizeEglRestriction(&restrictions);
			usedConfig = chooseOptimalEglConfig(display, &restrictions);
			context = createEglOpenGlEsContext(display, usedConfig, &openGlEsVersion, isDebugContext);
		}

		template<class T>
		void createSurface(WsalWindow<T> *window) {
			surface = window->createEglSurface(display, usedConfig);
			bindEglSurfaceToContext(display, surface, context);
		}

		void drawToSurface() {
			draw(display, surface);
		}

		int getGLESMajorVersion() {
			int conformantAPIS = getEglAttribute_EGL_CONFORMANT(display, usedConfig);
			(void) conformantAPIS;
			return getEglOpenGlEsVersion(display, context);
		}

		[[nodiscard]] EglVersion getGLESVersion() const {
			return openGlEsVersion;
		}

		void printVendor() {
			printEglVendor(display);
		}

		~EglContext() {
			if (!isCopy) {
				if (surface != nullptr) {
					destroyEglSurface(display, surface);
				}
				if (context != nullptr) {
					destroyEglContext(display, context);
				}
				cleanupEgl(display);
			}
		}
	};

};

#endif

template<WsalContextCreation contextType>
struct WsalOpenGlContext {

	void createContext() {

	}
};

struct WsalVulkanContext {

};


//}
#endif //WSAL_H
