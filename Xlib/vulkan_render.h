//
// Created by af on 16.08.21.
//

#ifndef WSAL_VULKAN_RENDER_H
#define WSAL_VULKAN_RENDER_H

#define VK_USE_PLATFORM_XLIB_KHR

#include "ws_xlib.h"
#include <algorithm>
#include <array>
#include <span>
#include <vector>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>
/*
#define A_CPU 1
#define A_GCC 1
#define A_BYTE 1
#define A_LONG 1
#define A_DUBL 1
#include "ffx_a.h"
#include "ffx_fsr1.h"
*/
#include <cmath>
#include <optional>

#include <chrono>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <map>


template<class T>
std::span<const T> SPAN_E(const T &v) { return std::span(&v, 1); }

template<class T>
std::span<const T> SPAN(const std::vector<T> &v) {
	return std::span(v.data(), v.size());
}

template<class T, size_t size>
std::span<const T> SPAN(const std::array<T, size> &v) {
	return std::span(v.data(), v.size());
}

template<class T, class P>
struct DeleteInfo {
	const T m_class;
	const P parent;

	void (*f)(P, T);

	DeleteInfo(const T c, const P &p, const auto func) : m_class(c), parent(p),
														 f(func) {
	}

	~DeleteInfo() {
		f(m_class, parent);
	}
};

namespace VK {

	void vulkanError(VkResult result) {
		if (result != VK_SUCCESS) [[unlikely]] {
			switch (result) {
#define ERROR_HELPER(X)               \
    case X:                           \
        std::cout << #X << std::endl; \
        break;
				ERROR_HELPER(VK_ERROR_OUT_OF_HOST_MEMORY)
				ERROR_HELPER(VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS)
				ERROR_HELPER(VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT)
				ERROR_HELPER(VK_ERROR_FRAGMENTATION)
				ERROR_HELPER(VK_ERROR_NOT_PERMITTED_EXT)
				ERROR_HELPER(
						VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT)
				ERROR_HELPER(VK_ERROR_INVALID_EXTERNAL_HANDLE)
				ERROR_HELPER(VK_ERROR_OUT_OF_POOL_MEMORY)
				ERROR_HELPER(VK_ERROR_INVALID_SHADER_NV)
				ERROR_HELPER(VK_ERROR_VALIDATION_FAILED_EXT)
				ERROR_HELPER(VK_ERROR_INCOMPATIBLE_DISPLAY_KHR)
				ERROR_HELPER(VK_ERROR_OUT_OF_DATE_KHR)
				ERROR_HELPER(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR)
				ERROR_HELPER(VK_ERROR_SURFACE_LOST_KHR)
				ERROR_HELPER(VK_ERROR_UNKNOWN)
				ERROR_HELPER(VK_ERROR_FRAGMENTED_POOL)
				ERROR_HELPER(VK_ERROR_FORMAT_NOT_SUPPORTED)
				ERROR_HELPER(VK_ERROR_TOO_MANY_OBJECTS)
				ERROR_HELPER(VK_ERROR_INCOMPATIBLE_DRIVER)
				ERROR_HELPER(VK_ERROR_FEATURE_NOT_PRESENT)
				ERROR_HELPER(VK_ERROR_EXTENSION_NOT_PRESENT)
				ERROR_HELPER(VK_ERROR_LAYER_NOT_PRESENT)
				ERROR_HELPER(VK_ERROR_MEMORY_MAP_FAILED)
				ERROR_HELPER(VK_ERROR_DEVICE_LOST)
				ERROR_HELPER(VK_ERROR_INITIALIZATION_FAILED)
				ERROR_HELPER(VK_ERROR_OUT_OF_DEVICE_MEMORY)
				ERROR_HELPER(VK_SUCCESS)
				ERROR_HELPER(VK_NOT_READY)
				ERROR_HELPER(VK_TIMEOUT)
				ERROR_HELPER(VK_EVENT_SET)
				ERROR_HELPER(VK_EVENT_RESET)
				ERROR_HELPER(VK_INCOMPLETE)
				ERROR_HELPER(VK_SUBOPTIMAL_KHR)
				ERROR_HELPER(VK_THREAD_IDLE_KHR)
				ERROR_HELPER(VK_THREAD_DONE_KHR)
				ERROR_HELPER(VK_OPERATION_DEFERRED_KHR)
				ERROR_HELPER(VK_OPERATION_NOT_DEFERRED_KHR)
				ERROR_HELPER(VK_PIPELINE_COMPILE_REQUIRED_EXT)
				default:
					return;
			}
		}
	}

#define VK_API(X) vulkanError(X);

	template<class T>
	struct EnumType {
		T val;

		EnumType(T value) : val(value) {}

		EnumType operator|(EnumType other) { return (T) (val | other.val); }

		operator T() { return val; }
	};

	template<class T>
	constexpr EnumType<T> conditionalFlag(bool con, T val) {
		return (T) (con ? val : (T) 0u);
	}

	VkApplicationInfo createAppInfo() {
		VkApplicationInfo applicationInfo{};
		applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		applicationInfo.pNext = nullptr;
		applicationInfo.pEngineName = "VK_REN";
		applicationInfo.engineVersion = VK_MAKE_VERSION(0, 0, 0);
		applicationInfo.pApplicationName = "Hello World";
		applicationInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 0);
		applicationInfo.apiVersion = VK_API_VERSION_1_2;

		return applicationInfo;
	}

	void printVersion(uint32_t version) {
		std::cout << VK_VERSION_MAJOR(version) << "."
				  << VK_VERSION_MINOR(version)
				  << "." << VK_VERSION_PATCH(version) << std::endl;
	}

	void printInstanceExtensions() {
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount,
											   nullptr);
		std::vector<VkExtensionProperties> extensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount,
											   extensions.data());
		for (auto extension: extensions) {
			std::cout << extension.extensionName << " : ";
			printVersion(extension.specVersion);
		}
	}

	void printInstanceLayers() {
		uint32_t layerCount = 0;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		std::vector<VkLayerProperties> layers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, layers.data());
		std::cout << std::endl;
		for (auto layer: layers) {
			std::cout << layer.layerName << " : ";
			printVersion(layer.specVersion);
			std::cout << '\t' << layer.description << std::endl;
		}
	}

#define STR(X) #X

	VkInstance createInstance() {
		VkInstanceCreateInfo instanceCreateInfo{};
		instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceCreateInfo.pNext = nullptr;
		instanceCreateInfo.flags = 0;
		const auto appInfo = createAppInfo();
		const auto extensionNames = std::array<const char *, 2>(
				{VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
				 VK_KHR_SURFACE_EXTENSION_NAME});

		const auto layerNames =
				std::array<const char *, 1>({"VK_LAYER_KHRONOS_validation"});
		instanceCreateInfo.pApplicationInfo = &appInfo;
		instanceCreateInfo.ppEnabledExtensionNames = extensionNames.data();
		instanceCreateInfo.enabledExtensionCount = extensionNames.size();
		instanceCreateInfo.ppEnabledLayerNames = layerNames.data();
		instanceCreateInfo.enabledLayerCount = layerNames.size();
		VkInstance instance;
		std::cout << (instanceCreateInfo.ppEnabledLayerNames == nullptr)
				  << std::endl;
		VK_API(vkCreateInstance(&instanceCreateInfo, nullptr, &instance));
		return instance;
	}

	void destroyInstance(VkInstance instance) {
		vkDestroyInstance(instance, nullptr);
	}

	std::vector<VkPhysicalDevice> getDevices(VkInstance instance) {
		uint32_t deviceCount = 0;
		VK_API(vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr))
		std::vector<VkPhysicalDevice> devices(deviceCount);
		VK_API(vkEnumeratePhysicalDevices(instance, &deviceCount,
										  devices.data()))
		return devices;
	}

	void printDriverVersion(const VkPhysicalDeviceProperties &properties) {
		if (properties.vendorID == 0x10DE) {
			std::cout << (properties.driverVersion >> 22) << "."
					  << ((properties.driverVersion & ((1 << 22) - 1)) >> 14)
					  << "."
					  << ((properties.driverVersion & ((1 << 14) - 1)) >> 6)
					  << std::endl;
		} else {
			printVersion(properties.driverVersion);
		}
	}

	void printDeviceName(const VkPhysicalDeviceProperties &properties) {
		std::cout << properties.deviceName << std::endl;
	}

	struct VulkanBase {
		VkStructureType sType;
		void *pNext;
	};

#define CONTAINS(X, VAL) std::find((X).begin(), (X).end(), VAL) != (X).end()

	VkPhysicalDeviceProperties
	getPhysicalDeviceProperties(VkPhysicalDevice device) {
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(device, &properties);
		return properties;
	}

	std::vector<VkQueueFamilyProperties>
	getPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice device) {
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
												 nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
												 queueFamilies.data());
		return queueFamilies;
	}

#define VkPhysicalDeviceDriverProperties(X) \
    VkPhysicalDeviceDriverProperties X;     \
    (X).sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES

	void printDeviceInfo(VkInstance instance) {
		for (auto device: getDevices(instance)) {
			VkPhysicalDeviceProperties properties;
			properties = getPhysicalDeviceProperties(device);

			std::cout << "\t";
			printDeviceName(properties);
			std::cout << "\t";
			printDriverVersion(properties);
			std::cout << "\t";
			std::cout << std::endl;
		}
	}

#define BOOL(X) ((X) ? "true" : "false")

	void printQueueInfo(VkQueueFamilyProperties queueFamilyProperties) {
		std::cout << "\t " << queueFamilyProperties.queueCount << std::endl;
		std::cout << "\tGRAPHICS : "
				  << BOOL(queueFamilyProperties.queueFlags &
						  VK_QUEUE_GRAPHICS_BIT)
				  << std::endl;
		std::cout << "\tCOMPUTE : "
				  << BOOL(queueFamilyProperties.queueFlags &
						  VK_QUEUE_COMPUTE_BIT)
				  << std::endl;
		std::cout << "\tTRANSFER : "
				  << BOOL(queueFamilyProperties.queueFlags &
						  VK_QUEUE_TRANSFER_BIT)
				  << std::endl;
		std::cout << "\tSPARSE : "
				  << BOOL(queueFamilyProperties.queueFlags &
						  VK_QUEUE_SPARSE_BINDING_BIT)
				  << std::endl;
	}

	void printQueues(VkPhysicalDevice device) {
		auto queues = getPhysicalDeviceQueueFamilyProperties(device);
		std::cout << "queue count : " << queues.size() << std::endl;
		for (auto queue: queues) {
			std::cout << std::endl;
			printQueueInfo(queue);
		}
	}

	/*
        struct Allocator {
                VmaAllocator allocator;
                std::unordered_map<VkBuffer, VmaAllocation> bufferAllocations;

                Allocator(VkInstance instance, VkPhysicalDevice physicalDevice,
   VkDevice device) { VmaAllocatorCreateInfo allocatorInfo{ .physicalDevice =
   physicalDevice, .device = device, .instance = instance, .vulkanApiVersion =
   VK_API_VERSION_1_0,
                        };

                        vmaCreateAllocator(&allocatorInfo, &allocator);
                }

                ~Allocator() {
                        vmaDestroyAllocator(allocator);
                }

                VkBuffer createBuffer(VkBufferCreateInfo bufferCreateInfo) const
   { VmaAllocationCreateInfo allocationCreateInfo{ .usage =
   VMA_MEMORY_USAGE_GPU_ONLY,
                        };
                        VkBuffer buffer;
                        VmaAllocation allocation;
                        vmaCreateBuffer(allocator, &bufferCreateInfo,
   &allocationCreateInfo, &buffer, &allocation, nullptr); return buffer;
                }

                void free(VkBuffer buffer) {
                        vmaDestroyBuffer(allocator, buffer,
   bufferAllocations[buffer]); bufferAllocations.erase(buffer);
                }
        };
*/
	void printMemoryInfo(VkPhysicalDevice device) {
		std::cout << std::endl;
		VkPhysicalDeviceMemoryProperties memoryProperties;
		vkGetPhysicalDeviceMemoryProperties(device, &memoryProperties);
		for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {

			VkMemoryType memoryType = memoryProperties.memoryTypes[i];
			if (memoryType.propertyFlags == 0) {
				continue;
			}
			std::cout << "\t" << i << " : " << memoryType.heapIndex
					  << std::endl;
			std::cout << "\t HOST visible "
					  << ((memoryType.propertyFlags &
						   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
						  ? "TRUE"
						  : "FALSE")
					  << std::endl;
			std::cout << "\t HOST coherent "
					  << ((memoryType.propertyFlags &
						   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
						  ? "TRUE"
						  : "FALSE")
					  << std::endl;
			std::cout << "\t HOST cached "
					  << ((memoryType.propertyFlags &
						   VK_MEMORY_PROPERTY_HOST_CACHED_BIT)
						  ? "TRUE"
						  : "FALSE")
					  << std::endl;
			std::cout << "\t Lazy "
					  << ((memoryType.propertyFlags &
						   VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT)
						  ? "TRUE"
						  : "FALSE")
					  << std::endl;
			std::cout << "\t Device local "
					  << ((memoryType.propertyFlags &
						   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
						  ? "TRUE"
						  : "FALSE")
					  << std::endl;
			std::cout << std::endl;
		}
		for (uint32_t i = 0; i < memoryProperties.memoryHeapCount; i++) {

			VkMemoryHeap memoryHeap = memoryProperties.memoryHeaps[i];
			std::cout << "\t" << i << std::endl;
			std::cout << "\t Size "
					  << (memoryHeap.size > 1000000000 ? memoryHeap.size /
														 1000000000
													   : memoryHeap.size /
														 1000000)
					  << (memoryHeap.size > 1000000000 ? "GB" : "MB")
					  << std::endl;
			std::cout << std::endl;

			void (*f)() = []() { return; };
			f();
		}
	}

	struct MemoryReference {
		const uint32_t index = -1;
		const VkDeviceSize size = 0;
		const VkMemoryPropertyFlags properties = 0;
	};

	struct Pointer {
		uint32_t memoryReference;
		VkDeviceMemory allocation;
		VkDeviceSize index;

		void operator+=(VkDeviceSize a) { index += a; }

		void operator-=(VkDeviceSize a) { index -= a; }
	};

	/*
 * can request device local or host visible
 */
	MemoryReference findMemory(VkPhysicalDevice device,
							   VkMemoryPropertyFlags requirements) {
		VkPhysicalDeviceMemoryProperties memoryProperties;
		vkGetPhysicalDeviceMemoryProperties(device, &memoryProperties);
		// minimum index is optimal
		for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
			const VkMemoryType memoryType = memoryProperties.memoryTypes[i];

			const bool deviceLocal =
					memoryType.propertyFlags &
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			const bool deviceLocalRequired =
					requirements & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			if (deviceLocalRequired && !deviceLocal) {
				continue;
			}
			const bool hostVisible =
					memoryType.propertyFlags &
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
			const bool hostVisibleRequired =
					requirements & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
			if (hostVisibleRequired && !hostVisible) {
				continue;
			}
			const bool hostCoherent =
					memoryType.propertyFlags &
					VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
			const bool hostCoherentRequired =
					requirements & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
			if (hostCoherentRequired && !hostCoherent) {
				continue;
			}
			std::cout << "found " << (memoryType.heapIndex) << std::endl;
			return {
					.index = i,
					.size = memoryProperties.memoryHeaps[memoryType.heapIndex].size,
					.properties = memoryType.propertyFlags,
			};
		}
		throw 1;
		return {};
	}

	VkDeviceMemory allocateMemory(VkDevice device, uint64_t size,
								  uint32_t memoryType) {
		VkMemoryAllocateInfo memoryAllocateInfo{
				.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
				.pNext = nullptr,
				.allocationSize = size,
				.memoryTypeIndex = memoryType,
		};
		VkDeviceMemory memory;
		vkAllocateMemory(device, &memoryAllocateInfo, nullptr, &memory);
		return memory;
	}

	void freeMemory(VkDevice device, VkDeviceMemory memory) {
		vkFreeMemory(device, memory, nullptr);
	}

	void bind(VkDevice device, VkBuffer buffer, Pointer ptr) {
		VK_API(vkBindBufferMemory(device, buffer, ptr.allocation, ptr.index))
	}

	void bind(VkDevice device, VkImage buffer, Pointer ptr) {
		VK_API(vkBindImageMemory(device, buffer, ptr.allocation, ptr.index))
	}

	void *
	mapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset = 0,
			  VkDeviceSize size = VK_WHOLE_SIZE) {
		void *mem;
		VK_API(vkMapMemory(device, memory, offset, size, 0, &mem));
		return mem;
	}

	void unmapMemory(VkDevice device, VkDeviceMemory memory) {
		vkUnmapMemory(device, memory);
	}

	VkMappedMemoryRange
	generateMappedMemoryRange(VkDeviceMemory memory, VkDeviceSize offset = 0,
							  VkDeviceSize size = VK_WHOLE_SIZE) {
		return {
				.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
				.pNext = nullptr,
				.memory = memory,
				.offset = offset,
				.size = size,
		};
	}

	VkMemoryRequirements
	getMemoryRequirement(VkDevice device, VkBuffer buffer) {
		VkMemoryRequirements requirements;
		vkGetBufferMemoryRequirements(device, buffer, &requirements);
		return requirements;
	}

	VkMemoryRequirements getMemoryRequirement(VkDevice device, VkImage image) {
		VkMemoryRequirements requirements;
		vkGetImageMemoryRequirements(device, image, &requirements);
		return requirements;
	}

	void flushMappedMemoryRanges(
			VkDevice device,
			const std::span<const VkMappedMemoryRange> memoryRanges) {
		vkFlushMappedMemoryRanges(device, memoryRanges.size(),
								  memoryRanges.data());
	}

	void invalidateMappedMemoryRanges(
			VkDevice device,
			const std::span<const VkMappedMemoryRange> memoryRanges) {
		vkInvalidateMappedMemoryRanges(device, memoryRanges.size(),
									   memoryRanges.data());
	}

	struct PhysicalDeviceType {
		VkPhysicalDeviceType deviceType = VK_PHYSICAL_DEVICE_TYPE_OTHER;

		PhysicalDeviceType() = default;

		PhysicalDeviceType(VkPhysicalDeviceType type) : deviceType(type) {}

		[[nodiscard]] int getHelperId() const {
			switch (deviceType) {
				case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
					return 0;
				case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
					return 1;
				case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
					return 2;
				case VK_PHYSICAL_DEVICE_TYPE_CPU:
					return 3;
				case VK_PHYSICAL_DEVICE_TYPE_OTHER:
					return 4;
				default:// can't happen
					return 5;
			}
		}

		[[nodiscard]] int getWeakestAcceleratorId() const {
			switch (deviceType) {
				case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
					return 1;
				case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
					return 0;
				case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
					return 2;
				case VK_PHYSICAL_DEVICE_TYPE_CPU:
					return 3;
				case VK_PHYSICAL_DEVICE_TYPE_OTHER:
					return 4;
				default:// can't happen
					return 5;
			}
		}

		std::strong_ordering operator<=>(PhysicalDeviceType other) const {
			if (getHelperId() < other.getHelperId()) {
				return std::strong_ordering::greater;
			}
			if (getHelperId() > other.getHelperId()) {
				return std::strong_ordering::less;
			}
			return std::strong_ordering::equal;
		}
	};

	VkPhysicalDevice getBestDevice(VkInstance instance) {
		auto devices = getDevices(instance);
		VkPhysicalDevice max = devices[0];
		for (auto device: devices) {
			PhysicalDeviceType deviceType =
					getPhysicalDeviceProperties(device).deviceType;
			if (deviceType > getPhysicalDeviceProperties(max).deviceType) {
				max = device;
			}
		}
		return max;
	}

	struct QueueFamily {
		const VkQueueFamilyProperties properties{};
		const uint32_t queueCount = 0;
		const uint32_t queueIndex = 0;
		VkQueue *queues = nullptr;

		QueueFamily(const VkQueueFamilyProperties &properties,
					uint32_t queueIndex,
					uint32_t maxUsedQueues = 1)
				: properties(properties),
				  queueCount(std::min(maxUsedQueues, properties.queueCount)),
				  queueIndex(queueIndex) {}

		[[nodiscard]] VkDeviceQueueCreateInfo createQueues() const {
			auto *priorities = new float[queueCount];
			for (uint32_t i = 0; i < queueCount; i++) {
				priorities[i] = 1.0f;
			}
			VkDeviceQueueCreateInfo queueCreateInfo{
					.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
					.pNext = nullptr,
					.flags = 0,
					.queueFamilyIndex = queueIndex,
					.queueCount = queueCount,
					.pQueuePriorities = priorities,
			};
			return queueCreateInfo;
		};

		static void free(VkDeviceQueueCreateInfo info) {
			delete[] info.pQueuePriorities;
		}

		void createQueues(VkDevice device) {
			queues = new VkQueue[queueCount];
			for (uint32_t i = 0; i < queueCount; i++) {
				vkGetDeviceQueue(device, queueIndex, i, &queues[i]);
			}
		}

		VkQueue operator[](int index) const { return queues[index]; }

		~QueueFamily() { delete[] queues; }
	};

	std::vector<QueueFamily> getQueueFamilies(VkPhysicalDevice device) {
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
												 nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilyProperties(
				queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
												 queueFamilyProperties.data());
		std::vector<QueueFamily> queueFamilies;
		queueFamilies.reserve(queueFamilyCount);
		for (uint32_t i = 0; i < queueFamilyCount; i++) {
			queueFamilies.emplace_back(queueFamilyProperties[i], i);
		}
		return queueFamilies;
	}

	VkDevice createDevice(VkPhysicalDevice physicalDevice,
						  const std::span<const QueueFamily> &queueFamilies) {

		std::vector<VkDeviceQueueCreateInfo> deviceQueueCreateInfo;
		deviceQueueCreateInfo.reserve(queueFamilies.size());
		for (const auto &queueFamily: queueFamilies) {
			deviceQueueCreateInfo.push_back(queueFamily.createQueues());
		}

		const auto extensionNames =
				std::array<const char *, 1>({VK_KHR_SWAPCHAIN_EXTENSION_NAME});
		VkDeviceCreateInfo deviceCreateInfo{
				.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.queueCreateInfoCount = (uint32_t) deviceQueueCreateInfo.size(),
				.pQueueCreateInfos = deviceQueueCreateInfo.data(),
				.enabledLayerCount = 0,
				.ppEnabledLayerNames = nullptr,
				.enabledExtensionCount = extensionNames.size(),
				.ppEnabledExtensionNames = extensionNames.data(),
				.pEnabledFeatures = {},
		};
		std::cout << " : .: " << deviceCreateInfo.queueCreateInfoCount
				  << std::endl;

		VkDevice device;
		VK_API(vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr,
							  &device));
		for (const auto &_: deviceQueueCreateInfo) {
			QueueFamily::free(_);
		}
		return device;
	}

	void destroyDevice(VkDevice device) {
		if (device == VK_NULL_HANDLE) {
			return;
		}
		vkDeviceWaitIdle(device);
		vkDestroyDevice(device, nullptr);
	}


	struct PushConstantVariableInfo {
		VkShaderStageFlagBits shaderStages;
		uint32_t size;
	};

	struct PushConstant {
		VkShaderStageFlagBits shaderStages = {};
		uint32_t size = 0;
		uint32_t buffer[128 / sizeof(uint32_t)];

		uint32_t addVariable(PushConstantVariableInfo variable) {
			uint32_t variableIndex = size / 4;
			size += (variable.size + 3) / 4 * 4;// align to int
			if (size > 128) {
				std::cout << "error" << std::endl;
				// make template to catch error at compile time
				return -1;
			}
			shaderStages = VkShaderStageFlagBits(
					shaderStages | variable.shaderStages);
			return variableIndex;
		}

		VkPushConstantRange crateRange() {
			const VkPushConstantRange pushConstantRange{
					.stageFlags = shaderStages,
					.offset = 0,
					.size = size,
			};
			return pushConstantRange;
		}

		void setValue(uint32_t index, std::span<const uint32_t> data) {
			// bad memcpy
			for (const auto element: data) {
				buffer[index] = element;
				index++;
			}
		}

		void clear() {
			for (uint32_t &v: buffer) {
				v = 0;
			}
		}
	};

	struct Vertex3DPosData {
		alignas(16) glm::vec3 pos;
	};

	template<class T>
	struct PushConstantVariable {
		uint32_t index = 0;

		void
		submit(PushConstant &pushConstant, VkShaderStageFlagBits shaderStages) {
			index = pushConstant.addVariable({
													 .shaderStages = shaderStages,
													 .size = sizeof(T),
											 });
		}

		void write(PushConstant &pushConstant, T data) {
			constexpr uint32_t size = (sizeof(data) + 3) / 4;// round up
			std::array<uint32_t, size> memory;
			memcpy(memory.data(), &data, sizeof(data));
			pushConstant.setValue(index, memory);
		}
	};

	template<VkQueueFlagBits queueFamilyProperties>
	struct CommandPool {
		VkCommandPool commandPool = VK_NULL_HANDLE;
		VkDevice device = VK_NULL_HANDLE;

		CommandPool() = default;

		CommandPool(VkDevice device, const QueueFamily &queueFamily,
					bool buffersAreShortLived = false, bool canReset = false)
				: device(device) {
			if ((queueFamily.properties.queueFlags & queueFamilyProperties) !=
				queueFamilyProperties) {
				throw std::runtime_error("Queue family has wrong properties");
			}
			VkCommandPoolCreateInfo commandPoolCreateInfo{
					.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
					.pNext = nullptr,
					.flags = conditionalFlag(buffersAreShortLived,
											 VK_COMMAND_POOL_CREATE_TRANSIENT_BIT) |
							 conditionalFlag(
									 canReset,
									 VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT),
					.queueFamilyIndex = queueFamily.queueIndex

			};

			VK_API(vkCreateCommandPool(device, &commandPoolCreateInfo, nullptr,
									   &commandPool))
		}

		CommandPool(const CommandPool &) = delete;

		CommandPool(const CommandPool &&) = delete;

		~CommandPool() {
			if (commandPool == VK_NULL_HANDLE) {
				return;
			}
			vkDestroyCommandPool(device, commandPool, nullptr);
		}

		void reset(bool reallocate = false) {
			vkResetCommandPool(
					device, commandPool,
					conditionalFlag(reallocate,
									VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT));
		}
	};

	namespace CMD {
		void
		beginRenderPass(VkCommandBuffer commandBuffer, VkRenderPass renderPass,
						VkFramebuffer framebuffer, VkExtent2D framebufferSize) {
			VkClearValue clearValue = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
			VkRenderPassBeginInfo renderPassBeginInfo{
					.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
					.pNext = nullptr,
					.renderPass = renderPass,
					.framebuffer = framebuffer,
					.renderArea = {{0, 0}, framebufferSize},
					.clearValueCount = 1,
					.pClearValues = &clearValue,
			};
			vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo,
								 VK_SUBPASS_CONTENTS_INLINE);
		}

		template<const VkPipelineBindPoint pipelineBindPoint>
		void bindDescriptorSets(VkCommandBuffer commandBuffer,
								VkPipelineLayout layout,
								std::span<const VkDescriptorSet> descriptorSets) {

			vkCmdBindDescriptorSets(commandBuffer, pipelineBindPoint, layout, 0,
									(uint32_t) descriptorSets.size(),
									descriptorSets.data(), 0, nullptr);
		}

		void endRenderPass(VkCommandBuffer commandBuffer) {
			vkCmdEndRenderPass(commandBuffer);
		}

		template<VkPipelineBindPoint type>
		void bindPipeline(VkCommandBuffer commandBuffer, VkPipeline pipeline) {
			vkCmdBindPipeline(commandBuffer, type, pipeline);
		}

		void draw(VkCommandBuffer commandBuffer, uint32_t vertices) {
			vkCmdDraw(commandBuffer, vertices, 1, 0, 0);
		}

		void drawIndexed(VkCommandBuffer commandBuffer, uint32_t indices) {
			vkCmdDrawIndexed(commandBuffer, indices, 1, 0, 0, 1);
		}

		void setViewport(VkCommandBuffer commandBuffer, VkExtent2D dimensions) {
			VkViewport viewport{
					.x = 0.0,
					.y = 0.0,
					.width = (float) dimensions.width,
					.height = (float) dimensions.height,
					.minDepth = 0.0,
					.maxDepth = 1.0,
			};
			vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		}

		void setScissor(VkCommandBuffer commandBuffer, VkExtent2D dimensions) {
			VkRect2D scissor{{0,                0},
							 {dimensions.width, dimensions.height}};
			vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
		}

		void bindVertexBuffers(VkCommandBuffer commandBuffer, VkBuffer buffer) {
			uint64_t offset = 0;
			vkCmdBindVertexBuffers(commandBuffer, 0, 1, &buffer, &offset);
		}

		void copyBuffer(VkCommandBuffer commandBuffer, VkBuffer source,
						VkBuffer destination, VkDeviceSize size) {
			VkBufferCopy bufferCopy{
					.srcOffset = 0,
					.dstOffset = 0,
					.size = size,
			};
			vkCmdCopyBuffer(commandBuffer, source, destination, 1, &bufferCopy);
		}

		void bindIndexBuffers(VkCommandBuffer commandBuffer, VkBuffer buffer) {
			VkDeviceSize offset = 0;
			vkCmdBindIndexBuffer(commandBuffer, buffer, offset,
								 VK_INDEX_TYPE_UINT32);
		}

		void
		pushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout,
					  VkShaderStageFlagBits shaderStages, uint32_t size,
					  void *pValues) {
			vkCmdPushConstants(commandBuffer, layout, shaderStages, 0, size,
							   pValues);
		}

	}// namespace CMD

	template<VkQueueFlagBits queueFamilyProperties>
	struct CommandBuffer {
		VkCommandBuffer commandBuffer = VK_NULL_HANDLE;

		CommandBuffer() = default;

		const CommandBuffer &copyBuffer(VkBuffer source, VkBuffer destination,
										VkDeviceSize size) const {
			static_assert(queueFamilyProperties & VK_QUEUE_GRAPHICS_BIT ||
						  queueFamilyProperties & VK_QUEUE_COMPUTE_BIT ||
						  queueFamilyProperties & VK_QUEUE_TRANSFER_BIT,
						  "Queue family does not support this operation");
			CMD::copyBuffer(commandBuffer, source, destination, size);
			return *this;
		}

		const CommandBuffer &bindVertexBuffer(VkBuffer vertexBuffer) const {
			static_assert(queueFamilyProperties & VK_QUEUE_GRAPHICS_BIT,
						  "Queue family does not support this operation");
			CMD::bindVertexBuffers(commandBuffer, vertexBuffer);
			return *this;
		}

		const CommandBuffer &bindIndexBuffer(VkBuffer vertexBuffer) const {
			static_assert(queueFamilyProperties & VK_QUEUE_GRAPHICS_BIT,
						  "Queue family does not support this operation");
			CMD::bindIndexBuffers(commandBuffer, vertexBuffer);
			return *this;
		}


		template<const VkPipelineBindPoint pipelineBindPoint>
		const CommandBuffer &bindDescriptorSets(VkPipelineLayout layout,
												std::span<const VkDescriptorSet> descriptorSets) const {
			static_assert(queueFamilyProperties & VK_QUEUE_GRAPHICS_BIT,
						  "Queue family does not support this operation");
			CMD::bindDescriptorSets<pipelineBindPoint>(commandBuffer, layout,
													   descriptorSets);
			return *this;
		}

		const CommandBuffer &setScissor(VkExtent2D dimensions) const {
			static_assert(queueFamilyProperties & VK_QUEUE_GRAPHICS_BIT,
						  "Queue family does not support this operation");
			CMD::setScissor(commandBuffer, dimensions);
			return *this;
		}

		const CommandBuffer &setViewport(VkExtent2D dimensions) const {
			static_assert(queueFamilyProperties & VK_QUEUE_GRAPHICS_BIT,
						  "Queue family does not support this operation");
			CMD::setViewport(commandBuffer, dimensions);
			return *this;
		}

		const CommandBuffer &draw(uint32_t vertices) const {
			static_assert(queueFamilyProperties & VK_QUEUE_GRAPHICS_BIT,
						  "Queue family does not support this operation");
			CMD::draw(commandBuffer, vertices);
			return *this;
		}

		const CommandBuffer &drawIndexed(uint32_t indices) const {
			static_assert(queueFamilyProperties & VK_QUEUE_GRAPHICS_BIT,
						  "Queue family does not support this operation");
			CMD::drawIndexed(commandBuffer, indices);
			return *this;
		}

		template<VkPipelineBindPoint type>
		const CommandBuffer &bindPipeline(VkPipeline pipeline) const {
			if constexpr (type == VK_PIPELINE_BIND_POINT_GRAPHICS) {
				static_assert(queueFamilyProperties & VK_QUEUE_GRAPHICS_BIT,
							  "Queue family does not support this operation");
			} else if constexpr (type == VK_PIPELINE_BIND_POINT_COMPUTE) {
				static_assert(queueFamilyProperties & VK_QUEUE_COMPUTE_BIT,
							  "Queue family does not support this operation");
			}
			CMD::bindPipeline<type>(commandBuffer, pipeline);
			return *this;
		}

		const CommandBuffer &bindGraphicsPipeline(VkPipeline pipeline) const {
			return bindPipeline<VK_PIPELINE_BIND_POINT_GRAPHICS>(pipeline);
		}

		const CommandBuffer &beginRenderPass(VkRenderPass renderPass,
											 VkFramebuffer framebuffer,
											 VkExtent2D framebufferSize) const {
			static_assert(queueFamilyProperties & VK_QUEUE_GRAPHICS_BIT,
						  "Queue family does not support this operation");
			CMD::beginRenderPass(commandBuffer, renderPass, framebuffer,
								 framebufferSize);
			return *this;
		}

		const CommandBuffer &endRenderPass() const {

			static_assert(queueFamilyProperties & VK_QUEUE_GRAPHICS_BIT,
						  "Queue family does not support this operation");
			CMD::endRenderPass(commandBuffer);
			return *this;
		}

		const CommandBuffer &pushConstants(VkPipelineLayout layout,
										   PushConstant pushConstant) const {

			static_assert(queueFamilyProperties & VK_QUEUE_GRAPHICS_BIT,
						  "Queue family does not support this operation");
			CMD::pushConstants(commandBuffer, layout, pushConstant.shaderStages,
							   pushConstant.size, pushConstant.buffer);
			return *this;
		}
	};

	typedef CommandBuffer<VK_QUEUE_GRAPHICS_BIT> GraphicsCommandBuffer;
	typedef CommandBuffer<VK_QUEUE_COMPUTE_BIT> ComputeCommandBuffer;
	typedef CommandBuffer<VK_QUEUE_TRANSFER_BIT> TransferCommandBuffer;

	template<VkQueueFlagBits queueFamilyProperties>
	std::vector<CommandBuffer<queueFamilyProperties>>
	allocateCommandBuffers(
			const CommandPool<queueFamilyProperties> *commandPool,
			uint32_t commandBufferCount, bool isPrimary = true) {
		std::vector<CommandBuffer<queueFamilyProperties>>
				commandBuffers(
				commandBufferCount);

		const VkCommandBufferAllocateInfo commandBufferAllocateInfo{
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
				.pNext = nullptr,
				.commandPool = commandPool->commandPool,
				.level = (isPrimary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY
									: VK_COMMAND_BUFFER_LEVEL_SECONDARY),
				.commandBufferCount = commandBufferCount,
		};

		VK_API(vkAllocateCommandBuffers(commandPool->device,
										&commandBufferAllocateInfo,
										(VkCommandBuffer *) commandBuffers.data()));

		return commandBuffers;
	}

	void resetCommandBuffer(VkCommandBuffer commandBuffer,
							bool reallocate = false) {
		VK_API(vkResetCommandBuffer(
				commandBuffer,
				conditionalFlag(reallocate,
								VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT)))
	}

	template<VkQueueFlagBits queueFamilyProperties>
	void freeCommandBuffers(
			const CommandPool<queueFamilyProperties> *commandPool,
			const std::span<const CommandBuffer<queueFamilyProperties>> &commandBuffers) {
		if (commandBuffers.empty()) {
			return;
		}
		vkFreeCommandBuffers(commandPool->device, commandPool->commandPool,
							 commandBuffers.size(),
							 (VkCommandBuffer *) commandBuffers.data());
	}

	template<VkQueueFlagBits queueFamilyProperties,
			VkCommandBufferUsageFlagBits usage = (VkCommandBufferUsageFlagBits) 0>
	void recordCommandBuffer(
			const CommandBuffer<queueFamilyProperties> &commandBuffer,
			auto lambda) {
		const VkCommandBufferBeginInfo commandBufferBeginInfo{
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
				.pNext = nullptr,
				.flags = usage,
				.pInheritanceInfo = nullptr};
		VK_API(vkBeginCommandBuffer(commandBuffer.commandBuffer,
									&commandBufferBeginInfo));
		lambda();
		VK_API(vkEndCommandBuffer(commandBuffer.commandBuffer))
	}

	// TODO make vector<pair> with AoS internal layout
	void queueSubmit(VkQueue queue, const std::vector<VkSemaphore> &wait,
					 const std::vector<VkPipelineStageFlags> &waitStage,
					 const std::vector<VkSemaphore> &signal,
					 const std::vector<VkCommandBuffer> &commandBuffers,
					 VkFence onOperationFinished = VK_NULL_HANDLE) {
		VkSubmitInfo submitInfo{
				.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
				.pNext = nullptr,
				.waitSemaphoreCount = (uint32_t) wait.size(),
				.pWaitSemaphores = wait.data(),
				.pWaitDstStageMask = waitStage.data(),
				.commandBufferCount = (uint32_t) commandBuffers.size(),
				.pCommandBuffers = commandBuffers.data(),
				.signalSemaphoreCount = (uint32_t) signal.size(),
				.pSignalSemaphores = signal.data(),
		};
		vkQueueSubmit(queue, 1, &submitInfo, onOperationFinished);
	}

	VkFence createFence(VkDevice device) {
		VkFenceCreateInfo fenceCreateInfo{
				.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
		};
		VkFence fence;
		vkCreateFence(device, &fenceCreateInfo, nullptr, &fence);
		return fence;
	}

	void destroyFence(VkDevice device, VkFence fence) {
		vkDestroyFence(device, fence, nullptr);
	}

	void wait(VkDevice device, VkFence fence, uint64_t timeout = -1) {
		vkWaitForFences(device, 1, &fence, VK_TRUE, timeout);
	}

	VkSemaphore createSemaphore(VkDevice device) {
		VkSemaphoreCreateInfo semaphoreCreateInfo{
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
		};
		VkSemaphore semaphore;
		VK_API(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr,
								 &semaphore));
		return semaphore;
	}

	void destroySemaphore(VkDevice device, VkSemaphore semaphore) {
		if (semaphore != VK_NULL_HANDLE) {
			vkDestroySemaphore(device, semaphore, nullptr);
		}
	}

	VkEvent createEvent(VkDevice device) {
		VkEventCreateInfo eventCreateInfo{
				.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
		};
		VkEvent event;
		VK_API(vkCreateEvent(device, &eventCreateInfo, nullptr, &event));
		return event;
	}

	bool isSignaled(VkDevice device, VkEvent event) {
		VkResult result = vkGetEventStatus(device, event);
		if (result == VK_EVENT_SET) {
			return true;
		}
		if (result == VK_EVENT_RESET) {
			return false;
		}
		vulkanError(result);
		exit(-1);
	}

	void signal(VkDevice device, VkEvent event) {
		VK_API(vkSetEvent(device, event));
	}

	void signal(VkCommandBuffer commandBuffer, VkEvent event,
				VkPipelineStageFlags pipelineStage) {
		vkCmdSetEvent(commandBuffer, event, pipelineStage);
	}

	void reset(VkCommandBuffer commandBuffer, VkEvent event,
			   VkPipelineStageFlags pipelineStage) {
		vkCmdResetEvent(commandBuffer, event, pipelineStage);
	}

	void destroyEvent(VkDevice device, VkEvent event) {
		if (event != VK_NULL_HANDLE) {
			vkDestroyEvent(device, event, nullptr);
		}
	}

	VkAttachmentDescription
	generateAttachmentDescriptionToPresent(VkFormat format) {
		VkAttachmentDescription attachmentDescription{
				.flags = 0,
				.format = format,
				.samples = VK_SAMPLE_COUNT_1_BIT,
				.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
				.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
				.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.initialLayout =
				VK_IMAGE_LAYOUT_UNDEFINED,// VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		};
		return attachmentDescription;
	}

	/*
 * Lifetime of attachments MUST include creation of the VkRenderPass containing
 * the Subpasses!
 */
	template<VkPipelineBindPoint pipelineType>
	VkSubpassDescription
	generateSubpass(const std::vector<VkAttachmentReference> &inputs,
					const std::vector<VkAttachmentReference> &color,
					const std::vector<uint32_t> &preserveAttachments = {}) {
		VkSubpassDescription subpassDescription{
				.flags = 0,
				.pipelineBindPoint = pipelineType,
				.inputAttachmentCount = (uint32_t) inputs.size(),
				.pInputAttachments = inputs.data(),
				.colorAttachmentCount = (uint32_t) color.size(),
				.pColorAttachments = color.data(),
				.pResolveAttachments = nullptr,
				.pDepthStencilAttachment = nullptr,
				.preserveAttachmentCount = (uint32_t) preserveAttachments.size(),
				.pPreserveAttachments = preserveAttachments.data(),
		};
		return subpassDescription;
	}

	VkRenderPass createRenderPass(
			VkDevice device,
			const std::vector<VkAttachmentDescription> &attachments,
			const std::vector<VkSubpassDescription> &subpasses,
			const std::vector<VkSubpassDependency> &subpassDependencies = {}) {
		VkRenderPassCreateInfo renderPassCreateInfo{
				.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.attachmentCount = (uint32_t) attachments.size(),
				.pAttachments = attachments.data(),
				.subpassCount = (uint32_t) subpasses.size(),
				.pSubpasses = subpasses.data(),
				.dependencyCount = (uint32_t) subpassDependencies.size(),
				.pDependencies = subpassDependencies.data(),
		};
		VkRenderPass renderPass;
		VK_API(
				vkCreateRenderPass(device, &renderPassCreateInfo, nullptr,
								   &renderPass))
		return renderPass;
	}

	void destroyRenderPass(VkDevice device, VkRenderPass renderPass) {
		if (renderPass != VK_NULL_HANDLE) {
			vkDestroyRenderPass(device, renderPass, nullptr);
		}
	}

	VkShaderModule createShader(VkDevice device,
								const std::vector<unsigned int> &shaderCode) {
		VkShaderModuleCreateInfo shaderModuleCreateInfo{
				.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.codeSize = shaderCode.size() * 4,
				.pCode = shaderCode.data(),
		};
		VkShaderModule shader;
		VK_API(
				vkCreateShaderModule(device, &shaderModuleCreateInfo, nullptr,
									 &shader))
		return shader;
	}

	void destroyShader(VkDevice device, VkShaderModule shaderModule) {
		if (shaderModule != VK_NULL_HANDLE) {
			vkDestroyShaderModule(device, shaderModule, nullptr);
		}
	}

	struct Vertex {
		glm::vec2 position{};
		glm::vec3 color{};

		Vertex() = default;

		Vertex(glm::vec2 pos, glm::vec3 col) : position(pos), color(col) {}

		enum Attributes {
			ATTRIBUTE_COLOR,
			ATTRIBUTE_POSITION,
		};

		constexpr static VkVertexInputBindingDescription
		getVertexBindingDescription() {
			return {0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX};
		}

		template<Attributes attribute>
		constexpr static VkVertexInputAttributeDescription getAttribute() {
			if constexpr (attribute == ATTRIBUTE_COLOR) {
				return {1, 0, VK_FORMAT_R32G32B32_SFLOAT,
						offsetof(Vertex, color)};
			}
			if constexpr (attribute == ATTRIBUTE_POSITION) {
				return {0, 0, VK_FORMAT_R32G32_SFLOAT,
						offsetof(Vertex, position)};
			}
		}

		static std::vector<VkVertexInputAttributeDescription>
		getAttributeDescriptions() {
			return {getAttribute<ATTRIBUTE_POSITION>(),
					getAttribute<ATTRIBUTE_COLOR>()};
		}
	};

	struct Vertex3D {
		glm::vec3 position{};
		glm::vec3 color{};

		Vertex3D() = default;

		Vertex3D(glm::vec3 pos, glm::vec3 col) : position(pos), color(col) {}

		enum Attributes {
			ATTRIBUTE_COLOR,
			ATTRIBUTE_POSITION,
		};

		constexpr static VkVertexInputBindingDescription
		getVertexBindingDescription() {
			return {0, sizeof(Vertex3D), VK_VERTEX_INPUT_RATE_VERTEX};
		}

		template<Attributes attribute>
		constexpr static VkVertexInputAttributeDescription getAttribute() {
			if constexpr (attribute == ATTRIBUTE_COLOR) {
				return {1, 0, VK_FORMAT_R32G32B32_SFLOAT,
						offsetof(Vertex3D, color)};
			}
			if constexpr (attribute == ATTRIBUTE_POSITION) {
				return {0, 0, VK_FORMAT_R32G32B32_SFLOAT,
						offsetof(Vertex3D, position)};
			}
		}

		static std::vector<VkVertexInputAttributeDescription>
		getAttributeDescriptions() {
			return {getAttribute<ATTRIBUTE_POSITION>(),
					getAttribute<ATTRIBUTE_COLOR>()};
		}
	};

	VkDescriptorPool createDescriptorPool(VkDevice device,
										  std::span<const VkDescriptorPoolSize> descriptorPoolSizes) {
		const VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{
				.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.maxSets = 1,
				.poolSizeCount = (uint32_t) descriptorPoolSizes.size(),
				.pPoolSizes = descriptorPoolSizes.data(),
		};
		VkDescriptorPool descriptorPool;
		VK_API(vkCreateDescriptorPool(device, &descriptorPoolCreateInfo,
									  nullptr, &descriptorPool));
		return descriptorPool;
	}

	void
	destroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool) {
		vkDestroyDescriptorPool(device, descriptorPool, nullptr);
	}

	//TODO if span has const length create array instead
	std::vector<VkDescriptorSet>
	allocateDescriptorSets(VkDevice device, VkDescriptorPool pool,
						   std::span<const VkDescriptorSetLayout> layouts) {
		const VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{
				.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
				.pNext = nullptr,
				.descriptorPool = pool,
				.descriptorSetCount = (uint32_t) layouts.size(),
				.pSetLayouts = layouts.data(),
		};
		std::vector<VkDescriptorSet> descriptorSets(layouts.size());
		VK_API(vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo,
										descriptorSets.data()))
		return descriptorSets;
	}

	void freeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool,
							std::span<const VkDescriptorSet> descriptorSets) {
		VK_API(vkFreeDescriptorSets(device, descriptorPool,
									(uint32_t) descriptorSets.size(),
									descriptorSets.data()));
	}

	void resetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool) {
		VK_API(vkResetDescriptorPool(device, descriptorPool, 0));
	}

	VkDescriptorSetLayout createDescriptorSetLayout(VkDevice device,
													const std::span<const VkDescriptorSetLayoutBinding> bindings) {
		const VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{
				.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.bindingCount = (uint32_t) bindings.size(),
				.pBindings = bindings.data(),
		};
		VkDescriptorSetLayout layout;
		VK_API(vkCreateDescriptorSetLayout(device,
										   &descriptorSetLayoutCreateInfo,
										   nullptr, &layout));
		return layout;
	}

	void
	destroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout layout) {
		vkDestroyDescriptorSetLayout(device, layout, nullptr);
	}

	struct MVP {
		glm::mat4 mvp;

		MVP() = default;

		explicit MVP(glm::mat4 mat) : mvp(mat) {}

		static constexpr const VkDescriptorSetLayoutBinding binding = {.binding = 0,
				.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				.descriptorCount = 1,
				.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
				.pImmutableSamplers = nullptr};

		static constexpr const std::array<VkDescriptorSetLayoutBinding, 1> bindings = {
				binding};
	};


	template<VkBufferUsageFlagBits usage>
	VkBuffer createBuffer(VkDevice device, uint64_t bufferSize) {
		VkBufferCreateInfo bufferCreateInfo{
				.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.size = bufferSize,
				.usage = usage,
				.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
				.queueFamilyIndexCount = 0,
				.pQueueFamilyIndices = nullptr,
		};
		VkBuffer buffer;
		vkCreateBuffer(device, &bufferCreateInfo, nullptr, &buffer);
		return buffer;
	}

	void destroyBuffer(VkDevice device, VkBuffer buffer) {
		vkDestroyBuffer(device, buffer, nullptr);
	}

	VkSampler createSampler(VkDevice device) {
		VkSamplerCreateInfo samplerCreateInfo{
				.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.magFilter = VK_FILTER_NEAREST,
				.minFilter = VK_FILTER_NEAREST,
				.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
				.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
				.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
				.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
				.mipLodBias = 1.0f,
				.anisotropyEnable = VK_FALSE,
				.maxAnisotropy = 1.0f,
				.compareEnable = VK_FALSE,
				.compareOp = VK_COMPARE_OP_ALWAYS,
				.minLod = 0.0f,
				.maxLod = VK_LOD_CLAMP_NONE,
				.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
				.unnormalizedCoordinates = VK_FALSE,
		};
		VkSampler sampler;
		VK_API(vkCreateSampler(device, &samplerCreateInfo, nullptr, &sampler))
		return sampler;
	}

	void vkDestroySampler(VkDevice device, VkSampler sampler) {
		vkDestroySampler(device, sampler, nullptr);
	}

	namespace Blend {
		struct BlendState {
			VkPipelineColorBlendAttachmentState colorBlendAttachmentState{};

			BlendState operator&(VkColorComponentFlagBits bits) {
				BlendState ret = {colorBlendAttachmentState};
				colorBlendAttachmentState.colorWriteMask = bits;
				return ret;
			}
		};

		constexpr const inline BlendState NO_BLEND{{
														   .blendEnable = VK_FALSE,
														   .srcColorBlendFactor = {},
														   .dstColorBlendFactor = {},
														   .colorBlendOp = {},
														   .srcAlphaBlendFactor = {},
														   .dstAlphaBlendFactor = {},
														   .alphaBlendOp = {},
														   .colorWriteMask =
														   VK_COLOR_COMPONENT_R_BIT |
														   VK_COLOR_COMPONENT_G_BIT |
														   VK_COLOR_COMPONENT_B_BIT |
														   VK_COLOR_COMPONENT_A_BIT,
												   }};

		enum BlendColor {
			BLEND_RGB,
			BLEND_A
		};
		enum BlendInput {
			BLEND_SRC,
			BLEND_DST
		};

		template<BlendColor color, BlendInput input>
		struct BlendChannel {
		};
		struct BlendChannelRGBSrc {
		};

		template<BlendInput input>
		struct BlendInputColor {
			BlendChannel<BLEND_A, input> a;
			BlendChannel<BLEND_RGB, input> rgb;
		};

		template<BlendColor color, BlendInput input>
		struct BlendOne {
		};

		template<BlendColor color, BlendInput input>
		BlendOne<color, input> operator-(int, BlendChannel<color, input>) {
			return {};
		}

		template<BlendColor color, BlendInput input>
		struct BlendMul {
			VkBlendFactor factor;
		};

		template<BlendColor color>
		struct BlendMerge {
			VkBlendFactor srcBlendFactor;
			VkBlendFactor dstBlendFactor;
			VkBlendOp BlendOp;
		};

		template<BlendColor color, BlendInput input>
		constexpr BlendMul<color, input>
		operator*(BlendChannel<color, input>, int i) {
			return {i ? VK_BLEND_FACTOR_ONE : VK_BLEND_FACTOR_ZERO};
		}

		template<BlendColor color, BlendInput input>
		constexpr BlendMul<color, input>
		operator*(int i, BlendChannel<color, input>) {
			return {i ? VK_BLEND_FACTOR_ONE : VK_BLEND_FACTOR_ZERO};
		}

		template<BlendColor color, BlendInput input, BlendColor color2,
				BlendInput input2>
		constexpr BlendMul<color, input> operator*(BlendChannel<color, input>,
												   BlendChannel<color2, input2>) {
			if (color2 == BLEND_RGB && input2 == BLEND_SRC) {
				return {VK_BLEND_FACTOR_SRC_COLOR};
			}
			if (color2 == BLEND_A && input2 == BLEND_SRC) {
				return {VK_BLEND_FACTOR_SRC_ALPHA};
			}
			if (color2 == BLEND_RGB && input2 == BLEND_DST) {
				return {VK_BLEND_FACTOR_DST_COLOR};
			}
			if (color2 == BLEND_A && input2 == BLEND_DST) {
				return {VK_BLEND_FACTOR_DST_ALPHA};
			}
		}

		template<BlendColor color, BlendInput input, BlendColor color2,
				BlendInput input2>
		constexpr BlendMul<color, input> operator*(BlendChannel<color, input>,
												   BlendOne<color2, input2>) {
			if (color2 == BLEND_RGB && input2 == BLEND_SRC) {
				return {VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR};
			}
			if (color2 == BLEND_A && input2 == BLEND_SRC) {
				return {VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA};
			}
			if (color2 == BLEND_RGB && input2 == BLEND_DST) {
				return {VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR};
			}
			if (color2 == BLEND_A && input2 == BLEND_DST) {
				return {VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA};
			}
		}

		template<BlendColor color>
		BlendMerge<color> operator+(BlendMul<color, BLEND_DST> dst,
									BlendMul<color, BLEND_SRC> src) {
			BlendMerge <color> ret{
					.srcBlendFactor = src.factor,
					.dstBlendFactor = dst.factor,
					.BlendOp = VK_BLEND_OP_ADD,
			};
			return ret;
		}

		template<BlendColor color>
		BlendMerge<color> operator+(BlendMul<color, BLEND_SRC> src,
									BlendMul<color, BLEND_DST> dst) {
			BlendMerge <color> ret{
					.BlendOp = VK_BLEND_OP_ADD,
					.srcBlendFactor = src.factor,
					.dstBlendFactor = dst.factor,
			};
			return ret;
		}

		template<BlendColor color>
		BlendMerge<color> operator-(BlendMul<color, BLEND_SRC> src,
									BlendMul<color, BLEND_DST> dst) {
			BlendMerge <color> ret{
					.BlendOp = VK_BLEND_OP_SUBTRACT,
					.srcBlendFactor = src.factor,
					.dstBlendFactor = dst.factor,
			};
			return ret;
		}

		template<BlendColor color>
		BlendMerge<color> operator-(BlendMul<color, BLEND_DST> dst,
									BlendMul<color, BLEND_SRC> src) {
			BlendMerge <color> ret{
					.BlendOp = VK_BLEND_OP_REVERSE_SUBTRACT,
					.srcBlendFactor = src.factor,
					.dstBlendFactor = dst.factor,
			};
			return ret;
		}

		BlendState
		operator,(BlendMerge<BLEND_RGB> color, BlendMerge<BLEND_A> alpha) {
			BlendState ret{};
			ret.colorBlendAttachmentState.blendEnable = VK_TRUE;
			ret.colorBlendAttachmentState.srcColorBlendFactor = color.srcBlendFactor;
			ret.colorBlendAttachmentState.dstColorBlendFactor = color.dstBlendFactor;
			ret.colorBlendAttachmentState.colorBlendOp = color.BlendOp;
			ret.colorBlendAttachmentState.srcAlphaBlendFactor = alpha.srcBlendFactor;
			ret.colorBlendAttachmentState.dstAlphaBlendFactor = alpha.dstBlendFactor;
			ret.colorBlendAttachmentState.alphaBlendOp = alpha.BlendOp;
			ret.colorBlendAttachmentState.colorWriteMask =
					VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
					VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			return ret;
		};

		VkPipelineColorBlendAttachmentState getBlendEq(auto f) {
			return f(BlendInputColor<BLEND_DST>(), BlendInputColor<BLEND_SRC>())
					.colorBlendAttachmentState;
		}
	}// namespace Blend

	VkPipelineLayout createPipelineLayout(
			VkDevice device,
			const std::span<const VkDescriptorSetLayout> descriptorSetLayouts,
			const std::span<const VkPushConstantRange> pushConstantRanges) {
		VkPipelineLayoutCreateInfo layoutCreateInfo{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.setLayoutCount = (uint32_t) descriptorSetLayouts.size(),
				.pSetLayouts = descriptorSetLayouts.data(),
				.pushConstantRangeCount = (uint32_t) pushConstantRanges.size(),
				.pPushConstantRanges = pushConstantRanges.data(),
		};
		VkPipelineLayout layout;
		VK_API(vkCreatePipelineLayout(device, &layoutCreateInfo, nullptr,
									  &layout))
		return layout;
	}

	void destroyPipelineLayout(VkDevice device, VkPipelineLayout layout) {
		if (layout != VK_NULL_HANDLE) {
			vkDestroyPipelineLayout(device, layout, nullptr);
		}
	}

	VkPipeline createGraphicsPipeline(
			VkDevice device,
			const std::pair<VkShaderModule, std::string> &vertexShader,
			const std::pair<VkShaderModule, std::string> &fragmentShader,
			const std::vector<VkVertexInputBindingDescription> &inputBindingDescription,
			const std::vector<VkVertexInputAttributeDescription> &attributeDescription,
			VkPipelineLayout layout, VkRenderPass renderPass, uint32_t subpass,
			const std::vector<VkDynamicState> &disable,
			VkExtent2D dimensions = {0, 0},
			VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST) {
		VkPipelineShaderStageCreateInfo vertexShaderInfo{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.stage = VK_SHADER_STAGE_VERTEX_BIT,
				.module = vertexShader.first,
				.pName = vertexShader.second.c_str(),
				.pSpecializationInfo = nullptr,
		};
		VkPipelineShaderStageCreateInfo fragmentShaderInfo{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
				.module = fragmentShader.first,
				.pName = fragmentShader.second.c_str(),
				.pSpecializationInfo = nullptr,
		};
		VkPipelineShaderStageCreateInfo shaderStages[] = {vertexShaderInfo,
														  fragmentShaderInfo};

		VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.vertexBindingDescriptionCount = (uint32_t) inputBindingDescription.size(),
				.pVertexBindingDescriptions = inputBindingDescription.data(),
				.vertexAttributeDescriptionCount = (uint32_t) attributeDescription.size(),
				.pVertexAttributeDescriptions = attributeDescription.data(),
		};

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.topology = topology,
				.primitiveRestartEnable = VK_FALSE,
		};

		VkViewport viewport{
				.x = 0.0,
				.y = 0.0,
				.width = (float) dimensions.width,
				.height = (float) dimensions.height,
				.minDepth = 0.0,
				.maxDepth = 1.0,
		};

		VkRect2D scissor{{0,                0},
						 {dimensions.width, dimensions.height}};

		VkPipelineViewportStateCreateInfo viewportStateCreateInfo{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.viewportCount = 1,
				.pViewports = &viewport,
				.scissorCount = 1,
				.pScissors = &scissor,
		};

		VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.depthClampEnable = VK_FALSE,
				.rasterizerDiscardEnable = VK_FALSE,
				.polygonMode = VK_POLYGON_MODE_FILL,
				.cullMode = VK_CULL_MODE_NONE,
				.frontFace = VK_FRONT_FACE_CLOCKWISE,
				.depthBiasEnable = VK_FALSE,
				.depthBiasConstantFactor = 0.0,
				.depthBiasClamp = 0.0,
				.depthBiasSlopeFactor = 0.0,
				.lineWidth = 1.0,
		};

		VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
				.sampleShadingEnable = VK_FALSE,
				.minSampleShading = 0.0,
				.pSampleMask = nullptr,
				.alphaToCoverageEnable = VK_FALSE,
				.alphaToOneEnable = VK_FALSE,
		};

		VkPipelineColorBlendAttachmentState colorBlendAttachmentState =
				Blend::getBlendEq([](auto, auto) { return Blend::NO_BLEND; });

		VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.logicOpEnable = VK_FALSE,
				.logicOp = VK_LOGIC_OP_NO_OP,
				.attachmentCount = 1,
				.pAttachments = &colorBlendAttachmentState,
				.blendConstants = {0.0, 0.0, 0.0, 0.0},
		};

		VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.dynamicStateCount = (uint32_t) disable.size(),
				.pDynamicStates = disable.data(),
		};

		VkPipelineDynamicStateCreateInfo *pDynamicState =
				disable.empty() ? nullptr : &dynamicStateCreateInfo;

		const VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo{
				.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.stageCount = 2,
				.pStages = shaderStages,
				.pVertexInputState = &vertexInputStateCreateInfo,
				.pInputAssemblyState = &inputAssemblyStateCreateInfo,
				.pTessellationState = nullptr,
				.pViewportState = &viewportStateCreateInfo,
				.pRasterizationState = &rasterizationStateCreateInfo,
				.pMultisampleState = &multisampleStateCreateInfo,
				.pDepthStencilState = nullptr,
				.pColorBlendState = &colorBlendStateCreateInfo,
				.pDynamicState = pDynamicState,
				.layout = layout,
				.renderPass = renderPass,
				.subpass = subpass,
				.basePipelineHandle = VK_NULL_HANDLE,
				.basePipelineIndex = 0,
		};

		VkPipeline pipeline;
		VK_API(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1,
										 &graphicsPipelineCreateInfo, nullptr,
										 &pipeline))

		return pipeline;
	}

	VkPipeline
	createComputePipeline(VkDevice device, VkShaderModule shaderModule,
						  const std::string &mainName = "name") {
		VkComputePipelineCreateInfo computePipelineCreateInfo{
				.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.stage =
						{
								.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
								.pNext = nullptr,
								.flags = 0,
								.stage = VK_SHADER_STAGE_COMPUTE_BIT,
								.module = shaderModule,
								.pName = mainName.c_str(),
								.pSpecializationInfo = nullptr,
						},
				.layout = VK_NULL_HANDLE,
				.basePipelineHandle = VK_NULL_HANDLE,
				.basePipelineIndex = 0,
		};

		VkPipeline pipeline = nullptr;
		VK_API(vkCreateComputePipelines(device, VK_NULL_HANDLE, 1,
										&computePipelineCreateInfo, nullptr,
										&pipeline))
		return pipeline;
	}

	void destroyPipeline(VkDevice device, VkPipeline pipeline) {
		if (pipeline != VK_NULL_HANDLE) {
			vkDestroyPipeline(device, pipeline, nullptr);
		}
	}

	void
	bindComputePipeline(VkCommandBuffer commandBuffer, VkPipeline pipeline) {
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
						  pipeline);
	}

	bool queueCanPresent(VkPhysicalDevice device, uint32_t queueFamilyIndex,
						 VkSurfaceKHR surface) {
		VkBool32 canPresent = false;
		VK_API(vkGetPhysicalDeviceSurfaceSupportKHR(device, queueFamilyIndex,
													surface,
													&canPresent));
		return canPresent;
	}

	VkSurfaceCapabilitiesKHR
	getSurfaceCapabilities(VkPhysicalDevice physicalDevice,
						   VkSurfaceKHR surface) {
		VkSurfaceCapabilitiesKHR surfaceCapabilities;
		VK_API(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice,
														 surface,
														 &surfaceCapabilities));
		return surfaceCapabilities;
	}

	struct SwapchainImageCountInfo {
		uint32_t min;
		uint32_t max;

		[[nodiscard]] bool imagesAvailable(uint32_t images) const {
			if (max == 0) {
				return images >= min;
			} else {
				return images <= max && images >= min;
			}
		}

		[[nodiscard]] uint32_t getClosestTo(uint32_t images) const {
			if (images < min) {
				return min;
			}
			if (max > images && max != 0) {
				return max;
			}
			return images;
		}

		[[nodiscard]] uint32_t getMinimum() const { return min; }
	};

	SwapchainImageCountInfo
	getImageCountInSwapchain(VkSurfaceCapabilitiesKHR surfaceCapabilities) {
		return {surfaceCapabilities.minImageCount,
				surfaceCapabilities.maxImageCount};
	}

	struct SwapchainImageExtendInfo {
		VkExtent2D currentExtent;
		VkExtent2D minImageExtent;
		VkExtent2D maxImageExtent;

		[[nodiscard]] bool hasExtend() const {
			//			return currentExtent.height != 0xFFFFFFFF && 0xFFFFFFFF !=
			//currentExtent.width; CLANG and GCC won't optimize
			return (((uint64_t) currentExtent.height << 32) |
					(uint64_t) currentExtent.width) != (uint64_t) -1ul;
		};
	};

	std::vector<VkSurfaceFormatKHR>
	getSurfaceFormats(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
		uint32_t surfaceFormatCount;
		VK_API(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface,
													&surfaceFormatCount,
													nullptr))
		std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);
		VK_API(vkGetPhysicalDeviceSurfaceFormatsKHR(
				physicalDevice, surface, &surfaceFormatCount,
				surfaceFormats.data()))
		return surfaceFormats;
	}

	enum SignedProperties {
		SIGN_UNDEFINED,
		SIGN_UNSIGNED,
		SIGN_SIGNED,

	};

	enum TypeProperties {
		TYPE_UNDEFINED,
		TYPE_INT,
		TYPE_FLOAT,
	};

	enum TypeRangeProperties {
		TYPE_RANGE_UNDEFINED,
		TYPE_RANGE_DEFAULT,
		TYPE_RANGE_NORM,
		TYPE_RANGE_SCALED,
		TYPE_RANGE_SRGB,
	};

	enum OrderProperties {
		ORDER_UNDEFINED,
		ORDER_RGBA,
		ORDER_ARGB,
		ORDER_BGRA,
		ORDER_ABGR,
	};

	struct FormatLayoutProperties {
		char r = 0;
		char g = 0;
		char b = 0;
		char a = 0;
		SignedProperties hasSign = SIGN_UNDEFINED;
		TypeProperties isInt = TYPE_UNDEFINED;
		TypeRangeProperties range = TYPE_RANGE_UNDEFINED;
		OrderProperties order = ORDER_UNDEFINED;
	};

	VkFormatProperties getFormatProperties(VkPhysicalDevice physicalDevice,
										   VkFormat format) {
		VkFormatProperties properties;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format,
											&properties);
		return properties;
	}

	constexpr FormatLayoutProperties getFormatProperties(VkFormat format) {
		switch (format) {
			case VK_FORMAT_UNDEFINED:
				return {0, 0, 0, 0};
			case VK_FORMAT_R4G4_UNORM_PACK8:
				return {4, 4, 0, 0};
			case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
			case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
				return {4, 4, 4, 4};
			case VK_FORMAT_R5G6B5_UNORM_PACK16:
			case VK_FORMAT_B5G6R5_UNORM_PACK16:
				return {5, 6, 5, 0};
			case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
			case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
			case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
				return {5, 5, 5, 1};
			case VK_FORMAT_R8_UNORM:
			case VK_FORMAT_R8_SNORM:
			case VK_FORMAT_R8_USCALED:
			case VK_FORMAT_R8_SSCALED:
			case VK_FORMAT_R8_UINT:
			case VK_FORMAT_R8_SINT:
			case VK_FORMAT_R8_SRGB:
				return {8, 0, 0, 0};
			case VK_FORMAT_R8G8_UNORM:
			case VK_FORMAT_R8G8_SNORM:
			case VK_FORMAT_R8G8_USCALED:
			case VK_FORMAT_R8G8_SSCALED:
			case VK_FORMAT_R8G8_UINT:
			case VK_FORMAT_R8G8_SINT:
			case VK_FORMAT_R8G8_SRGB:
				return {8, 8, 0, 0};
			case VK_FORMAT_R8G8B8_UNORM:
			case VK_FORMAT_R8G8B8_SNORM:
			case VK_FORMAT_R8G8B8_USCALED:
			case VK_FORMAT_R8G8B8_SSCALED:
			case VK_FORMAT_R8G8B8_UINT:
			case VK_FORMAT_R8G8B8_SINT:
			case VK_FORMAT_R8G8B8_SRGB:
			case VK_FORMAT_B8G8R8_UNORM:
			case VK_FORMAT_B8G8R8_SNORM:
			case VK_FORMAT_B8G8R8_USCALED:
			case VK_FORMAT_B8G8R8_SSCALED:
			case VK_FORMAT_B8G8R8_UINT:
			case VK_FORMAT_B8G8R8_SINT:
			case VK_FORMAT_B8G8R8_SRGB:
				return {8, 8, 8, 0};
			case VK_FORMAT_R8G8B8A8_UNORM:
			case VK_FORMAT_R8G8B8A8_SNORM:
			case VK_FORMAT_R8G8B8A8_USCALED:
			case VK_FORMAT_R8G8B8A8_SSCALED:
			case VK_FORMAT_R8G8B8A8_UINT:
			case VK_FORMAT_R8G8B8A8_SINT:
			case VK_FORMAT_R8G8B8A8_SRGB:
			case VK_FORMAT_B8G8R8A8_UNORM:
			case VK_FORMAT_B8G8R8A8_SNORM:
			case VK_FORMAT_B8G8R8A8_USCALED:
			case VK_FORMAT_B8G8R8A8_SSCALED:
			case VK_FORMAT_B8G8R8A8_UINT:
			case VK_FORMAT_B8G8R8A8_SINT:
			case VK_FORMAT_B8G8R8A8_SRGB:
			case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
			case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
			case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
			case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
			case VK_FORMAT_A8B8G8R8_UINT_PACK32:
			case VK_FORMAT_A8B8G8R8_SINT_PACK32:
			case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
				return {8, 8, 8, 8};
			case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
			case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
			case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
			case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
			case VK_FORMAT_A2R10G10B10_UINT_PACK32:
			case VK_FORMAT_A2R10G10B10_SINT_PACK32:
			case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
			case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
			case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
			case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
			case VK_FORMAT_A2B10G10R10_UINT_PACK32:
			case VK_FORMAT_A2B10G10R10_SINT_PACK32:
				return {10, 10, 10, 2};
			case VK_FORMAT_R16_UNORM:
			case VK_FORMAT_R16_SNORM:
			case VK_FORMAT_R16_USCALED:
			case VK_FORMAT_R16_SSCALED:
			case VK_FORMAT_R16_UINT:
			case VK_FORMAT_R16_SINT:
			case VK_FORMAT_R16_SFLOAT:
				return {16, 0, 0, 0};
			case VK_FORMAT_R16G16_UNORM:
			case VK_FORMAT_R16G16_SNORM:
			case VK_FORMAT_R16G16_USCALED:
			case VK_FORMAT_R16G16_SSCALED:
			case VK_FORMAT_R16G16_UINT:
			case VK_FORMAT_R16G16_SINT:
			case VK_FORMAT_R16G16_SFLOAT:
				return {16, 16, 0, 0};
			case VK_FORMAT_R16G16B16_UNORM:
			case VK_FORMAT_R16G16B16_SNORM:
			case VK_FORMAT_R16G16B16_USCALED:
			case VK_FORMAT_R16G16B16_SSCALED:
			case VK_FORMAT_R16G16B16_UINT:
			case VK_FORMAT_R16G16B16_SINT:
			case VK_FORMAT_R16G16B16_SFLOAT:
				return {16, 16, 16, 0};
			case VK_FORMAT_R16G16B16A16_UNORM:
			case VK_FORMAT_R16G16B16A16_SNORM:
			case VK_FORMAT_R16G16B16A16_USCALED:
			case VK_FORMAT_R16G16B16A16_SSCALED:
			case VK_FORMAT_R16G16B16A16_UINT:
			case VK_FORMAT_R16G16B16A16_SINT:
			case VK_FORMAT_R16G16B16A16_SFLOAT:
				return {16, 16, 16, 16};
			case VK_FORMAT_R32_UINT:
			case VK_FORMAT_R32_SINT:
			case VK_FORMAT_R32_SFLOAT:
				return {32, 0, 0, 0};
			case VK_FORMAT_R32G32_UINT:
			case VK_FORMAT_R32G32_SINT:
			case VK_FORMAT_R32G32_SFLOAT:
				return {32, 32, 0, 0};
			case VK_FORMAT_R32G32B32_UINT:
			case VK_FORMAT_R32G32B32_SINT:
			case VK_FORMAT_R32G32B32_SFLOAT:
				return {32, 32, 32, 0};
			case VK_FORMAT_R32G32B32A32_UINT:
			case VK_FORMAT_R32G32B32A32_SINT:
			case VK_FORMAT_R32G32B32A32_SFLOAT:
				return {32, 32, 32, 32};
			case VK_FORMAT_R64_UINT:
			case VK_FORMAT_R64_SINT:
			case VK_FORMAT_R64_SFLOAT:
				return {64, 0, 0, 0};
			case VK_FORMAT_R64G64_UINT:
			case VK_FORMAT_R64G64_SINT:
			case VK_FORMAT_R64G64_SFLOAT:
				return {64, 64, 0, 0};
			case VK_FORMAT_R64G64B64_UINT:
			case VK_FORMAT_R64G64B64_SINT:
			case VK_FORMAT_R64G64B64_SFLOAT:
				return {64, 64, 64, 0};
			case VK_FORMAT_R64G64B64A64_UINT:
			case VK_FORMAT_R64G64B64A64_SINT:
			case VK_FORMAT_R64G64B64A64_SFLOAT:
				return {64, 64, 64, 64};
			case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
				return {11, 11, 10, 0, SIGN_UNSIGNED, TYPE_FLOAT,
						TYPE_RANGE_DEFAULT,
						ORDER_BGRA};
			default:
				return {0, 0, 0, 0};
		}
	}

	void printFormats(const std::vector<VkSurfaceFormatKHR> &formats) {
		for (auto format: formats) {
			auto properties = getFormatProperties(format.format);
			std::cout << format.format << " " << (int) properties.r << " : "
					  << (int) properties.g << " : " << (int) properties.b
					  << " : "
					  << (int) properties.a << std::endl;
		}
	}

	std::vector<VkPresentModeKHR>
	getPresentModes(VkPhysicalDevice physicalDevice,
					VkSurfaceKHR surface) {
		uint32_t presentModeCount;
		VK_API(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice,
														 surface,
														 &presentModeCount,
														 nullptr))
		std::vector<VkPresentModeKHR> presentModes(presentModeCount);
		VK_API(vkGetPhysicalDeviceSurfacePresentModesKHR(
				physicalDevice, surface, &presentModeCount,
				presentModes.data()))
		return presentModes;
	}

	void printPresentModes(const std::vector<VkPresentModeKHR> &presentModes) {
		for (auto presentMode: presentModes) {
			std::cout <<
					  [](const auto presentMode) {
						  switch (presentMode) {
							  case VK_PRESENT_MODE_IMMEDIATE_KHR:
								  return "VK_PRESENT_MODE_IMMEDIATE_KHR";
							  case VK_PRESENT_MODE_MAILBOX_KHR:
								  return "VK_PRESENT_MODE_MAILBOX_KHR";
							  case VK_PRESENT_MODE_FIFO_KHR:
								  return "VK_PRESENT_MODE_FIFO_KHR";
							  case VK_PRESENT_MODE_FIFO_RELAXED_KHR:
								  return "VK_PRESENT_MODE_FIFO_RELAXED_KHR";
							  case VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR:
								  return "VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR";
							  case VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR:
								  return "VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR";
							  default:
								  return "";
						  }
					  }(presentMode)
					  << std::endl;
		}
	}

	void destroySwapchain(VkDevice device, VkSwapchainKHR swapchain) {

		if (swapchain != VK_NULL_HANDLE) {
			vkDestroySwapchainKHR(device, swapchain, nullptr);
		}
	}

	VkSwapchainKHR createSwapchain(VkDevice device, VkSurfaceKHR surface,
								   uint32_t imagesInSwapchain,
								   VkSurfaceFormatKHR format, VkExtent2D size,
								   uint32_t queueFamily,
								   VkPresentModeKHR presentMode,
								   VkSwapchainKHR oldSwapchain = VK_NULL_HANDLE) {
		VkSwapchainCreateInfoKHR swapchainCreateInfo{
				.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
				.pNext = nullptr,
				.flags = 0,
				.surface = surface,
				.minImageCount = imagesInSwapchain,
				.imageFormat = format.format,
				.imageColorSpace = format.colorSpace,
				.imageExtent = size,
				.imageArrayLayers = 1,
				.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
				.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
				.queueFamilyIndexCount = 1,
				.pQueueFamilyIndices = &queueFamily,
				.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
				.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
				.presentMode = presentMode,
				.clipped = VK_TRUE,
				.oldSwapchain = oldSwapchain,

		};
		VkSwapchainKHR swapchain;
		VK_API(
				vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr,
									 &swapchain))
		destroySwapchain(device, oldSwapchain);
		return swapchain;
	}

	std::vector<VkImage> getSwapchainImage(VkDevice device,
										   VkSwapchainKHR swapchain) {
		uint32_t swapchainImageCount;
		VK_API(vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount,
									   nullptr));
		std::vector<VkImage> swapchainImages(swapchainImageCount);
		VK_API(vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount,
									   swapchainImages.data()));
		return swapchainImages;
	}

	uint32_t
	acquireSwapchainImage(VkDevice device, VkSwapchainKHR swapchain,
						  VkSemaphore semaphoreOnImageAvailable = VK_NULL_HANDLE,
						  VkFence fenceOnImageAvailable = VK_NULL_HANDLE,
						  uint64_t timeout = -1) {
		uint32_t imageIndex;
		VK_API(vkAcquireNextImageKHR(device, swapchain, timeout,
									 semaphoreOnImageAvailable,
									 fenceOnImageAvailable,
									 &imageIndex));
		return imageIndex;
	}

	std::optional<uint32_t> tryToAcquireSwapchainImage(
			VkDevice device, VkSwapchainKHR swapchain,
			VkSemaphore semaphoreOnImageAvailable = VK_NULL_HANDLE,
			VkFence fenceOnImageAvailable = VK_NULL_HANDLE) {
		uint32_t imageIndex;
		VkResult r =
				vkAcquireNextImageKHR(device, swapchain, 0,
									  semaphoreOnImageAvailable,
									  fenceOnImageAvailable, &imageIndex);
		if (r == VK_NOT_READY) {
			return std::nullopt;
		} else if (r == VK_SUCCESS) {
			return imageIndex;
		} else {
			vulkanError(r);
			exit(-1);
		}
	}

	void presentImage(VkQueue queue, const std::vector<VkSemaphore> &semaphores,
					  VkSwapchainKHR swapchain, uint32_t imageIndex) {

		VkPresentInfoKHR presentInfo{
				.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
				.pNext = nullptr,
				.waitSemaphoreCount = (uint32_t) semaphores.size(),
				.pWaitSemaphores = semaphores.data(),
				.swapchainCount = 1,
				.pSwapchains = &swapchain,
				.pImageIndices = &imageIndex,
				.pResults = nullptr,
		};
		VK_API(vkQueuePresentKHR(queue, &presentInfo))
	}

	VkImage createImage2D(VkDevice device, VkFormat format) {
		const VkImageCreateInfo imageCreateInfo{
				.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.imageType = VK_IMAGE_TYPE_2D,
				.format = format,
				.extent = {10, 10, 0},
				.mipLevels = 1,
				.arrayLayers = 1,
				.samples = VK_SAMPLE_COUNT_1_BIT,
				.tiling = VK_IMAGE_TILING_OPTIMAL,
				.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
				.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
				.queueFamilyIndexCount = 0,
				.pQueueFamilyIndices = nullptr,
				.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		};
		VkImage image;
		VK_API(vkCreateImage(device, &imageCreateInfo, nullptr, &image));
		return image;
	}

	void destroyImage(VkDevice device, VkImage image) {
		vkDestroyImage(device, image, nullptr);
	}

	VkImageView
	createImageView(VkDevice device, VkImage image, VkFormat format) {
		VkImageViewCreateInfo imageViewCreateInfo{
				.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.image = image,
				.viewType = VK_IMAGE_VIEW_TYPE_2D,
				.format = format,
				.components = {VK_COMPONENT_SWIZZLE_IDENTITY,
							   VK_COMPONENT_SWIZZLE_IDENTITY,
							   VK_COMPONENT_SWIZZLE_IDENTITY,
							   VK_COMPONENT_SWIZZLE_IDENTITY},
				.subresourceRange = {
						.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
						.baseMipLevel = 0,
						.levelCount = 1,
						.baseArrayLayer = 0,
						.layerCount = 1,
				},
		};
		VkImageView imageView;
		VK_API(vkCreateImageView(device, &imageViewCreateInfo, nullptr,
								 &imageView))
		return imageView;
	}

	std::vector<VkImageView> createImageViews(VkDevice device,
											  const std::vector<VkImage> &images,
											  VkFormat format) {
		std::vector<VkImageView> imageViews;
		imageViews.reserve(images.size());
		for (auto image: images) {
			imageViews.push_back(createImageView(device, image, format));
		}
		return imageViews;
	}

	void destroyImageView(VkDevice device, VkImageView image) {
		if (image != VK_NULL_HANDLE) {
			vkDestroyImageView(device, image, nullptr);
		}
	}

	void
	destroyImageView(VkDevice device, const std::span<VkImageView> &images) {
		for (const auto image: images) {
			if (image == VK_NULL_HANDLE) {
				continue;
			}
			destroyImageView(device, image);
		}
	}

	std::vector<VkFramebuffer>
	createFramebuffers(VkDevice device, VkRenderPass renderPass,
					   const std::vector<VkImageView> &images,
					   VkExtent2D extent) {
		std::vector<VkFramebuffer> framebuffers;
		framebuffers.reserve(images.size());
		for (const auto &image: images) {
			const VkFramebufferCreateInfo framebufferCreateInfo{
					.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
					.pNext = nullptr,
					.flags = 0,
					.renderPass = renderPass,
					.attachmentCount = 1,
					.pAttachments = &image,
					.width = extent.width,
					.height = extent.height,
					.layers = 1,
			};
			VkFramebuffer framebuffer;
			VK_API(vkCreateFramebuffer(device, &framebufferCreateInfo, nullptr,
									   &framebuffer));
			framebuffers.push_back(framebuffer);
		}
		return framebuffers;
	};

	void destroyFramebuffers(VkDevice device,
							 const std::vector<VkFramebuffer> &framebuffers) {
		for (auto frame: framebuffers) {
			vkDestroyFramebuffer(device, frame, nullptr);
		}
	}

	struct Fence {
		VkFence fence;
		VkDevice device;

		explicit Fence(VkDevice d) : device(d) { fence = createFence(device); }

		void wait() const { VK::wait(device, fence); }

		~Fence() { destroyFence(device, fence); }
	};

	template<VkBufferUsageFlagBits usage>
	struct Buffer {
		VkBuffer buffer;

		VkDeviceSize size;
		VkDevice device;

		Buffer(VkDevice d, VkDeviceSize bufferSize) : device(d) {
			size = bufferSize;
			buffer = createBuffer<usage>(device, bufferSize);
		}

		VkMemoryRequirements getRequirements() {
			return getMemoryRequirement(device, buffer);
		}

		~Buffer() { destroyBuffer(device, buffer); }
	};

	template<VkBufferUsageFlagBits usage>
	struct BufferContentPtr {
		Buffer<usage> *buffer;
		VkDeviceSize indexInBuffer;
	};
	/*
        void copyOnce(VkDevice device = nullptr, VkCommandPool commandPool =
   nullptr, Buffer *source = nullptr, Buffer *destination = nullptr, VkQueue
   queue = nullptr) { VkCommandBuffer commandBuffer =
   allocateCommandBuffers(device, commandPool, 1)[0];
                recordCommandBuffer(commandBuffer, [=]() {
                        CMD::copyBuffer(commandBuffer, source->buffer,
   destination->buffer, source->size);
                });
                Fence copyFinished(device);
                queueSubmit(queue, {}, {}, {}, {commandBuffer},
   copyFinished.fence); copyFinished.wait(); freeCommandBuffers(device,
   commandPool, {commandBuffer});
        }
*/
}// namespace VK

#endif// WSAL_VULKAN_RENDER_H
