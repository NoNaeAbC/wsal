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
	BUTTON_PRESSED
};

struct WSALMouseEvent{
	int position_X;
	int position_Y;
};

struct WSALKeyboardEvent{
	unsigned int keycode;
};

union WSALEvent{
	WSALMouseEvent mouseEvent;
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
