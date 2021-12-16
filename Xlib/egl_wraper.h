//
// Created by af on 10/4/21.
//

#ifndef WSAL_EGL_WRAPER_H
#define WSAL_EGL_WRAPER_H
#define CL_TARGET_OPENCL_VERSION 100

#include <X11/X.h>

extern "C" {
#include <CL/cl.h>
#include <CL/cl_egl.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
}

#include <X11/Xlib.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <array>

#ifdef GL_API_GL
#define GL_API 2
#elifdef GL_API_ES
#define GL_API 1
#endif

#ifndef GL_API
#define GL_API 1// ES
#endif

namespace EGL {

	bool handleEglError() {

		//PFNEGLOUTPUTLAYERATTRIBEXTPROC
#define ERROR_HELPER(X)                            \
    case X:                                        \
        std::cout << "Error: " << #X << std::endl; \
        return false;
		switch (eglGetError()) {
			case EGL_SUCCESS:
				return true;
			ERROR_HELPER(EGL_NOT_INITIALIZED)
			ERROR_HELPER(EGL_BAD_ACCESS)
			ERROR_HELPER(EGL_BAD_ALLOC)
			ERROR_HELPER(EGL_BAD_ATTRIBUTE)
			ERROR_HELPER(EGL_BAD_CONTEXT)
			ERROR_HELPER(EGL_BAD_CONFIG)
			ERROR_HELPER(EGL_BAD_CURRENT_SURFACE)
			ERROR_HELPER(EGL_BAD_DISPLAY)
			ERROR_HELPER(EGL_BAD_SURFACE)
			ERROR_HELPER(EGL_BAD_MATCH)
			ERROR_HELPER(EGL_BAD_PARAMETER)
			ERROR_HELPER(EGL_BAD_NATIVE_PIXMAP)
			ERROR_HELPER(EGL_BAD_NATIVE_WINDOW)
			ERROR_HELPER(EGL_CONTEXT_LOST)
			default:
				std::cout << "unknown error" << std::endl;
				return false;
		}
#undef ERROR_HELPER
	}

#define EGL(X) \
    if (EGLBoolean error = X; !error) {handleEglError();}

	void bindAPI() {
#if GL_API == 1
		EGL(eglBindAPI(EGL_OPENGL_ES_API))
#elif GL_API == 2
		EGL(eglBindAPI(EGL_OPENGL_API))
#endif
	}


#define WSAL_EGL_GET_ATTRIBUTE(X)                                          \
    EGLint getEglAttribute_##X(EGLDisplay display, EGLConfig config) {     \
        EGLint value;                                                      \
        EGL( eglGetConfigAttrib(display, config, X, &value))               \
        return value;                                                      \
    }

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

#define WSAL_EGL_PRINT_ATTRIBUTE(DISPLAY, CONFIG, X) std::cout << #X << " : " << WSAL::getEglAttribute_##X(DISPLAY, CONFIG) << std::endl


	struct Fence {
		EGLSync sync;
	};
	struct Event {
		EGLSync sync;
	};


	EGLDisplay getDisplay(Display *dpy) {
		return eglGetDisplay(dpy);
	}

	struct EglColorSize {
		EGLint r;
		EGLint g;
		EGLint b;
	};

	struct ConfigRestrictions : public std::vector<EGLint> {

		void addEglColorSizeRestriction(const EglColorSize color) {
			push_back(EGL_RED_SIZE);
			push_back(color.r);
			push_back(EGL_GREEN_SIZE);
			push_back(color.g);
			push_back(EGL_BLUE_SIZE);
			push_back(color.b);
		}

		void addEglApiRestriction() {
			push_back(EGL_CONFORMANT);
#if GL_API == 1
			push_back(EGL_OPENGL_ES2_BIT);
#elif GL_API == 2
			restrictions.push_back(EGL_OPENGL_BIT);
#endif
		}

		void addEglDepthBufferRestriction(const EGLint bits) {
			push_back(EGL_DEPTH_SIZE);
			push_back(bits);
		}

		void addEglStencilBufferRestriction(const EGLint bits) {
			push_back(EGL_STENCIL_SIZE);
			push_back(bits);
		}


		void finalizeEglRestriction() {
			push_back(EGL_NONE);
		}
	};


	struct Context {

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
				if ((major < other.major) ||
					((major == other.major) && (minor < other.minor))) {
					return std::strong_ordering::less;
				}
				return std::strong_ordering::greater;
			}

		};


		EGLContext context{};
		EglVersion geVersion{};

	};

	struct Display_ {
		EGLDisplay display;

		Display_(Display *dpy) {
			display = getDisplay(dpy);
			initialize();
		}

		~Display_() {
			terminate();
		}

		void printEglVendor() const {
			const char *vendorString = eglQueryString(display, EGL_VENDOR);
			if (vendorString == nullptr) {
				handleEglError();
			} else {
				std::cout << vendorString << std::endl;
			}
		}

		void printEglApis() const {
			const char *apiString = eglQueryString(display, EGL_CLIENT_APIS);
			if (apiString == nullptr) {
				handleEglError();
			} else {
				std::cout << apiString << std::endl;
			}
		}

		void printEglExtensions() const {
			const char *extensionString = eglQueryString(display,
														 EGL_EXTENSIONS);
			if (extensionString == nullptr) {
				handleEglError();
			} else {
				std::cout << extensionString << std::endl;
			}
		}

		[[nodiscard]] std::vector<std::string>
		getSupportedEglExtensions() const {
			const char *extensionString = eglQueryString(display,
														 EGL_EXTENSIONS);
			std::vector<std::string> supportedExtensions;
			if (extensionString == nullptr) {
				handleEglError();
			} else {
				std::istringstream extensions(extensionString);
				std::string extension;
				while (std::getline(extensions, extension, ' ')) {
					supportedExtensions.push_back(extension);
				}
			}
			return supportedExtensions;
		}


		/*
		 * requires:
		 * 	OpenGL		3.2 or GL_ARB_sync
		 * 	OpenGLes	3.0 or GL_OES_EGL_sync
		 */
		[[nodiscard]] Fence createFence() const {
			Fence sync{};
			EGLAttrib attrib_list = EGL_NONE;
			sync.sync = eglCreateSync(display, EGL_SYNC_FENCE, &attrib_list);
			return sync;
		}

		void waitFence(Fence sync) const {
			EGL(eglWaitSync(display, sync.sync, 0));
		}

		void waitClientFence(Fence sync) const {
			eglClientWaitSync(display, sync.sync, 0, EGL_FOREVER);
		}

		[[nodiscard]] bool isSignaled(Fence sync) const {
			switch (eglClientWaitSync(display, sync.sync, 0, 0)) {
				case EGL_TIMEOUT_EXPIRED:
					return false;
				case EGL_CONDITION_SATISFIED:
					return true;
				case EGL_FALSE:
					handleEglError();
			}
			return false;
		}

		[[nodiscard]] Event createEvent(cl_int clEvent) const {
			Event sync{};
			EGLAttrib attrib_list[3] = {EGL_CL_EVENT_HANDLE, clEvent, EGL_NONE};
			sync.sync = eglCreateSync(display, EGL_SYNC_CL_EVENT, attrib_list);
			return sync;
		}

		void initialize() const {
			EGLint version;
			EGL(eglInitialize(display, nullptr, &version));
			//return version; always assume 1.5
		}

		void terminate() const {
			EGL(eglTerminate(display));
		}

		[[nodiscard]] std::vector<EGLConfig> getEglConfigs() const {
			EGLint configCount;
			EGL(eglGetConfigs(display, nullptr, 0, &configCount))
			std::vector<EGLConfig> configs(configCount);
			std::cout << "EGL config count " << configCount << std::endl;
			EGL(eglGetConfigs(display, configs.data(), configCount,
							  &configCount))
			return configs;

		}

		[[nodiscard]] EGLConfig
		chooseOptimalEglConfig(const ConfigRestrictions &restrictions) const {
			std::cout << restrictions.size() << " : ";
			for (auto i: restrictions) {
				std::cout << i << " ";
			}
			std::cout << std::endl;
			EGLint numConfigs;
			EGLConfig retConfig;
			EGL(eglChooseConfig(display, restrictions.data(), &retConfig, 1,
								&numConfigs))
			std::cout << "Usable configs : " << numConfigs << std::endl;

			return retConfig;
		}


		Context createContext(EGLConfig config, Context::EglVersion maxVersion,
							  bool debug = false) const {
#if GL_API == 1
			std::array<Context::EglVersion, 5> glVersions{
					{{3, 2}, {3, 1}, {3, 0}, {2, 0}, {1, 1}}};
#else
			std::array<EglVersion, 5> glVersions{{{2,1},{2,0}}};
#endif
			for (auto selectedVersion: glVersions) {
				if (maxVersion < selectedVersion) {
					continue;
				}
				std::cout << " _ " << selectedVersion.major << "."
						  << selectedVersion.minor << std::endl;
				auto attributes = selectedVersion.toAttributeVersion();
				if (debug) {
					attributes.push_back(EGL_CONTEXT_OPENGL_DEBUG);
					attributes.push_back(EGL_TRUE);
				}
				attributes.push_back(EGL_NONE);
				EGLContext context = eglCreateContext(display, config,
													  EGL_NO_CONTEXT,
													  attributes.data());
				if (context != EGL_NO_CONTEXT) {
					return {context, selectedVersion};
				}
				std::cout << "fail" << std::endl;
				handleEglError();
			}
			return {nullptr, {0, 0}};
		}

		EGLSurface createSurface(EGLConfig config, Window window) const {
			return eglCreateWindowSurface(display, config, window, nullptr);
		}

		void draw(EGLSurface surface) const {
			eglSwapBuffers(display, surface);
		}

		void bindSurfaceToContext(EGLSurface surface, Context context) const {
			EGL(eglMakeCurrent(display, surface, surface, context.context))
		}

		void destroySurface(EGLSurface surface) const {
			EGL(eglDestroySurface(display, surface))
		}

		void destroyContext(Context context) const {
			EGL(eglDestroyContext(display, context.context))
		}
	};


	struct Image {
		EGLImage context;
	};

	void waitClient() {
		EGL(eglWaitClient());
	}

	void waitNative() {
		eglWaitNative(EGL_CORE_NATIVE_ENGINE);
	}


}// namespace EGL

#endif//WSAL_EGL_WRAPER_H
