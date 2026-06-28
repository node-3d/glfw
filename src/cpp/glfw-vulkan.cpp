#include <iostream>

#if (defined(VK_USE_PLATFORM_MACOS_MVK) && defined(VK_EXAMPLE_XCODE_GENERATED))
	#include <Cocoa/Cocoa.h>
	#include <QuartzCore/CAMetalLayer.h>
	#include <CoreVideo/CVDisplayLink.h>
#endif

#include "glfw-common.hpp"
#include "glfw-timers.hpp"


#define THIS_VULKAN                                                            \
	REQ_OFFS_ARG(0, __vk_instance);                                            \
	VkInstance instance = reinterpret_cast<VkInstance>(__vk_instance);

#define VK_INSTANCE_LEVEL_FUNCTION(name)                                       \
	PFN_##name name = reinterpret_cast<PFN_##name>(                            \
		glfwGetInstanceProcAddress(instance, #name)                            \
	);


namespace glfw {


Napi::Array toStringArray(Napi::Env env, const char **strings, size_t count) {
	Napi::Array arr = JS_ARRAY;
	
	for (size_t i = 0; i < count; i++) {
		arr.Set(i, strings[i]);
	}
	
	return arr;
}


DBG_EXPORT JS_METHOD(vulkanSupported) { NAPI_ENV;
	RET_BOOL(glfwVulkanSupported());
}


DBG_EXPORT JS_METHOD(getRequiredInstanceExtensions) { NAPI_ENV;
	uint32_t count;
	const char** extensions = glfwGetRequiredInstanceExtensions(&count);
	RET_VALUE(toStringArray(env, extensions, count));
}


DBG_EXPORT JS_METHOD(getInstanceProcAddress) { NAPI_ENV; THIS_VULKAN;
	REQ_STR_ARG(1, name)
	uint64_t addr = reinterpret_cast<uint64_t>(
		glfwGetInstanceProcAddress(instance, name.c_str())
	);
	RET_NUM(addr);
}


DBG_EXPORT JS_METHOD(getPhysicalDevicePresentationSupport) { NAPI_ENV; THIS_VULKAN;
	REQ_OFFS_ARG(1, __vk_physDevice);
	VkPhysicalDevice physDevice = reinterpret_cast<VkPhysicalDevice>(__vk_physDevice);
	REQ_UINT_ARG(2, queuefamily);
	auto result = glfwGetPhysicalDevicePresentationSupport(instance, physDevice, queuefamily);
	RET_BOOL(result == GLFW_TRUE);
}


DBG_EXPORT JS_METHOD(createWindowSurface) { NAPI_ENV; THIS_VULKAN;
	REQ_OFFS_ARG(1, __win_handle);
	GLFWwindow *window = reinterpret_cast<GLFWwindow*>(__win_handle);
	
	LET_OFFS_ARG(2, __alloc_handle);
	VkAllocationCallbacks *allocator = reinterpret_cast<VkAllocationCallbacks*>(__alloc_handle);
	
	VkSurfaceKHR surface;
	auto result = glfwCreateWindowSurface(instance, window, allocator, &surface);
	
	if (result != VK_SUCCESS) {
		RET_NULL;
	}
	
	RET_PTR(surface);
}


DBG_EXPORT JS_METHOD(vulkanCreateInstance) { NAPI_ENV;
	VkApplicationInfo appInfo;
	memset(&appInfo, 0, sizeof(VkApplicationInfo));
	
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.apiVersion = VK_HEADER_VERSION_COMPLETE;
	
	VkInstanceCreateInfo createInfo;
	memset(&createInfo, 0, sizeof(VkInstanceCreateInfo));
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	
	createInfo.enabledExtensionCount = glfwExtensionCount;
	createInfo.ppEnabledExtensionNames = glfwExtensions;
	
	VkInstance instance = nullptr;
	VK_INSTANCE_LEVEL_FUNCTION(vkCreateInstance);
	
	VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
	if (result != VK_SUCCESS) {
		std::cerr << "Error: vkCreateInstance failed. Code: " << result << "." << std::endl;
		RET_NULL;
	}
	
	RET_PTR(instance);
}


bool checkDeviceProperties(
	VkInstance instance,
	VkPhysicalDevice physical_device,
	uint32_t *queue_family_index
) {
	VK_INSTANCE_LEVEL_FUNCTION(vkGetPhysicalDeviceQueueFamilyProperties);
	VK_INSTANCE_LEVEL_FUNCTION(vkGetPhysicalDeviceProperties);
	VK_INSTANCE_LEVEL_FUNCTION(vkGetPhysicalDeviceFeatures);
	
	VkPhysicalDeviceProperties device_properties;
	VkPhysicalDeviceFeatures device_features;
	
	vkGetPhysicalDeviceProperties(physical_device, &device_properties);
	vkGetPhysicalDeviceFeatures(physical_device, &device_features);
	
	uint32_t versionMajor = VK_API_VERSION_MAJOR(device_properties.apiVersion);
	
	if ((versionMajor < 1) && (device_properties.limits.maxImageDimension2D < 4096)) {
		std::cerr << "Device " << physical_device << " doesn't support required parameters." << std::endl;
		return false;
	}
	
	uint32_t queue_families_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_families_count, nullptr);
	if (queue_families_count == 0) {
		std::cerr << "Device " << physical_device << " doesn't have any queue families." << std::endl;
		return false;
	}
	
	std::vector<VkQueueFamilyProperties> queue_family_properties(queue_families_count);
	vkGetPhysicalDeviceQueueFamilyProperties(
		physical_device,
		&queue_families_count,
		&queue_family_properties[0]
	);
	for(uint32_t i = 0; i < queue_families_count; ++i) {
		if (
			(queue_family_properties[i].queueCount > 0) &&
			(queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		) {
			*queue_family_index = i;
			return true;
		}
	}
	
	std::cerr << "Error: Could not find a good queue family on " << physical_device << "." << std::endl;
	return false;
}


DBG_EXPORT JS_METHOD(vulkanCreateDevice) { NAPI_ENV; THIS_VULKAN;
	VK_INSTANCE_LEVEL_FUNCTION(vkEnumeratePhysicalDevices);
	VK_INSTANCE_LEVEL_FUNCTION(vkCreateDevice);
	
	uint32_t num_devices = 0;
	if (
		(vkEnumeratePhysicalDevices(instance, &num_devices, nullptr) != VK_SUCCESS) ||
		(num_devices == 0)
	) {
		std::cerr << "Error: can't count physical devices." << std::endl;
		RET_NULL;
	}
	
	std::vector<VkPhysicalDevice> physical_devices(num_devices);
	if (vkEnumeratePhysicalDevices(instance, &num_devices, &physical_devices[0]) != VK_SUCCESS) {
		std::cerr << "Error: can't enumerate physical devices." << std::endl;
		RET_NULL;
	}
	
	VkPhysicalDevice selected_physical_device = VK_NULL_HANDLE;
	uint32_t selected_queue_family_index = UINT32_MAX;
	for(uint32_t i = 0; i < num_devices; ++i) {
		if (checkDeviceProperties(instance, physical_devices[i], &selected_queue_family_index)) {
			selected_physical_device = physical_devices[i];
		}
	}
	if (selected_physical_device == VK_NULL_HANDLE) {
		std::cerr << "Error: Could not find a suitable Vulkan physical device." << std::endl;
		RET_NULL;
	}
	
	std::vector<float> queue_priorities = { 1.0f };
	
	VkDeviceQueueCreateInfo queue_create_info = {
		VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,     // VkStructureType           sType
		nullptr,                                        // const void               *pNext
		0,                                              // VkDeviceQueueCreateFlags  flags
		selected_queue_family_index,                    // uint32_t                  queueFamilyIndex
		static_cast<uint32_t>(queue_priorities.size()), // uint32_t                  queueCount
		&queue_priorities[0]                            // const float              *pQueuePriorities
	};

	VkDeviceCreateInfo device_create_info = {
		VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, // VkStructureType                 sType
		nullptr,                              // const void                     *pNext
		0,                                    // VkDeviceCreateFlags             flags
		1,                                    // uint32_t                        queueCreateInfoCount
		&queue_create_info,                   // const VkDeviceQueueCreateInfo  *pQueueCreateInfos
		0,                                    // uint32_t                        enabledLayerCount
		nullptr,                              // const char * const             *ppEnabledLayerNames
		0,                                    // uint32_t                        enabledExtensionCount
		nullptr,                              // const char * const             *ppEnabledExtensionNames
		nullptr                               // const VkPhysicalDeviceFeatures *pEnabledFeatures
	};
	
	VkDevice device;
	if (vkCreateDevice(selected_physical_device, &device_create_info, nullptr, &device) != VK_SUCCESS) {
		std::cerr << "Could not create Vulkan device!" << std::endl;
		RET_NULL;
	}
	
	auto object = JS_OBJECT;
	object.Set("device", JS_NUM(reinterpret_cast<uint64_t>(device)));
	object.Set("physicalDevice", JS_NUM(reinterpret_cast<uint64_t>(selected_physical_device)));
	object.Set("queueFamily", JS_NUM(selected_queue_family_index));
	
	RET_VALUE(object);
}


DBG_EXPORT JS_METHOD(vulkanDestroyDevice) { NAPI_ENV; THIS_VULKAN;
	REQ_OFFS_ARG(1, __vk_device);
	VkDevice device = reinterpret_cast<VkDevice>(__vk_device);
	
	VK_INSTANCE_LEVEL_FUNCTION(vkDeviceWaitIdle);
	VK_INSTANCE_LEVEL_FUNCTION(vkDestroyDevice);
	
	if (device != VK_NULL_HANDLE) {
		vkDeviceWaitIdle(device);
		vkDestroyDevice(device, nullptr);
	}
	
	RET_GLFW_VOID;
}


DBG_EXPORT JS_METHOD(vulkanDestroyInstance) { NAPI_ENV; THIS_VULKAN;
	VK_INSTANCE_LEVEL_FUNCTION(vkDestroyInstance);
	
	if (instance != VK_NULL_HANDLE) {
		vkDestroyInstance(instance, nullptr);
	}
	
	RET_GLFW_VOID;
}

} // namespace glfw
