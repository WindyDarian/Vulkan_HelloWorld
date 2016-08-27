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
	glm::vec2 tex_coord;

	static VkVertexInputBindingDescription getBindingDesciption()
	{
		VkVertexInputBindingDescription binding_description = {};
		binding_description.binding = 0; // index of the binding, defined in vertex shader
		binding_description.stride = sizeof(Vertex);
		binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // move to next data engty after each vertex
		return binding_description;
	}

	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions()
	{
		std::array<VkVertexInputAttributeDescription, 3> attr_descriptions = {};
		attr_descriptions[0].binding = 0; 
		attr_descriptions[0].location = 0;
		attr_descriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attr_descriptions[0].offset = offsetof(Vertex, pos); //bytes of a member since beginning of struct
		attr_descriptions[1].binding = 0;
		attr_descriptions[1].location = 1;
		attr_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attr_descriptions[1].offset = offsetof(Vertex, color); //bytes of a member since beginning of struct
		attr_descriptions[2].binding = 0;
		attr_descriptions[2].location = 2;
		attr_descriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attr_descriptions[2].offset = offsetof(Vertex, tex_coord);

		return attr_descriptions;
	}
};

struct UniformBufferObject
{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
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
	VDeleter<VkRenderPass> render_pass{ graphics_device, vkDestroyRenderPass };

	VDeleter<VkDescriptorSetLayout> descriptor_set_layout{ graphics_device, vkDestroyDescriptorSetLayout };
	VDeleter<VkPipelineLayout> pipeline_layout{ graphics_device, vkDestroyPipelineLayout };
	VDeleter<VkPipeline> graphics_pipeline{ graphics_device, vkDestroyPipeline };

	// Command buffers
	VDeleter<VkCommandPool> command_pool{ graphics_device, vkDestroyCommandPool };
	std::vector<VkCommandBuffer> command_buffers; // buffers will be released when pool destroyed

	VDeleter<VkSemaphore> image_available_semaphore{ graphics_device, vkDestroySemaphore };
	VDeleter<VkSemaphore> render_finished_semaphore{ graphics_device, vkDestroySemaphore };

	// texture image
	VDeleter<VkImage> texture_image{ graphics_device, vkDestroyImage };
	VDeleter<VkDeviceMemory> texture_image_memory{ graphics_device, vkFreeMemory };
	VDeleter<VkImageView> texture_image_view{ graphics_device, vkDestroyImageView };
	VDeleter<VkSampler> texture_sampler{ graphics_device, vkDestroySampler };


	// vertex buffer
	VDeleter<VkBuffer> vertex_buffer{ graphics_device, vkDestroyBuffer };
	VDeleter<VkDeviceMemory> vertex_buffer_memory{ graphics_device, vkFreeMemory };
	VDeleter<VkBuffer> index_buffer{ graphics_device, vkDestroyBuffer };
	VDeleter<VkDeviceMemory> index_buffer_memory{ graphics_device, vkFreeMemory };

	// uniform buffer and descriptor
	VDeleter<VkBuffer> uniform_staging_buffer{ graphics_device, vkDestroyBuffer };
	VDeleter<VkDeviceMemory> uniform_staging_buffer_memory{ graphics_device, vkFreeMemory };
	VDeleter<VkBuffer> uniform_buffer{ graphics_device, vkDestroyBuffer };
	VDeleter<VkDeviceMemory> uniform_buffer_memory{ graphics_device, vkFreeMemory };

	VDeleter<VkDescriptorPool> descriptor_pool{ graphics_device, vkDestroyDescriptorPool };
	VkDescriptorSet descriptor_set;


	const int WINDOW_WIDTH = 1920;
	const int WINDOW_HEIGHT = 1080;
	const bool WINDOW_RESIZABLE = true;

	const std::vector<Vertex> VERTICES = {
		{ { -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f }, {0.0f, 0.0f} },
		{ { 0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f }, {0.0f, 1.0f} },
		{ { 0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f }, {1.0f, 1.0f} },
		{ { -0.5f, 0.5f }, { 1.0f, 1.0f, 1.0f }, {1.0f, 0.0f} }
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
	void createDescriptorSetLayout();
	void createGraphicsPipeline();
	void createFrameBuffers();
	void createCommandPool();
	void createTextureImage();
	void createTextureImageView();
	void createTextureSampler();
	void createVertexBuffer();
	void createIndexBuffer();
	void createUniformBuffer();
	void createDescriptorPool();
	void createDescriptorSet();
	void createCommandBuffers();
	void createSemaphores();

	void updateUniformBuffer();
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

	void createImage(uint32_t image_width, uint32_t image_height
		, VkFormat format, VkImageTiling tiling
		, VkImageUsageFlags usage, VkMemoryPropertyFlags memory_properties
		, VkImage* p_vkimage, VkDeviceMemory* p_image_memory);
	void copyImage(VkImage src_image, VkImage dst_image, uint32_t width, uint32_t height);
	void transitionImageLayout(VkImage image, VkImageLayout old_layout, VkImageLayout new_layout);

	void createImageView(VkImage image, VkFormat format, VkImageView* p_image_view);

	VkCommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);
};

