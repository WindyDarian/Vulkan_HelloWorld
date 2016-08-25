#include "VulkanShowBase.h"

#include <iostream>
#include <functional>
#include <vector>
#include <unordered_set>
#include <string>
#include <cstring>
#include <set>
#include <algorithm>
#include <fstream>

static std::vector<char> readFile(const std::string& filename) 
{
	std::ifstream file_stream(filename, std::ios::ate | std::ios::binary);

	if (!file_stream.is_open()) 
	{
		throw std::runtime_error("failed to open file!");
	}

	// starts reading at the end of file to determine file size (ate)
	size_t file_size = (size_t)file_stream.tellg();
	std::vector<char> buffer(file_size);

	file_stream.seekg(0);
	file_stream.read(buffer.data(), file_size);

	file_stream.close();
	return buffer;
}

struct SwapChainSupportDetails 
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> present_modes;

	static SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) 
	{
		SwapChainSupportDetails details;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

		// Getting supported surface formats
		uint32_t format_count;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, nullptr);
		if (format_count != 0) 
		{
			details.formats.resize(format_count);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, details.formats.data());
		}

		// Getting supported present modes
		uint32_t present_mode_count;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, nullptr);
		if (present_mode_count != 0) 
		{
			details.present_modes.resize(present_mode_count);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, details.present_modes.data());
		}

		return details;
	}
};

QueueFamilyIndices QueueFamilyIndices::findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
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

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
		if (queuefamily.queueCount > 0 && presentSupport)
		{
			// Graphics queue_family
			indices.presentFamily = i;
		}

		if (indices.isComplete()) {
			break;
		}

		i++;
	}


	return indices;
}

VulkanShowBase::VulkanShowBase()
{
}



void VulkanShowBase::run()
{
	initWindow();
	initVulkan();
	mainLoop();
}

VkResult VulkanShowBase::CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback)
{
	auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
	if (func != nullptr) 
	{
		return func(instance, pCreateInfo, pAllocator, pCallback);
	}
	else 
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void VulkanShowBase::DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
	if (func != nullptr) {
		func(instance, callback, pAllocator);
	}
}

// the debug callback function that Vulkan runs
VkBool32 debugCallback(
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

void VulkanShowBase::initWindow()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // no OpenGL context

	if (WINDOW_RESIZABLE)
	{
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	}
	else
	{
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	}

	window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Vulkan Hello World", nullptr, nullptr);

	glfwSetWindowUserPointer(window, this);
	glfwSetWindowSizeCallback(window, VulkanShowBase::onWindowResized);
}

void VulkanShowBase::onWindowResized(GLFWwindow * window, int width, int height)
{
	if (width == 0 || height == 0) return;

	VulkanShowBase* app = reinterpret_cast<VulkanShowBase*>(glfwGetWindowUserPointer(window));
	app->recreateSwapChain();
}

void VulkanShowBase::initVulkan()
{
	createInstance();
	setupDebugCallback();
	createWindowSurface();
	pickPhysicalDevice();
	createLogicalDevice();
	createSwapChain();
	createImageViews();
	createRenderPass();
	createGraphicsPipeline();
	createFrameBuffers();
	createCommandPool();
	// TODO: better to use a single memory allocation for multiple buffers
	createVertexBuffer();
	createIndexBuffer();
	createCommandBuffers();
	createSemaphores();
}

// Needs to be called right after instance creation because it may influence device selection
void VulkanShowBase::createWindowSurface()
{
	auto result = glfwCreateWindowSurface(instance, window, nullptr, &window_surface);

	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create window surface!");
	}
}

void VulkanShowBase::mainLoop()
{
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		drawFrame();
	}

	vkDeviceWaitIdle(graphics_device);
}

void VulkanShowBase::recreateSwapChain()
{
	vkDeviceWaitIdle(graphics_device);

	createSwapChain();
	createImageViews();
	createRenderPass();
	createGraphicsPipeline();
	createFrameBuffers();
	createCommandBuffers();
}

void VulkanShowBase::createInstance()
{
	if (ENABLE_VALIDATION_LAYERS && !checkValidationLayerSupport())
	{
		throw std::runtime_error("validation layers requested, but not available!");
	}


	VkApplicationInfo app_info = {}; // optional
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pApplicationName = "Vulkan Hello World";
	app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	app_info.pEngineName = "No Engine";
	app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	app_info.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo instance_info = {}; // not optional
	instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instance_info.pApplicationInfo = &app_info;

	// Getting Vulkan instance extensions required by GLFW
	auto glfwExtensions = getRequiredExtensions();

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
	instance_info.enabledExtensionCount = static_cast<uint32_t>(glfwExtensions.size());
	instance_info.ppEnabledExtensionNames = glfwExtensions.data();

	if (ENABLE_VALIDATION_LAYERS) {
		instance_info.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
		instance_info.ppEnabledLayerNames = VALIDATION_LAYERS.data();
	}
	else {
		instance_info.enabledLayerCount = 0;
	}

	VkResult result = vkCreateInstance(&instance_info, nullptr, &instance);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create instance!");
	}

}

bool VulkanShowBase::checkValidationLayerSupport()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : VALIDATION_LAYERS)
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

std::vector<const char*> VulkanShowBase::getRequiredExtensions()
{
	std::vector<const char*> extensions;

	unsigned int glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	for (unsigned int i = 0; i < glfwExtensionCount; i++)
	{
		extensions.push_back(glfwExtensions[i]);
	}

	if (ENABLE_VALIDATION_LAYERS)
	{
		extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	}

	return extensions;
	// should I free sth after?
}

void VulkanShowBase::setupDebugCallback()
{
	if (!ENABLE_VALIDATION_LAYERS) return;

	VkDebugReportCallbackCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
	createInfo.pfnCallback = (PFN_vkDebugReportCallbackEXT)debugCallback;

	if (CreateDebugReportCallbackEXT(instance, &createInfo, nullptr, &callback) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to set up debug callback!");
	}


}


// Pick up a graphics card to use
void VulkanShowBase::pickPhysicalDevice()
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

bool VulkanShowBase::isDeviceSuitable(VkPhysicalDevice device)
{
	//VkPhysicalDeviceProperties properties;
	//vkGetPhysicalDeviceProperties(device, &properties);

	//VkPhysicalDeviceFeatures features;
	//vkGetPhysicalDeviceFeatures(device, &features);

	//return properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
	//	&& features.geometryShader;

	QueueFamilyIndices indices = QueueFamilyIndices::findQueueFamilies(device, static_cast<VkSurfaceKHR>(window_surface));

	bool extensions_supported = checkDeviceExtensionSupport(device);

	bool swap_chain_adequate = false;
	if (extensions_supported)
	{
		auto swap_chain_support = SwapChainSupportDetails::querySwapChainSupport(device, static_cast<VkSurfaceKHR>(window_surface));
		swap_chain_adequate = !swap_chain_support.formats.empty() && !swap_chain_support.present_modes.empty();
	}

	return indices.isComplete() && extensions_supported && swap_chain_adequate;
}

bool VulkanShowBase::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
	uint32_t extension_count;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);

	std::vector<VkExtensionProperties> available_extensions(extension_count);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, available_extensions.data());

	std::set<std::string> required_extensions(DEVICE_EXTENSIONS.begin(), DEVICE_EXTENSIONS.end());

	for (const auto& extension : available_extensions) 
	{
		required_extensions.erase(extension.extensionName);
	}

	return required_extensions.empty();
}

void VulkanShowBase::createLogicalDevice()
{
	QueueFamilyIndices indices = QueueFamilyIndices::findQueueFamilies(physical_device, static_cast<VkSurfaceKHR>(window_surface));

	std::vector <VkDeviceQueueCreateInfo> queue_create_infos;
	std::set<int> queue_families = { indices.graphicsFamily, indices.presentFamily };

	float queue_priority = 1.0f;
	for (int family : queue_families)
	{
		// Create a graphics queue
		VkDeviceQueueCreateInfo queue_create_info = {};
		queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_create_info.queueFamilyIndex = indices.graphicsFamily;
		queue_create_info.queueCount = 1;

		queue_create_info.pQueuePriorities = &queue_priority;
		queue_create_infos.push_back(queue_create_info);
	}

	// Specify used device features
	VkPhysicalDeviceFeatures device_features = {}; // Everything is by default VK_FALSE

												   // Create the logical device
	VkDeviceCreateInfo device_create_info = {};
	device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_create_info.pQueueCreateInfos = queue_create_infos.data();
	device_create_info.queueCreateInfoCount = static_cast<uint32_t> (queue_create_infos.size());

	device_create_info.pEnabledFeatures = &device_features;

	if (ENABLE_VALIDATION_LAYERS)
	{
		device_create_info.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
		device_create_info.ppEnabledLayerNames = VALIDATION_LAYERS.data();
	}
	else
	{
		device_create_info.enabledLayerCount = 0;
	}

	device_create_info.enabledExtensionCount = static_cast<uint32_t>(DEVICE_EXTENSIONS.size());
	device_create_info.ppEnabledExtensionNames = DEVICE_EXTENSIONS.data();

	auto result = vkCreateDevice(physical_device, &device_create_info, nullptr, &graphics_device);

	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create logical device!");
	}

	vkGetDeviceQueue(graphics_device, indices.graphicsFamily, 0, &graphics_queue);
	vkGetDeviceQueue(graphics_device, indices.presentFamily, 0, &present_queue);
}

void VulkanShowBase::createSwapChain()
{
	auto support_details = SwapChainSupportDetails::querySwapChainSupport(physical_device, window_surface);

	VkSurfaceFormatKHR surface_format = chooseSwapSurfaceFormat(support_details.formats);
	VkPresentModeKHR present_mode = chooseSwapPresentMode(support_details.present_modes);
	VkExtent2D extent = chooseSwapExtent(support_details.capabilities);

	uint32_t queue_length = support_details.capabilities.minImageCount + 1;
	if (support_details.capabilities.maxImageCount > 0 && queue_length > support_details.capabilities.maxImageCount) 
	{
		// 0 for maxImageCount means no limit
		queue_length = support_details.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	create_info.surface = window_surface;
	create_info.minImageCount = queue_length;
	create_info.imageFormat = surface_format.format;
	create_info.imageColorSpace = surface_format.colorSpace;
	create_info.imageExtent = extent;
	create_info.imageArrayLayers = 1; // >1 when developing stereoscopic application
	create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // render directly
	// VK_IMAGE_USAGE_TRANSFER_DST_BIT and memory operation to enable post processing

	QueueFamilyIndices indices = QueueFamilyIndices::findQueueFamilies(physical_device, window_surface);
	uint32_t queueFamilyIndices[] = { (uint32_t)indices.graphicsFamily, (uint32_t)indices.presentFamily };

	if (indices.graphicsFamily != indices.presentFamily) 
	{
		create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		create_info.queueFamilyIndexCount = 2;
		create_info.pQueueFamilyIndices = queueFamilyIndices;
	}
	else 
	{
		create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		create_info.queueFamilyIndexCount = 0; // Optional
		create_info.pQueueFamilyIndices = nullptr; // Optional
	}

	create_info.preTransform = support_details.capabilities.currentTransform; // not doing any transformation
	create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // ignore alpha channel (for blending with other windows)

	create_info.presentMode = present_mode;
	create_info.clipped = VK_TRUE; // ignore pixels obscured

	auto old_swap_chain = std::move(swap_chain); //which will be destroyed when out of scope
	create_info.oldSwapchain = old_swap_chain; // required when recreating a swap chain (like resizing windows)

	auto result = vkCreateSwapchainKHR(graphics_device, &create_info, nullptr, &swap_chain);

	if (result != VK_SUCCESS) 
	{
		throw std::runtime_error("Failed to create swap chain!");
	}

	uint32_t image_count;
	vkGetSwapchainImagesKHR(graphics_device, swap_chain, &image_count, nullptr);
	swap_chain_images.resize(image_count);
	vkGetSwapchainImagesKHR(graphics_device, swap_chain, &image_count, swap_chain_images.data());

	swap_chain_image_format = surface_format.format;
	swap_chain_extent = extent;
}

void VulkanShowBase::createImageViews()
{
	swap_chain_imageviews.clear(); // VDeleter will delete old objects
	swap_chain_imageviews.reserve(swap_chain_images.size());

	for (uint32_t i = 0; i < swap_chain_images.size(); i++) 
	{
		swap_chain_imageviews.push_back(std::move(VDeleter<VkImageView>( graphics_device, vkDestroyImageView )));

		VkImageViewCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		create_info.image = swap_chain_images[i];
		create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		create_info.format = swap_chain_image_format;

		// no swizzle
		create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		create_info.subresourceRange.baseMipLevel = 0;
		create_info.subresourceRange.levelCount = 1;
		create_info.subresourceRange.baseArrayLayer = 0;
		create_info.subresourceRange.layerCount = 1;

		auto result = vkCreateImageView(graphics_device, &create_info, nullptr, &swap_chain_imageviews[i]);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create image views!");
		}
	}
}

void VulkanShowBase::createRenderPass()
{
	VkAttachmentDescription color_attachment = {};
	color_attachment.format = swap_chain_image_format;
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // before rendering
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // after rendering
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; // no stencil
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // to be directly used in swap chain

	VkAttachmentReference color_attachment_ref = {};
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;

	// overwrite subpass dependency to make it wait until VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0; // 0  refers to the subpass
	dependency.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependency.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo render_pass_info = {};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_info.attachmentCount = 1;
	render_pass_info.pAttachments = &color_attachment;
	render_pass_info.subpassCount = 1;
	render_pass_info.pSubpasses = &subpass;
	render_pass_info.dependencyCount = 1;
	render_pass_info.pDependencies = &dependency;

	if (vkCreateRenderPass(graphics_device, &render_pass_info, nullptr, &render_pass) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to create render pass!");
	}
}

void VulkanShowBase::createGraphicsPipeline()
{
	auto vert_shader_code = readFile("content/helloworld_vert.spv");
	auto frag_shader_code = readFile("content/helloworld_frag.spv");

	createShaderModule(vert_shader_code, &vert_shader_module);
	createShaderModule(frag_shader_code, &frag_shader_module);

	VkPipelineShaderStageCreateInfo vert_shader_stage_info = {};
	vert_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vert_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vert_shader_stage_info.module = vert_shader_module;
	vert_shader_stage_info.pName = "main";

	VkPipelineShaderStageCreateInfo frag_shader_stage_info = {};
	frag_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	frag_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	frag_shader_stage_info.module = frag_shader_module;
	frag_shader_stage_info.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vert_shader_stage_info, frag_shader_stage_info };

	// vertex data info
	VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
	vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	
	auto binding_description = Vertex::getBindingDesciption();
	auto attr_description = Vertex::getAttributeDescriptions();
	
	vertex_input_info.vertexBindingDescriptionCount = 1;
	vertex_input_info.pVertexBindingDescriptions = &binding_description;
	vertex_input_info.vertexAttributeDescriptionCount = attr_description.size();
	vertex_input_info.pVertexAttributeDescriptions = attr_description.data(); // Optional

	// input assembler
	VkPipelineInputAssemblyStateCreateInfo input_assembly_info = {};
	input_assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	input_assembly_info.primitiveRestartEnable = VK_FALSE;

	// viewport
	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)swap_chain_extent.width;
	viewport.height = (float)swap_chain_extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = swap_chain_extent;
	VkPipelineViewportStateCreateInfo viewport_state_info = {};
	viewport_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state_info.viewportCount = 1;
	viewport_state_info.pViewports = &viewport;
	viewport_state_info.scissorCount = 1;
	viewport_state_info.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f; // requires wideLines feature enabled when larger than one
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE; // what
	//rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	rasterizer.depthBiasClamp = 0.0f; // Optional
	rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

	// no multisampling
	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f; // Optional
	multisampling.pSampleMask = nullptr; /// Optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling.alphaToOneEnable = VK_FALSE; // Optional

	// no depth or stencil testing for now

	VkPipelineColorBlendAttachmentState color_blend_attachment = {};
	color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	// Use alpha blending
	color_blend_attachment.blendEnable = VK_TRUE;
	color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
	color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo color_blending_info = {};
	color_blending_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blending_info.logicOpEnable = VK_FALSE;
	color_blending_info.logicOp = VK_LOGIC_OP_COPY; // Optional
	color_blending_info.attachmentCount = 1;
	color_blending_info.pAttachments = &color_blend_attachment;
	color_blending_info.blendConstants[0] = 0.0f; // Optional
	color_blending_info.blendConstants[1] = 0.0f; // Optional
	color_blending_info.blendConstants[2] = 0.0f; // Optional
	color_blending_info.blendConstants[3] = 0.0f; // Optional

	// parameters allowed to be changed without recreating a pipeline
	VkDynamicState dynamicStates[] = 
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_LINE_WIDTH
	};
	VkPipelineDynamicStateCreateInfo dynamic_state_info = {};
	dynamic_state_info.dynamicStateCount = 2;
	dynamic_state_info.pDynamicStates = dynamicStates;

	// no uniform variables or push constants
	VkPipelineLayoutCreateInfo pipeline_layout_info = {};
	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.setLayoutCount = 0; // Optional
	pipeline_layout_info.pSetLayouts = nullptr; // Optional
	pipeline_layout_info.pushConstantRangeCount = 0; // Optional
	pipeline_layout_info.pPushConstantRanges = 0; // Optional
	auto pipeline_layout_result = vkCreatePipelineLayout(graphics_device, &pipeline_layout_info, nullptr,
		&pipeline_layout);
	if (pipeline_layout_result != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to create pipeline layout!");
	}

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;

	pipelineInfo.pVertexInputState = &vertex_input_info;
	pipelineInfo.pInputAssemblyState = &input_assembly_info;
	pipelineInfo.pViewportState = &viewport_state_info;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = nullptr; // Optional
	pipelineInfo.pColorBlendState = &color_blending_info;
	pipelineInfo.pDynamicState = nullptr; // Optional
	pipelineInfo.layout = pipeline_layout;
	pipelineInfo.renderPass = render_pass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // not deriving from existing pipeline
	pipelineInfo.basePipelineIndex = -1; // Optional

	auto pipeline_result = vkCreateGraphicsPipelines(graphics_device, VK_NULL_HANDLE, 1
		, &pipelineInfo, nullptr, &graphics_pipeline);

	if (pipeline_result != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}
}

void VulkanShowBase::createFrameBuffers()
{
	swap_chain_framebuffers.clear(); // VDeleter will delete old objects
	swap_chain_framebuffers.reserve(swap_chain_imageviews.size());

	for (size_t i = 0; i < swap_chain_imageviews.size(); i++)
	{
		swap_chain_framebuffers.push_back(VDeleter<VkFramebuffer>{ graphics_device, vkDestroyFramebuffer });

		VkImageView attachments[] = { swap_chain_imageviews[i] };

		VkFramebufferCreateInfo framebuffer_info = {};
		framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_info.renderPass = render_pass;
		framebuffer_info.attachmentCount = 1;
		framebuffer_info.pAttachments = attachments;
		framebuffer_info.width = swap_chain_extent.width;
		framebuffer_info.height = swap_chain_extent.height;
		framebuffer_info.layers = 1;

		auto result = vkCreateFramebuffer(graphics_device, &framebuffer_info, nullptr, &swap_chain_framebuffers[i]);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create framebuffer!");
		}
	}
}

void VulkanShowBase::createCommandPool()
{
	auto indices = QueueFamilyIndices::findQueueFamilies(physical_device, window_surface);

	VkCommandPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool_info.queueFamilyIndex = indices.graphicsFamily;
	pool_info.flags = 0; // Optional
	// hint the command pool will rerecord buffers by VK_COMMAND_POOL_CREATE_TRANSIENT_BIT
	// allow buffers to be rerecorded individually by VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT

	auto result = vkCreateCommandPool(graphics_device, &pool_info, nullptr, &command_pool);
	if (result != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to create command pool!");
	}
}

uint32_t findMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties, VkPhysicalDevice physical_device)
{
	VkPhysicalDeviceMemoryProperties memory_properties;
	vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);

	for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++)
	{
		bool type_supported = (type_filter & (1 << i)) != 0;
		bool properties_supported = ((memory_properties.memoryTypes[i].propertyFlags & properties) == properties);
		if (type_supported && properties_supported)
		{
			return i;
		}
	}

	throw std::runtime_error("Failed to find suitable memory type!");
}

void VulkanShowBase::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags property_bits
	, VkBuffer* p_buffer, VkDeviceMemory* p_buffer_memory)
{
	// create vertex buffer
	VkBufferCreateInfo buffer_info = {};
	buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_info.size = size;
	buffer_info.usage = usage;
	buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // vertex buffer only used in graphics queue
	buffer_info.flags = 0;

	auto buffer_result = vkCreateBuffer(graphics_device, &buffer_info, nullptr, p_buffer);

	if (buffer_result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create buffer!");
	}

	// allocate memory for buffer
	VkMemoryRequirements memory_req;
	vkGetBufferMemoryRequirements(graphics_device, *p_buffer, &memory_req);

	VkMemoryAllocateInfo memory_alloc_info = {};
	memory_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memory_alloc_info.allocationSize = memory_req.size;
	memory_alloc_info.memoryTypeIndex = findMemoryType(memory_req.memoryTypeBits
		, property_bits
		, physical_device);

	auto memory_result = vkAllocateMemory(graphics_device, &memory_alloc_info, nullptr, p_buffer_memory);
	if (memory_result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate buffer memory!");
	}

	// bind buffer with memory
	auto bind_result = vkBindBufferMemory(graphics_device, *p_buffer, *p_buffer_memory, 0);
	if (bind_result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to bind buffer memory!");
	}
}

void VulkanShowBase::copyBuffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size)
{	
	// create a temperorary command buffer for copy operation
	VkCommandBufferAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc_info.commandPool = command_pool;
	alloc_info.commandBufferCount = 1;

	VkCommandBuffer copy_command_buffer;
	vkAllocateCommandBuffers(graphics_device, &alloc_info, &copy_command_buffer);

	VkCommandBufferBeginInfo begin_info = {};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	VkBufferCopy copy_region = {};
	copy_region.srcOffset = 0; // Optional
	copy_region.dstOffset = 0; // Optional
	copy_region.size = size;

	vkBeginCommandBuffer(copy_command_buffer, &begin_info);
	vkCmdCopyBuffer(copy_command_buffer, src_buffer, dst_buffer, 1, &copy_region);
	vkEndCommandBuffer(copy_command_buffer);

	// execute the command buffer and wait for the execution
	VkSubmitInfo submit_info = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &copy_command_buffer;
	vkQueueSubmit(graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
	vkQueueWaitIdle(graphics_queue);

	// free the temperorary command buffer
	vkFreeCommandBuffers(graphics_device, command_pool, 1, &copy_command_buffer);

}

void VulkanShowBase::createVertexBuffer()
{
	VkDeviceSize buffer_size = sizeof(VERTICES[0]) * VERTICES.size();

	// create staging buffer
	VDeleter<VkBuffer> staging_buffer{ graphics_device, vkDestroyBuffer };
	VDeleter<VkDeviceMemory> staging_buffer_memory{ graphics_device, vkFreeMemory };
	createBuffer(buffer_size
		, VK_BUFFER_USAGE_TRANSFER_SRC_BIT // to be transfered from
		, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		, &staging_buffer
		, &staging_buffer_memory);

	// copy data to staging buffer
	void* data;
	vkMapMemory(graphics_device, staging_buffer_memory, 0, buffer_size, 0, &data); // access the graphics memory using mapping
		memcpy(data, VERTICES.data(), (size_t)buffer_size); // may not be immediate due to memory caching or write operation not visiable without VK_MEMORY_PROPERTY_HOST_COHERENT_BIT or explict flusing
	vkUnmapMemory(graphics_device, staging_buffer_memory);

	// create vertex buffer at optimized local memory which may not be directly accessable by memory mapping
	// as copy destination of staging buffer
	createBuffer(buffer_size
		, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
		, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		, &vertex_buffer
		, &vertex_buffer_memory);

	// copy content of staging buffer to vertex buffer
	copyBuffer(staging_buffer, vertex_buffer, buffer_size);
}

void VulkanShowBase::createIndexBuffer()
{
	VkDeviceSize buffer_size = sizeof(VERTEX_INDICES[0]) * VERTEX_INDICES.size();

	// create staging buffer
	VDeleter<VkBuffer> staging_buffer{ graphics_device, vkDestroyBuffer };
	VDeleter<VkDeviceMemory> staging_buffer_memory{ graphics_device, vkFreeMemory };
	createBuffer(buffer_size
		, VK_BUFFER_USAGE_TRANSFER_SRC_BIT // to be transfered from
		, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		, &staging_buffer
		, &staging_buffer_memory);

	void* data;
	vkMapMemory(graphics_device, staging_buffer_memory, 0, buffer_size, 0, &data); // access the graphics memory using mapping
	memcpy(data, VERTEX_INDICES.data(), (size_t)buffer_size); // may not be immediate due to memory caching or write operation not visiable without VK_MEMORY_PROPERTY_HOST_COHERENT_BIT or explict flusing
	vkUnmapMemory(graphics_device, staging_buffer_memory);

	createBuffer(buffer_size
		, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT
		, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		, &index_buffer
		, &index_buffer_memory);

	// copy content of staging buffer to index buffer
	copyBuffer(staging_buffer, index_buffer, buffer_size);
}

void VulkanShowBase::createCommandBuffers()
{
	// Free old command buffers, if any
	if (command_buffers.size() > 0)
	{
		vkFreeCommandBuffers(graphics_device, command_pool, (uint32_t)command_buffers.size(), command_buffers.data());
	}

	command_buffers.resize(swap_chain_framebuffers.size());

	VkCommandBufferAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.commandPool = command_pool;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; 
	// primary: can be submitted to a queue but cannot be called from other command buffers
	// secondary: can be called by others but cannot be submitted to a queue
	alloc_info.commandBufferCount = (uint32_t)command_buffers.size();

	auto alloc_result = vkAllocateCommandBuffers(graphics_device, &alloc_info, command_buffers.data());
	if (alloc_result != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate command buffers!");
	}

	// record command buffers
	for (size_t i = 0; i < command_buffers.size(); i++) 
	{
		VkCommandBufferBeginInfo begin_info = {};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		begin_info.pInheritanceInfo = nullptr; // Optional

		vkBeginCommandBuffer(command_buffers[i], &begin_info);

		VkRenderPassBeginInfo render_pass_info = {};
		render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		render_pass_info.renderPass = render_pass;
		render_pass_info.framebuffer = swap_chain_framebuffers[i];
		render_pass_info.renderArea.offset = { 0, 0 };
		render_pass_info.renderArea.extent = swap_chain_extent;
		VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
		render_pass_info.clearValueCount = 1;
		render_pass_info.pClearValues = &clearColor;

		vkCmdBeginRenderPass(command_buffers[i], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline);

		// bind vertex buffer
		VkBuffer vertex_buffers[] = { vertex_buffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(command_buffers[i], 0, 1, vertex_buffers, offsets);
		//vkCmdBindIndexBuffer(command_buffers[i], index_buffer, 0, VK_INDEX_TYPE_UINT16);
		vkCmdBindIndexBuffer(command_buffers[i], index_buffer, 0, VK_INDEX_TYPE_UINT32);

		// TODO: better to store vertex buffer and index buffer in a single VkBuffer

		//vkCmdDraw(command_buffers[i], VERTICES.size(), 1, 0, 0);
		vkCmdDrawIndexed(command_buffers[i], VERTEX_INDICES.size(), 1, 0, 0, 0);

		vkCmdEndRenderPass(command_buffers[i]);

		auto record_result = vkEndCommandBuffer(command_buffers[i]);
		if (record_result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to record command buffer!");
		}
	}
}

void VulkanShowBase::createSemaphores()
{
	VkSemaphoreCreateInfo semaphore_info = {};
	semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	auto result1 = vkCreateSemaphore(graphics_device, &semaphore_info, nullptr, &image_available_semaphore);
	auto result2 = vkCreateSemaphore(graphics_device, &semaphore_info, nullptr, &render_finished_semaphore);

	if (result1 != VK_SUCCESS || result2 != VK_SUCCESS) 
	{

		throw std::runtime_error("failed to create semaphores!");
	}
}

const uint64_t ACQUIRE_NEXT_IMAGE_TIMEOUT{ std::numeric_limits<uint64_t>::max() };
void VulkanShowBase::drawFrame()
{
	// 1. Acquiring an image from the swap chain
	uint32_t image_index;
	auto aquiring_result = vkAcquireNextImageKHR(graphics_device, swap_chain
		, ACQUIRE_NEXT_IMAGE_TIMEOUT, image_available_semaphore, VK_NULL_HANDLE, &image_index);

	if (aquiring_result == VK_ERROR_OUT_OF_DATE_KHR) 
	{
		// when swap chain needs recreation
		recreateSwapChain();
		return;
	}
	else if (aquiring_result != VK_SUCCESS && aquiring_result != VK_SUBOPTIMAL_KHR) 
	{
		throw std::runtime_error("Failed to acquire swap chain image!");
	}

	// 2. Submitting the command buffer
	VkSubmitInfo submit_info = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	VkSemaphore wait_semaphores[] = { image_available_semaphore }; // which semaphore to wait
	VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT }; // which stage to execute
	submit_info.waitSemaphoreCount = 1;
	submit_info.pWaitSemaphores = wait_semaphores;
	submit_info.pWaitDstStageMask = wait_stages;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &command_buffers[image_index];
	VkSemaphore signal_semaphores[] = { render_finished_semaphore };
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores = signal_semaphores;

	auto submit_result = vkQueueSubmit(graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
	if (submit_result != VK_SUCCESS) {
		throw std::runtime_error("Failed to submit draw command buffer!");
	}

	// 3. Submitting the result back to the swap chain to show it on screen
	VkPresentInfoKHR present_info = {};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = signal_semaphores;
	VkSwapchainKHR swapChains[] = { swap_chain };
	present_info.swapchainCount = 1;
	present_info.pSwapchains = swapChains;
	present_info.pImageIndices = &image_index;
	present_info.pResults = nullptr; // Optional, check for if every single chains is successful

	auto present_result = vkQueuePresentKHR(present_queue, &present_info);

	if (present_result == VK_ERROR_OUT_OF_DATE_KHR || present_result == VK_SUBOPTIMAL_KHR) 
	{
		recreateSwapChain();
	}
	else if (present_result != VK_SUCCESS) 
	{
		throw std::runtime_error("Failed to present swap chain image!");
	}
}

void VulkanShowBase::createShaderModule(const std::vector<char>& code, VkShaderModule* p_shader_module)
{
	VkShaderModuleCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	create_info.codeSize = code.size();
	create_info.pCode = (uint32_t*)code.data();

	auto result = vkCreateShaderModule(graphics_device, &create_info, nullptr, p_shader_module);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create shader module!");
	}
}

VkSurfaceFormatKHR VulkanShowBase::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& available_formats)
{
	// When free to choose format 
	if (available_formats.size() == 1 && available_formats[0].format == VK_FORMAT_UNDEFINED) 
	{
		return{ VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}

	for (const auto& available_format : available_formats)
	{
	    // prefer 32bits RGBA color with SRGB support
		if (available_format.format == VK_FORMAT_B8G8R8A8_UNORM && available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) 
		{
			return available_format;
		}
	}

	// TODO: Rank how good the formats are and choose the best?

	return available_formats[0];
}

VkPresentModeKHR VulkanShowBase::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& available_present_modes)
{
	for (const auto& available_present_mode : available_present_modes)
	{
		if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR) 
		{
			return available_present_mode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanShowBase::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	// The swap extent is the resolution of the swap chain images 
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) 
	{
		return capabilities.currentExtent;
	}
	else
	{
		VkExtent2D actual_extent = { (uint32_t)WINDOW_WIDTH, (uint32_t)WINDOW_HEIGHT };

		actual_extent.width = std::max(capabilities.minImageExtent.width
			, std::min(capabilities.maxImageExtent.width, actual_extent.width));
		actual_extent.height = std::max(capabilities.minImageExtent.height
			, std::min(capabilities.maxImageExtent.height, actual_extent.height));

		return actual_extent;
	}
}
