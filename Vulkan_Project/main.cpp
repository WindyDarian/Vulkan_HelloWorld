#include "VDeleter.h"

#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>

#include <iostream>
#include <functional>
#include <vector>
#include <unordered_set>
#include <string>

class HelloWorldApplication
{
public:
	void run() 
	{
		initWindow();
		initVulkan();
		mainLoop();
	}

	HelloWorldApplication() : instance(vkDestroyInstance) {}

private:
	GLFWwindow* window;
	VDeleter<VkInstance> instance;

	int window_width = 1920;
	int window_height = 1080;


	void initWindow()
	{
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // no OpenGL context
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		
		window = glfwCreateWindow(window_width, window_height, "Vulkan Hello World", nullptr, nullptr);
	}

	void createInstance()
	{
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
		unsigned int glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

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
		for (int i = 0; i < glfwExtensionCount; i++)
		{
			std::string name(glfwExtensions[i]);
			if (supportedExtensionNames.count(name) <= 0)
			{
				std::cout << "unsupported extension required by GLFW: " << name << std::endl;
			}
		}

		// Enable required extensions
		createInfo.enabledExtensionCount = glfwExtensionCount;
		createInfo.ppEnabledExtensionNames = glfwExtensions;
		createInfo.enabledLayerCount = 0;

		VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
		if (result != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to create instance!");
		}
		
		for (int i = 0; i < glfwExtensionCount; i++)
		{
			delete[] glfwExtensions[i];
		}
		delete glfwExtensions;
	}



	void initVulkan() 
	{
		createInstance();
	}

	void mainLoop() 
	{
		while (!glfwWindowShouldClose(window)) 
		{
			glfwPollEvents();
		}
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




//#define GLFW_INCLUDE_VULKAN
//#include <GLFW/glfw3.h>
//
//#define GLM_FORCE_RADIANS
//#define GLM_FORCE_DEPTH_ZERO_TO_ONE
//#include <glm/vec4.hpp>
//#include <glm/mat4x4.hpp>
//
//#include <iostream>
//
//int main() 
//{
//	glfwInit();
//
//	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
//	GLFWwindow* window = glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr);
//
//	uint32_t extensionCount = 0;
//	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
//
//	std::cout << extensionCount << " extensions supported" << std::endl;
//
//	glm::mat4 matrix;
//	glm::vec4 vec;
//	auto test = matrix * vec;
//
//	while (!glfwWindowShouldClose(window)) {
//		glfwPollEvents();
//	}
//
//	glfwDestroyWindow(window);
//
//	glfwTerminate();
//
//	return 0;
//}
