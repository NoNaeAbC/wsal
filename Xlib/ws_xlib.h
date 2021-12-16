//
// Created by af on 10.08.21.
//

#ifndef WSAL_WS_XLIB_H
#define WSAL_WS_XLIB_H

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <vector>
#include <iostream>
#include <memory>
#include <exception>
#include <cstring>

#if defined(VK_USE_PLATFORM_XLIB_KHR) || 1

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_xlib.h>

#endif


namespace X {

	template<int exception>
	struct X_Exception : std::exception {
	};

#define X_NO_DISPLAY X_Exception<1>()

	typedef unsigned long X_Color;
	typedef int X_Depth;
	typedef XSetWindowAttributes X_WindowAttributes;

	enum X_ColorName {
		X_ColorBlack,
		X_ColorWhite,
	};

	struct X_ColorMap {
		Colormap colorMap;
	};

	struct X_Screen {
		int screen_number;
		Screen *screen;

		template<const X_ColorName colorName>
		[[nodiscard]] X_Color getColor() const {
			if constexpr (colorName == X_ColorBlack) {
				return screen->black_pixel;
			}
			if constexpr (colorName == X_ColorWhite) {
				return screen->white_pixel;
			}
		}
	};

	struct X_GraphicsContext {
		GC gc;
	};

	struct X_Window {
		Window window = 0;
	};

	struct X_Point {
		int x = 0;
		int y = 0;

		X_Point() = default;

		X_Point(int x_, int y_) : x(x_), y(y_) {}
	};

#define TO_PARAM_POINT(POINT) (POINT).x, (POINT).y

	struct X_Dimensions {
		unsigned int width = 0;
		unsigned int height = 0;

		X_Dimensions() = default;

		X_Dimensions(int width_, int height_) : width(width_),
												height(height_) {}
	};

	struct X_Rectangle {
		X_Point location;
		X_Dimensions extent;

		X_Rectangle() = default;

		X_Rectangle(X_Point location_, X_Dimensions extent_) : location(
				location_), extent(extent_) {}
	};

#define TO_PARAM_RECT(RECT) (RECT).location.x,(RECT).location.y,(RECT).extent.width,(RECT).extent.height

	struct X_ExposeEvent {
		//X_Window window;
		X_Point upperCorner;
		X_Point lowerCorner;

		explicit X_ExposeEvent(XExposeEvent event) {
			//window.window = event.window;
			upperCorner = {event.x, event.y};
			lowerCorner = {event.x + event.width, event.y + event.height};
		}
	};

	struct X_KeyEvent {
		//X_Window window;

		explicit X_KeyEvent(XKeyEvent) {
			//window.window = event.window;
		}
	};

	struct X_ConfigureEvent {

		explicit X_ConfigureEvent(XConfigureEvent) {
			//window.window = event.window;
		}
	};

	struct X_MouseButtonEvent {

		explicit X_MouseButtonEvent(XButtonEvent) {
			//window.window = event.window;
		}
	};

	struct X_Event {
		XEvent event;

		explicit operator X_ExposeEvent() const {
			return X_ExposeEvent(event.xexpose);
		}

		explicit operator X_MouseButtonEvent() const {
			return X_MouseButtonEvent(event.xbutton);
		}

		explicit operator X_KeyEvent() const {
			return X_KeyEvent(event.xkey);
		}

		explicit operator X_ConfigureEvent() const {
			return X_ConfigureEvent(event.xconfigure);
		}
	};

	struct X_Border {
		X_Color color = 0;
		unsigned int width = 0;

		X_Border() = default;

		explicit X_Border(X_Color color) : color(color) {}

		explicit X_Border(X_Color color, unsigned int width) : color(color),
															   width(width) {}
	};

	/*
	 * This text property should be used just one time. Never pass X_TextProperty as a parameter.
	 * Instead, use std::string. This object works only around X allocation and C strings.
	 */
	struct X_TextProperty {
		XTextProperty property;

		X_TextProperty(const std::vector<std::string> &values) {
			char *cStringList[values.size()];
			for (size_t i = 0; i < values.size(); i++) {
				cStringList[i] = (char *) malloc(values[i].size() + 1);
				cStringList[values[i].size()] = 0;
				std::memcpy(cStringList[i], values[i].c_str(),
							values[i].size());
			}
			XStringListToTextProperty(cStringList, values.size(), &property);
			for (auto string: cStringList) {
				free(string);
			}
		}

		X_TextProperty(const std::string &value) {
			char *string = (char *) malloc(value.size() + 1);
			string[value.size()] = 0;
			std::memcpy(string, value.c_str(), value.size());
			XStringListToTextProperty(&string, 1, &property);
			free(string);
		}

		X_TextProperty(X_TextProperty &&other) {
			property = other.property;
			other.property = {};
		}

		~X_TextProperty() {
			if (property.value != nullptr) {
				XFree(property.value);
			}
		}

	};

	struct X_WMHints {
		XWMHints hints{};

		X_WMHints &initialState(int state) {
			hints.initial_state = state;
			hints.flags |= StateHint;
			return *this;
		}
	};

	struct X_WMSizeHints {
		XSizeHints hints{};

		X_WMSizeHints &position(X_Point pos) {
			hints.x = pos.x;
			hints.y = pos.y;
			hints.flags |= PPosition;
			return *this;
		}

		X_WMSizeHints &position() {
			hints.flags |= USPosition;
			return *this;
		}

		X_WMSizeHints &size(X_Point size) {
			hints.width = size.y;
			hints.height = size.y;
			hints.flags |= PSize;
			return *this;
		}

		X_WMSizeHints &size() {
			hints.flags |= USSize;
			return *this;
		}
	};

	struct X_WindowChange {
		XWindowChanges changes{};
		unsigned long values = 0;

		X_WindowChange &position(X_Point pos) {
			changes.x = pos.x;
			changes.y = pos.y;
			values |= CWX | CWY;
			return *this;
		}

		X_WindowChange &x_position(int x) {
			changes.x = x;
			values |= CWX;
			return *this;
		}

		X_WindowChange &y_position(int y) {
			changes.y = y;
			values |= CWY;
			return *this;
		}
	};

	struct X_Display {
		Display *display;

		std::vector<std::pair<bool (*)(X_ExposeEvent), X_Window>> exposeEvents;
		std::vector<std::pair<bool (*)(X_KeyEvent), X_Window>> keyPressedEvents;
		std::vector<std::pair<bool (*)(
				X_MouseButtonEvent), X_Window>> buttonPressedEvents;
		std::vector<std::pair<bool (*)(
				X_ConfigureEvent), X_Window>> configureEvents;

		X_Display() {
			display = XOpenDisplay(nullptr);
		}

		[[nodiscard]] bool check() const noexcept {
			if (!display) {
				std::cerr << "can't open display" << std::endl;
				return false;
			}
			return true;
		}

		void throwIfCheckFailed() const {
			if (check()) {
				return;
			}
			throw X_NO_DISPLAY;
		}

		[[nodiscard]] X_Screen getScreen(int screen_number) const {
			return {screen_number, XScreenOfDisplay(display, screen_number)};
		}

		[[nodiscard]] X_Screen getScreen() const {
			const int screen_number = XDefaultScreen(display);
			return {screen_number, XScreenOfDisplay(display, screen_number)};
		}

		[[nodiscard]] X_ColorMap getDefaultColorMap(X_Screen screen) const {
			return {XDefaultColormap(display, screen.screen_number)};
		}

		[[nodiscard]] X_GraphicsContext
		getDefaultGraphicsContext(X_Screen screen) const {
			return {XDefaultGC(display, screen.screen_number)};
		}


		[[nodiscard]] X_Depth getDefaultDepth(X_Screen screen) const {
			return XDefaultDepth(display, screen.screen_number);
		}

		[[nodiscard]] X_Window
		createSimpleWindow(X_Window parent, X_Rectangle dimensions,
						   X_Border border, X_Color background,
						   long eventMask = 0) const {
			X_Window window{};
			window.window = XCreateSimpleWindow(display, parent.window,
												TO_PARAM_RECT(dimensions),
												border.width,
												border.color,
												background);
			XSelectInput(display, window.window, eventMask);
			return window;
		}

		[[nodiscard]] X_Window
		createSimpleWindow(X_Screen parent, X_Rectangle dimensions,
						   X_Border border, X_Color background,
						   long eventMask = 0) const {
			X_Window window{};
			window.window = XCreateSimpleWindow(display, RootWindow(display,
																	parent.screen_number),
												TO_PARAM_RECT(dimensions),
												border.width, border.color,
												background);
			XSelectInput(display, window.window, eventMask);
			return window;
		}

		[[nodiscard]] X_Window
		createWindow(X_Screen parent, X_Rectangle dimensions,
					 uint32_t usedAttributes,
					 X_WindowAttributes &attributes) const {
			X_Window window{};
			window.window = XCreateWindow(display, RootWindow(display,
															  parent.screen_number),
										  TO_PARAM_RECT(dimensions), 0,
										  DefaultDepth(display,
													   parent.screen_number),
										  InputOutput,
										  DefaultVisualOfScreen(parent.screen),
										  usedAttributes,
										  &attributes);
			return window;
		}

		void destroyWindow(X_Window window) const {
			XDestroyWindow(display, window.window);
		}

		void setWindowHints(X_Window window, X_WMHints hints) const {
			XSetWMHints(display, window.window, &hints.hints);
		}

		void setWindowSizeHints(X_Window window, X_WMSizeHints hints) const {
			XSetWMNormalHints(display, window.window, &hints.hints);
		}

		void setWindowName(X_Window window, X_TextProperty &&windowName) const {
			XSetWMName(display, window.window, &windowName.property);
		}


		void setIcon(X_Window window, X_TextProperty &&iconName) const {
			XSetWMIconName(display, window.window, &iconName.property);
		}

		void configureWindow(X_Window window, X_WindowChange &change) const {
			XConfigureWindow(display, window.window, change.values,
							 &change.changes);
		}

		void showWindow(X_Window window) const {
			XMapWindow(display, window.window);
		}

		void hideWindow(X_Window window) const {
			XUnmapWindow(display, window.window);
		}

		void addExposeEvent(X_Window window, bool(*f)(X_ExposeEvent)) {
			exposeEvents.emplace_back(f, window);
		}

		void addKeyPressedEvent(X_Window window, bool(*f)(X_KeyEvent)) {
			keyPressedEvents.emplace_back(f, window);
		}

		void
		addButtonPressedEvent(X_Window window, bool(*f)(X_MouseButtonEvent)) {
			buttonPressedEvents.emplace_back(f, window);
		}

		void addConfigureEvent(X_Window window, bool(*f)(X_ConfigureEvent)) {
			configureEvents.emplace_back(f, window);
		}

		bool shouldQuit = false;

		void quit() {
			shouldQuit = true;
		}

		void bell(int volume) const {
			XBell(display, volume);
		}

		void flush() const {
			XFlush(display);
		}

		void runEventLoop() {
			while (!shouldQuit) {
				handleEvent();
			}
		}

		void pollEvent() {
			while (!shouldQuit && XPending(display) != 0) {
				handleEvent();
			}
		}

		void handleEvent() {
			X_Event event{};
			XNextEvent(display, &event.event);
			switch (event.event.type) {
				case Expose:
					for (auto eventHandler: exposeEvents) {
						if (event.event.xexpose.window ==
							eventHandler.second.window ||
							eventHandler.second.window == 0) {
							X_ExposeEvent exposeEvent(event);
							eventHandler.first(exposeEvent);
						}
					}
					break;
				case KeyPress:
					for (auto eventHandler: keyPressedEvents) {
						if (event.event.xkey.window ==
							eventHandler.second.window ||
							eventHandler.second.window == 0) {
							X_KeyEvent keyEvent(event);
							eventHandler.first(keyEvent);
						}
					}
					break;
				case ButtonPress:
					for (auto eventHandler: buttonPressedEvents) {
						if (event.event.xbutton.window ==
							eventHandler.second.window ||
							eventHandler.second.window == 0) {
							X_MouseButtonEvent buttonEvent(event);
							eventHandler.first(buttonEvent);
						}
					}
					break;
				case ConfigureNotify:
					for (auto eventHandler: configureEvents) {
						if (event.event.xconfigure.window ==
							eventHandler.second.window ||
							eventHandler.second.window == 0) {
							X_ConfigureEvent configureEvent(event);
							eventHandler.first(configureEvent);
						}
					}
					break;
			}
		}

		template<bool shouldFill>
		void drawRectangle(X_Window window, X_GraphicsContext gc,
						   X_Rectangle rect) const {
			if constexpr(shouldFill) {
				XFillRectangle(display, window.window, gc.gc,
							   TO_PARAM_RECT(rect));
			} else {
				XDrawRectangle(display, window.window, gc.gc,
							   TO_PARAM_RECT(rect));
			}
		}

		template<bool shouldFill>
		void drawRectangle(X_Window window, X_GraphicsContext gc,
						   std::vector<X_Rectangle> rects) const {
			if constexpr(shouldFill) {
				XFillRectangles(display, window.window, gc.gc,
								(XRectangle *) rects.data(),
								(int) rects.size());
			} else {
				XDrawRectangles(display, window.window, gc.gc,
								(XRectangle *) rects.data(),
								(int) rects.size());
			}
		}

		void drawString(X_Window window, X_GraphicsContext gc, X_Point location,
						const std::string &text) const {
			XDrawString(display, window.window, gc.gc, TO_PARAM_POINT(location),
						text.c_str(), (int) text.length());
		}

		static void initMultiThreading() {
			static bool init = false;
			if (!init) {
				XInitThreads();
				init = true;
			}
		}

		void lock() const {
			XLockDisplay(display);
		}

		void unlock() const {
			XUnlockDisplay(display);
		}


#if defined(VK_USE_PLATFORM_XLIB_KHR) || 1

		VkSurfaceKHR createSurface(VkInstance instance, X_Window window) const {
			VkXlibSurfaceCreateInfoKHR xlibSurfaceCreateInfo{
					.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
					.pNext = nullptr,
					.flags = 0,
					.dpy = display,
					.window = window.window,
			};
			VkSurfaceKHR surface;
			vkCreateXlibSurfaceKHR(instance, &xlibSurfaceCreateInfo, nullptr,
								   &surface);
			return surface;
		}

		bool queueCanPresent(VkPhysicalDevice device, uint32_t queueFamilyIndex,
							 X_Screen screen) const {
			return vkGetPhysicalDeviceXlibPresentationSupportKHR(device,
																 queueFamilyIndex,
																 display,
																 XDefaultVisualOfScreen(
																		 screen.screen)->visualid);
		}

		static void destroySurface(VkInstance instance, VkSurfaceKHR surface) {
			vkDestroySurfaceKHR(instance, surface, nullptr);
		}

#endif

		~X_Display() {
			std::cout << std::hex << display << std::endl;
			XCloseDisplay(display);
		}
	};


}

#endif //WSAL_WS_XLIB_H
