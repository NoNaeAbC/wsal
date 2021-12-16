
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

	[[nodiscard]] bool success() const {
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
	WsalApi usedApi;
	WsalContextCreation usedContext;
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
#if WSAL_GLX == 2
		initGLX();
#endif
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
#if WSAL_GLX == 2
		initGLX();
#endif
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

#ifdef WSAL_GLX

	std::vector<std::string> extensions;

	void getExtensions() {
		std::istringstream extensionString(glXQueryExtensionsString(display->getDisplay(), screenId));
		std::string extension;
		while (std::getline(extensionString, extension, ' ')) {
			extensions.push_back(extension);
		}
	}

	[[nodiscard]] bool supportsGlx() const {
#if WSAL_OGL == 1
		return std::ranges::any_of(extensions, [](const std::string &extension) {
			return extension == "GLX_EXT_create_context_es2_profile";
		});
#elif WSAL_OGL == 2
		return true;
#endif
		return false;
	}

	void initGLX() const {
		std::cout << " hi " << std::endl;
		std::cout << glXQueryExtensionsString(display->getDisplay(), screenId) << std::endl;
		glewInit();
		glxewInit();
	}

#endif

	std::vector<WsalContextCreation> getPossibleContext(WsalApi api) {
		usedApi = api;
		switch (api) {
			case WSAL_API_GL: {
				if
#if WSAL_GLX == 2
						(supportsGlx()) {
					return {WSAL_CONTEXT_EGL, WSAL_CONTEXT_GLX};
				} else if
#endif
						(true) {
					return {WSAL_CONTEXT_EGL};
				}
			}
			default:
				return {WSAL_CONTEXT_NATIVE};
		}
	};

	void selectContext(WsalContextCreation api) {
		usedContext = api;
	};


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
		windowAttributes.event_mask =
				ButtonReleaseMask | ButtonPressMask | KeyPressMask | SubstructureNotifyMask | PointerMotionMask;
		window = XCreateWindow(screen->getDisplay(), screen->getRootWindow(), 500, 500, 800, 600, 2, screen->getDepth(),
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

	bool handleEvent(XEvent event) {
		switch (event.type) {
			case DestroyNotify:
				return false;
				break;
			case MotionNotify: {
				WSALMouseEvent e{};
				e.position_X = event.xmotion.x;
				e.position_Y = event.xmotion.y;
				WSALEvent resultEvent{};
				resultEvent.mouseEvent = e;
				eventHandler->createEvent(resultEvent, MOUSE_POSITION);
				break;
			}
			case ButtonPress: {
				WSALMouseEvent e{};
				e.position_X = event.xbutton.x;
				e.position_Y = event.xbutton.y;
				WSALEvent resultEvent{};
				resultEvent.mouseEvent = e;
				eventHandler->createEvent(resultEvent, BUTTON_PRESSED);
			}
				break;
			case ButtonRelease: {
				WSALMouseEvent e{};
				e.position_X = event.xbutton.x;
				e.position_Y = event.xbutton.y;
				WSALEvent resultEvent;
				resultEvent.mouseEvent = e;
				eventHandler->createEvent(resultEvent, BUTTON_RELEASED);
			}
				break;
			case KeyPress: {
				WSALKeyboardEvent e{};
				e.keycode = event.xkey.keycode;
				WSALEvent resultEvent;
				resultEvent.keyboardEvent = e;
				eventHandler->createEvent(resultEvent, KEY_PRESSED);
			}
				break;
		}
		return true;
	}

	bool pollEvents() {
		XEvent event;
		while (XPending(screen->getDisplay()) != 0) {
			XNextEvent(screen->getDisplay(), &event);
			handleEvent(event);
		}
		return true;
	}

	bool waitForEvents() {
		XEvent event;
		XNextEvent(screen->getDisplay(), &event);
		handleEvent(event);
		pollEvents();
		return true;
	}

	~XlibWindow() {
		if (!isCopy) {
		}
	}

#ifdef WSAL_EGL

	std::vector<int> attributes;

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
		attributes.push_back(EGL_NONE);
		EGLSurface surface = eglCreateWindowSurface(display, config, window, attributes.data());
		if (surface == EGL_NO_SURFACE) {
			ASSERT_EGL();
		}
		return surface;
	}

#else
	bool supportsEgl() {
		return false;
	}
#endif

#ifdef WSAL_GLX


#endif

#ifdef WSAL_VULKAN


#endif

};


struct WsalXlibContext {
	XlibDisplay *display;
	XlibScreen *screen;

};

#endif
