//
// Created by af on 18.04.21.
//

#ifndef WSAL_WSAL_UTIL_H
#define WSAL_WSAL_UTIL_H

#include <exception>


#define WSAL_EGL_ERROR(X) WSAL_ERROR_ ## X

enum WsalErrorCodes {
	WSAL_EGL_ERROR(EGL_NOT_INITIALIZED),
	WSAL_EGL_ERROR(EGL_BAD_ACCESS),
	WSAL_EGL_ERROR(EGL_BAD_ALLOC),
	WSAL_EGL_ERROR(EGL_BAD_ATTRIBUTE),
	WSAL_EGL_ERROR(EGL_BAD_CONTEXT),
	WSAL_EGL_ERROR(EGL_BAD_CONFIG),
	WSAL_EGL_ERROR(EGL_BAD_CURRENT_SURFACE),
	WSAL_EGL_ERROR(EGL_BAD_DISPLAY),
	WSAL_EGL_ERROR(EGL_BAD_SURFACE),
	WSAL_EGL_ERROR(EGL_BAD_MATCH),
	WSAL_EGL_ERROR(EGL_BAD_PARAMETER),
	WSAL_EGL_ERROR(EGL_BAD_NATIVE_PIXMAP),
	WSAL_EGL_ERROR(EGL_BAD_NATIVE_WINDOW),
	WSAL_EGL_ERROR(EGL_CONTEXT_LOST),
};


enum WSALEventTypes {
	KEY_PRESSED,
	KEY_RELEASED,
	BUTTON_PRESSED,
	BUTTON_RELEASED,
	MOUSE_POSITION
};

struct WSALMouseEvent {
	int position_X;
	int position_Y;
};

struct WSALKeyboardEvent {
	unsigned int keycode;
};

union WSALEvent {
	WSALMouseEvent mouseEvent;
	WSALKeyboardEvent keyboardEvent;
};


enum WsalApi {
	WSAL_NO_API,
	WSAL_API_GL,//GL_ES is controlled by preprocessor flags
	WSAL_API_VULKAN,
	WSAL_API_DX11,
	WSAL_API_DX12,
	WSAL_API_GPU_WEB,
};

enum WsalContextCreation {
	WSAL_CONTEXT_NATIVE,
	WSAL_CONTEXT_EGL,
	WSAL_CONTEXT_GLX,
	WSAL_CONTEXT_WGL,
};

/*
 * Not ABI save or stable, consider use in header files as undefined behaviour!
 */
enum WsalWs {
#if defined(WSAL_XLIB)
	WSAL_WS_XLIB,
#endif
#if defined(WSAL_XCB)
	WSAL_WS_XCB,
#endif
#if defined(WSAL_WAYLAND)
	WSAL_WS_WAYLAND,
#endif
#if defined(WSAL_WEB)
	WSAL_WS_WEB,
#endif
#if defined(WSAL_WIN)
	WSAL_WS_WIN,
#endif
#if defined(WSAL_ANDROID)
	WSAL_WS_ANDROID,
#endif
};

class WSAL_Exception : std::exception {
public:
	int id;

	explicit WSAL_Exception(int e_id) : id(e_id) {}
};

#ifdef __EMSCRIPTEN__

#define NO_VULKAN
#define NO_EGL
#define WSAL_NO_XLIB
#define NO_VULKAN

#endif


#endif //WSAL_WSAL_UTIL_H
