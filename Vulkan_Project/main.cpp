#include "VDeleter.h"

#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>

#include <iostream>
#include <functional>
#include <vector>
#include <unordered_set>
#include <string>
#include <cstring>

class HelloWorldApplication
{
public:
	void run() 
	{
		initWindow();
		initVulkan();
		mainLoop();
	}

	HelloWorldApplication() {} 

private:
	GLFWwindow* window;

	VDeleter<VkInstance> instance{ vkDestroyInstance };
	VDeleter<VkDebugReportCallbackEXT> callback{ instance, DestroyDebugReportCallbackEXT };
	VkPhysicalDevice physical_device;

	VDeleter<VkDevice> graphics_device{vkDestroyDevice};
	VkQueue graphics_queue;

	const int window_width = 1920;
	const int window_height = 1080;


#ifdef NDEBUG
	// if not debugging
	const bool bEnableValidationLayers = false;
#else
	const bool bEnableValidationLayers = true;
#endif


	void initWindow()
	{
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // no OpenGL context
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		
		window = glfwCreateWindow(window_width, window_height, "Vulkan Hello World", nullptr, nullptr);
	}


	void initVulkan()
	{
		createInstance();
		setupDebugCallback();
		pickPhysicalDevice();
		//createLogicalDevice();
	}


	void mainLoop()
	{
		while (!glfwWindowShouldClose(window))
		{
			glfwPollEvents();
		}
	}

	void createInstance()
	{
		if (bEnableValidationLayers && !checkValidationLayerSupport()) 
		{
			throw std::runtime_error("validation layers requested, but not available!");
		}


		VkApplicationInfo appInfo = {}; // optional
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Vulkan Hello World";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createInfo = {}; // not optional
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		// Getting Vulkan instance extensions required by GLFW
		/*unsigned int glfwExtensionCount = 0;
		const char** glfwExtensions;*/
		auto glfwExtensions = getRequiredExtensions();// glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		// Getting Vulkan supported extensions
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> extensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
		std::unordered_set<std::string> supportedExtensionNames;
		for (const auto& extension : extensions) 
		{
			supportedExtensionNames.insert(std::string(extension.extensionName));
		}

		// Print Vulkan supported extensions
		std::cout << "available extensions:" << std::endl;
		for (const auto& name : supportedExtensionNames) {
			std::cout << "\t" << name << std::endl;
		}
		// Check for and print any unsupported extension
		for (const auto& extension_name : glfwExtensions)
		{
			std::string name(extension_name);
			if (supportedExtensionNames.count(name) <= 0)
			{
				std::cout << "unsupported extension required by GLFW: " << name << std::endl;
			}
		}

		// Enable required extensions
		createInfo.enabledExtensionCount = static_cast<uint32_t>(glfwExtensions.size());
		createInfo.ppEnabledExtensionNames = glfwExtensions.data();

		if (bEnableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else {
			createInfo.enabledLayerCount = 0;
		}

		VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
		if (result != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to create instance!");
		}
		
	}


	const std::vector<const char*> validationLayers = {
		"VK_LAYER_LUNARG_standard_validation"
	};

	bool checkValidationLayerSupport()
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : validationLayers) 
		{
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers) 
			{
				if (strcmp(layerName, layerProperties.layerName) == 0) 
				{
					layerFound = true;
					break;
				}
			}

			if (!layerFound) {
				return false;
			}
		}

		return true;
	}

	std::vector<const char*> getRequiredExtensions()
	{
		std::vector<const char*> extensions;

		unsigned int glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		for (unsigned int i = 0; i < glfwExtensionCount; i++) 
		{
			extensions.push_back(glfwExtensions[i]);
		}

		if (bEnableValidationLayers) 
		{
			extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
		}

		return extensions;
		// should I free sth after?
	}

	void setupDebugCallback() 
	{
		if (!bEnableValidationLayers) return;

		VkDebugReportCallbackCreateInfoEXT createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
		createInfo.pfnCallback = (PFN_vkDebugReportCallbackEXT)debugCallback;

		if (CreateDebugReportCallbackEXT(instance, &createInfo, nullptr, &callback) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to set up debug callback!");
		}

		
	}

	VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback) 
	{
		auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
		if (func != nullptr) {
			return func(instance, pCreateInfo, pAllocator, pCallback);
		}
		else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	static void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator) 
	{
		auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
		if (func != nullptr) {
			func(instance, callback, pAllocator);
		}
	}

	// the debug callback function that Vulkan runs
	static VkBool32 debugCallback(
		VkDebugReportFlagsEXT flags,
		VkDebugReportObjectTypeEXT objType,
		uint64_t obj,
		size_t location,
		int32_t code,
		const char* layerPrefix,
		const char* msg,
		void* userData) 
	{

		std::cerr << "validation layer: " << msg << std::endl;

		return VK_FALSE;
	}

	// Pick up a graphics card to use
	void pickPhysicalDevice()
	{
		// This object will be implicitly destroyed when the VkInstance is destroyed, so we don't need to add a delete wrapper.
		VkPhysicalDevice physial_device = VK_NULL_HANDLE;
		uint32_t device_count = 0;
		vkEnumeratePhysicalDevices(instance, &device_count, nullptr);

		if (device_count == 0)
		{
			throw std::runtime_error("Failed to find GPUs with Vulkan support!");
		}

		std::vector<VkPhysicalDevice> devices(device_count);
		vkEnumeratePhysicalDevices(instance, &device_count, devices.data());
		//TODO: continue here

		for (const auto& device : devices)
		{
			if (isDeviceSuitable(device))
			{
				physial_device = device;
				break;
			}
		}

		if (physial_device == VK_NULL_HANDLE) 
		{
			throw std::runtime_error("Failed to find a suitable GPU!");
		}
		else
		{
			VkPhysicalDeviceProperties properties;
			vkGetPhysicalDeviceProperties(physial_device, &properties);
			std::cout << "Current Device: " << properties.deviceName << std::endl;
		}

		this->physical_device = physial_device;
	}

	bool isDeviceSuitable(VkPhysicalDevice device)
	{
		//VkPhysicalDeviceProperties properties;
		//vkGetPhysicalDeviceProperties(device, &properties);

		//VkPhysicalDeviceFeatures features;
		//vkGetPhysicalDeviceFeatures(device, &features);

		//return properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
		//	&& features.geometryShader;

		QueueFamilyIndices indices = findQueueFamilies(device);

		//return false;
		return indices.isComplete();
		//return true;
	}

	struct QueueFamilyIndices 
	{
		int graphicsFamily = -1;

		bool isComplete() 
		{
			return graphicsFamily >= 0;
		}
	};

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) 
	{
		QueueFamilyIndices indices;

		uint32_t queuefamily_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queuefamily_count, nullptr);

		std::vector<VkQueueFamilyProperties> queuefamilies(queuefamily_count);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queuefamily_count, queuefamilies.data());

		int i = 0;
		for (const auto& queuefamily : queuefamilies) 
		{
			if (queuefamily.queueCount > 0 && queuefamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) 
			{
				// Graphics queue_family
				indices.graphicsFamily = i;
			}

			if (indices.isComplete()) {
				break;
			}

			i++;
		}


		return indices;
	}

	void createLogicalDevice() 
	{
		QueueFamilyIndices indices = findQueueFamilies(physical_device);

		// Create a graphics qeue
		VkDeviceQueueCreateInfo queue_create_info = {};
		queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_create_info.queueFamilyIndex = indices.graphicsFamily;
		queue_create_info.queueCount = 1;
		// Required
		float queue_priority = 1.0f;
		queue_create_info.pQueuePriorities = &queue_priority;

		// Specify used device features
		VkPhysicalDeviceFeatures device_features = {}; // Everything is by default VK_FALSE

		// Create the logical device
		VkDeviceCreateInfo device_create_info = {};
		device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		device_create_info.pQueueCreateInfos = &queue_create_info;
		device_create_info.queueCreateInfoCount = 1;
		device_create_info.pEnabledFeatures = &device_features;
		
		device_create_info.enabledExtensionCount = 0;
		if (bEnableValidationLayers)
		{
			device_create_info.enabledLayerCount = validationLayers.size();
			device_create_info.ppEnabledLayerNames = validationLayers.data();
		}
		else
		{
			device_create_info.enabledLayerCount = 0;
		}

		auto result = vkCreateDevice(physical_device, &device_create_info, nullptr, &graphics_device);

		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create logical device!");
		}

		vkGetDeviceQueue(graphics_device, indices.graphicsFamily, 0, &graphics_queue);
	}
};

int main() 
{
	HelloWorldApplication app;

	try 
	{
		app.run();
	}
	catch (const std::runtime_error& e) 
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}


