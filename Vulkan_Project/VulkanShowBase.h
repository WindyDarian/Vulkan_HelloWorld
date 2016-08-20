#pragma once

#include "VDeleter.h"

#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>

#include <vector>

struct QueueFamilyIndices
{
	int graphicsFamily = -1;
	int presentFamily = -1;

	bool isComplete()
	{
		return graphicsFamily >= 0 && presentFamily >= 0;
	}

	static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
};

class VulkanShowBase
{
public:
	VulkanShowBase();
	void run();

private:
	GLFWwindow* window;

	VDeleter<VkInstance> instance{ vkDestroyInstance };
	VDeleter<VkDebugReportCallbackEXT> callback{ instance, DestroyDebugReportCallbackEXT };
	VkPhysicalDevice physical_device;

	VDeleter<VkDevice> graphics_device{ vkDestroyDevice };
	VkQueue graphics_queue;

	VDeleter<VkSurfaceKHR> window_surface{ instance, vkDestroySurfaceKHR };
	VkQueue present_queue;

	const int window_width = 1920;
	const int window_height = 1080;


#ifdef NDEBUG
	// if not debugging
	const bool ENABLE_VALIDATION_LAYERS = false;
#else
	const bool ENABLE_VALIDATION_LAYERS = true;
#endif

	const std::vector<const char*> VALIDATION_LAYERS = {
		"VK_LAYER_LUNARG_standard_validation"
	};

	const std::vector<const char*> DEVICE_EXTENSIONS = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	void initWindow();
	void initVulkan();
	void createWindowSurface();
	void mainLoop();
	void createInstance();

	bool checkValidationLayerSupport();
	std::vector<const char*> getRequiredExtensions();
	void setupDebugCallback();
	void pickPhysicalDevice();
	bool isDeviceSuitable(VkPhysicalDevice device);
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	void createLogicalDevice();

	static void DestroyDebugReportCallbackEXT(VkInstance instance
		, VkDebugReportCallbackEXT callback
		, const VkAllocationCallbacks* pAllocator);

	static VkResult CreateDebugReportCallbackEXT(VkInstance instance
		, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo
		, const VkAllocationCallbacks* pAllocator
		, VkDebugReportCallbackEXT* pCallback);
};

