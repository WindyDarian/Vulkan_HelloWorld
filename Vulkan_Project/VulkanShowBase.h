#pragma once

#include "VDeleter.h"

#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>
#include <glm/glm.hpp>

#include <vector>
#include <array>

struct Vertex
{
	glm::vec2 pos;
	glm::vec3 color;

	static VkVertexInputBindingDescription getBindingDesciption()
	{
		VkVertexInputBindingDescription binding_description = {};
		binding_description.binding = 0; // index of the binding, defined in vertex shader
		binding_description.stride = sizeof(Vertex);
		binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // move to next data engty after each vertex
		return binding_description;
	}

	static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions()
	{
		std::array<VkVertexInputAttributeDescription, 2> attr_descriptions = {};
		attr_descriptions[0].binding = 0; 
		attr_descriptions[0].location = 0;
		attr_descriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attr_descriptions[0].offset = offsetof(Vertex, pos); //bytes of a member since beginning of struct
		attr_descriptions[1].binding = 0;
		attr_descriptions[1].location = 1;
		attr_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attr_descriptions[1].offset = offsetof(Vertex, color); //bytes of a member since beginning of struct

		return attr_descriptions;
	}
};

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

	static void onWindowResized(GLFWwindow* window, int width, int height);

	static void DestroyDebugReportCallbackEXT(VkInstance instance
		, VkDebugReportCallbackEXT callback
		, const VkAllocationCallbacks* pAllocator);

	static VkResult CreateDebugReportCallbackEXT(VkInstance instance
		, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo
		, const VkAllocationCallbacks* pAllocator
		, VkDebugReportCallbackEXT* pCallback);

private:
	GLFWwindow* window;

	VDeleter<VkInstance> instance{ vkDestroyInstance };
	VDeleter<VkDebugReportCallbackEXT> callback{ instance, DestroyDebugReportCallbackEXT };
	VkPhysicalDevice physical_device;

	VDeleter<VkDevice> graphics_device{ vkDestroyDevice }; //logical device
	VkQueue graphics_queue;

	VDeleter<VkSurfaceKHR> window_surface{ instance, vkDestroySurfaceKHR };
	VkQueue present_queue;

	VDeleter<VkSwapchainKHR> swap_chain{ graphics_device, vkDestroySwapchainKHR };
	std::vector<VkImage> swap_chain_images;
	VkFormat swap_chain_image_format;
	VkExtent2D swap_chain_extent;
	std::vector<VDeleter<VkImageView>> swap_chain_imageviews;
	std::vector<VDeleter<VkFramebuffer>> swap_chain_framebuffers;

	VDeleter<VkShaderModule> vert_shader_module{ graphics_device, vkDestroyShaderModule };
	VDeleter<VkShaderModule> frag_shader_module{ graphics_device, vkDestroyShaderModule };
	VDeleter<VkRenderPass> render_pass{ graphics_device, vkDestroyRenderPass };
	VDeleter<VkPipelineLayout> pipeline_layout{ graphics_device, vkDestroyPipelineLayout };
	VDeleter<VkPipeline> graphics_pipeline{ graphics_device, vkDestroyPipeline };

	// Command buffers
	VDeleter<VkCommandPool> command_pool{ graphics_device, vkDestroyCommandPool };
	std::vector<VkCommandBuffer> command_buffers; // buffers will be released when pool destroyed

	VDeleter<VkSemaphore> image_available_semaphore{ graphics_device, vkDestroySemaphore };
	VDeleter<VkSemaphore> render_finished_semaphore{ graphics_device, vkDestroySemaphore };

	VDeleter<VkBuffer> vertex_buffer{ graphics_device, vkDestroyBuffer };
	VDeleter<VkDeviceMemory> vertex_buffer_memory{ graphics_device, vkFreeMemory };
	VDeleter<VkBuffer> index_buffer{ graphics_device, vkDestroyBuffer };
	VDeleter<VkDeviceMemory> index_buffer_memory{ graphics_device, vkFreeMemory };

	const int WINDOW_WIDTH = 1920;
	const int WINDOW_HEIGHT = 1080;
	const bool WINDOW_RESIZABLE = true;

	const std::vector<Vertex> VERTICES = {
		{ { -0.5f, -0.5f },{ 1.0f, 0.0f, 0.0f } },
		{ { 0.5f, -0.5f },{ 0.0f, 1.0f, 0.0f } },
		{ { 0.5f, 0.5f },{ 0.0f, 0.0f, 1.0f } },
		{ { -0.5f, 0.5f },{ 1.0f, 1.0f, 1.0f } }
	};
	const std::vector<uint32_t> VERTEX_INDICES = {
		0, 1, 2, 2, 3, 0
	};

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
	void mainLoop();

	void recreateSwapChain();

	void createInstance();
	void setupDebugCallback();
	void createWindowSurface();
	void pickPhysicalDevice();
	void createLogicalDevice();
	void createSwapChain();
	void createImageViews();
	void createRenderPass();
	void createGraphicsPipeline();
	void createFrameBuffers();
	void createCommandPool();
	void createVertexBuffer();
	void createIndexBuffer();
	void createCommandBuffers();
	void createSemaphores();

	void drawFrame();

	void createShaderModule(const std::vector<char>& code, VkShaderModule* p_shader_module);
	
	bool checkValidationLayerSupport();
	std::vector<const char*> getRequiredExtensions();
	bool isDeviceSuitable(VkPhysicalDevice device);
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& available_formats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& available_present_modes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags property_bits
		, VkBuffer* p_buffer, VkDeviceMemory* p_buffer_memory);
	void copyBuffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size);
};

