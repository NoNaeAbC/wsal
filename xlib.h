
#ifdef WSAL_XLIB

#include <iostream>
#include <cstdio>
#include <cstring>

class XlibDisplay {
	Display *display = nullptr;
	bool isCopy = true;
public:
	XlibDisplay() {
		display = XOpenDisplay(nullptr);
		isCopy = false;
	}

	[[nodiscard]] bool success()const{
		return display != nullptr;
	}

	[[nodiscard]] Display *getDisplay() const {
		return display;
	}

	[[nodiscard]]int getDefaultScreenNumber() const {
		return XDefaultScreen(display);
	}

	~XlibDisplay() {
		if (!isCopy) {
			XCloseDisplay(display);
		}
	}

};

class XlibScreen {
	const int screenId;
	const XlibDisplay *display;
	const unsigned long blackPixel;
	const unsigned long whitePixel;
	const Screen *screen;
	const int depth;
	const GC graphicsContext;
	Visual *visual;
	const Window rootWindow;
public:
	XlibScreen(int screenNumber, XlibDisplay *xlibDisplay) : screenId(screenNumber), display(xlibDisplay),
															 blackPixel(BlackPixel(display->getDisplay(), screenId)),
															 whitePixel(WhitePixel(display->getDisplay(), screenId)),
															 screen(ScreenOfDisplay(display->getDisplay(), screenId)),
															 depth(DefaultDepth(display->getDisplay(), screenId)),
															 graphicsContext(
																	 DefaultGC(display->getDisplay(), screenId)),
															 visual(DefaultVisual(display->getDisplay(), screenId)),
															 rootWindow(RootWindow(display->getDisplay(), screenId)) {
	}

	explicit XlibScreen(XlibDisplay *xlibDisplay) : screenId(xlibDisplay->getDefaultScreenNumber()),
													display(xlibDisplay),
													blackPixel(BlackPixel(display->getDisplay(), screenId)),
													whitePixel(WhitePixel(display->getDisplay(), screenId)),
													screen(ScreenOfDisplay(display->getDisplay(), screenId)),
													depth(DefaultDepth(display->getDisplay(), screenId)),
													graphicsContext(DefaultGC(display->getDisplay(), screenId)),
													visual(DefaultVisual(display->getDisplay(), screenId)),
													rootWindow(RootWindow(display->getDisplay(), screenId)) {

	}

	[[nodiscard]] Display *getDisplay() const {
		return display->getDisplay();
	}

	[[nodiscard]]unsigned long getBlackPixel() const {
		return blackPixel;
	}

	[[nodiscard]]unsigned long getWhitePixel() const {
		return whitePixel;
	}

	[[nodiscard]]int getDepth() const {
		return depth;
	}

	[[nodiscard]]GC getGraphicsContext() const {
		return graphicsContext;
	}

	[[nodiscard]]Visual *getVisual() const {
		return visual;
	}

	[[nodiscard]]Window getRootWindow() const {
		return rootWindow;
	}


};

template<class T>
class XlibWindow {
private:
	T *eventHandler;
	Window window;
	bool isCopy = true;
	const XlibScreen *screen;
public:
	explicit XlibWindow(const XlibScreen *displayScreen) : screen(displayScreen) {
		unsigned long valueMask = CWBackPixel | CWBorderPixel | CWEventMask;
		XSetWindowAttributes windowAttributes;
		windowAttributes.background_pixel = screen->getWhitePixel();
		windowAttributes.border_pixel = screen->getBlackPixel();
		windowAttributes.event_mask = ButtonPressMask | KeyPressMask | SubstructureNotifyMask;
		window = XCreateWindow(screen->getDisplay(), screen->getRootWindow(), 200, 200, 350, 250, 2, screen->getDepth(),
							   InputOutput, screen->getVisual(), valueMask, &windowAttributes);
	}

	void show() {
		XSizeHints size;
		size.flags = USPosition | USSize;
		XSetWMNormalHints(screen->getDisplay(), window, &size);
		XWMHints wmHints;
		wmHints.initial_state = NormalState;
		wmHints.flags = StateHint;
		XSetWMHints(screen->getDisplay(), window, &wmHints);
		XTextProperty windowName;
		const char *name = "hi";
		char *c_name = (char *) malloc((strlen(name) + 1) * sizeof(char));
		c_name[strlen(name)] = 0;
		memcpy(c_name, name, strlen(name));
		XStringListToTextProperty(&c_name, 1, &windowName);
		free(c_name);
		XSetWMName(screen->getDisplay(), window, &windowName);
		XMapWindow(screen->getDisplay(), window);
	}

	bool pollEvents() {
		XEvent event;
		while (XPending(screen->getDisplay()) != 0) {
			XNextEvent(screen->getDisplay(), &event);
			switch (event.type) {
				case DestroyNotify:
					return false;
					break;
				case ButtonPress:
					eventHandler->createEvent(0, BUTTON_PRESSED);
					break;
			}
		}
		return true;
	}

	bool waitForEvents() {
		XEvent event;
		XNextEvent(screen->getDisplay(), &event);
		switch (event.type) {
			case DestroyNotify:
				return false;
				break;
			case ButtonPress:
				eventHandler->createEvent(0, BUTTON_PRESSED);
				break;
		}
		return true;
	}

	~XlibWindow() {
		if (!isCopy) {
		}
	}

#ifdef WSAL_EGL

	bool supportsEgl() {
		return true;
	}

	EGLDisplay getEglDisplay() {
		EGLDisplay eglDisplay = eglGetDisplay(screen->getDisplay());
		EGLint versionMajor;
		EGLint versionMinor;
		EGLBoolean errorCode = eglInitialize(eglDisplay, &versionMajor, &versionMinor);
		if (errorCode == EGL_FALSE) {
			ASSERT_EGL();
		}
		std::cout << "EGL Version " << versionMajor << "." << versionMinor << std::endl;
		return eglDisplay;
	}

	EGLSurface createEglSurface(EGLDisplay display, EGLConfig config) {
		EGLSurface surface = eglCreateWindowSurface(display, config, window, nullptr);
		if (surface == EGL_NO_SURFACE) {
			ASSERT_EGL();
		}
		return surface;
	}


#endif


#ifdef WSAL_VULKAN


#endif

};


struct WsalXlibContext {
	XlibDisplay *display;
	XlibScreen *screen;

};

#endif
