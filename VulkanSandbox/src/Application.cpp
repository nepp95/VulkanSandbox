#include "Application.h"

#include "Allocator.h"
#include "Vertex.h"

#include <set>
#include <sstream>

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr)
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		
	return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr)
		func(instance, debugMessenger, pAllocator);
}

// Debug callback
static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	/*
		MessageSeverity:
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: Diagnostic message
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: Informational message like the creation of a resource
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: Message about behavior that is not necessarily an error, but very likely a bug in your application
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: Message about behavior that is invalid and may cause crashes

		MessageType:
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT: Some event has happened that is unrelated to the specification or performance
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT: Something has happened that violates the specification or indicates a possible mistake
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT: Potential non-optimal use of Vulkan
	*/
	std::stringstream ss;
	ss << "Validation layer: " << pCallbackData->pMessage;
	LOG(ss.str());

	return VK_FALSE;
}

Application* Application::s_instance = nullptr;

const std::vector<Vertex> vertices{
	{ { -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f } },
	{ {  0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f } },
	{ {  0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f } },
	{ { -0.5f,  0.5f }, { 1.0f, 1.0f, 1.0f } }
};

const std::vector<uint32_t> indices{
	0, 1, 2, 2, 3, 0
};

Application::Application()
{
	s_instance = this;

	LOG("Starting VulkanSandbox");

	int status = glfwInit();
	if (status != GLFW_TRUE)
		LOG("glfwInit() failed!");

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	m_window = glfwCreateWindow(VulkanConfig::Width, VulkanConfig::Height, "VulkanSandbox", nullptr, nullptr);

	// FRAMEBUFFER RESIZE CALLBACK HERE

	if (VulkanConfig::EnableValidation && !HasValidationLayerSupport())
		throw std::runtime_error("Validation layers requested but not available!");

	// Instance
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "VulkanSandbox";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_3;

	auto extensions = GetRequiredExtensions();

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledExtensionCount = (uint32_t)extensions.size();
	createInfo.ppEnabledExtensionNames = extensions.data();

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	if (VulkanConfig::EnableValidation)
	{
		debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		debugCreateInfo.pfnUserCallback = DebugCallback;
		debugCreateInfo.pUserData = nullptr;

		createInfo.enabledLayerCount = (uint32_t)VulkanConfig::ValidationLayers.size();
		createInfo.ppEnabledLayerNames = VulkanConfig::ValidationLayers.data();
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	} else
	{
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	}

	VK_CHECK(vkCreateInstance(&createInfo, nullptr, &m_instance), "Failed to create instance!");

	if (VulkanConfig::EnableValidation)
		VK_CHECK(CreateDebugUtilsMessengerEXT(m_instance, &debugCreateInfo, nullptr, &m_debugMessenger), "Failed to create debug messenger!");

	m_physicalDevice = std::make_shared<PhysicalDevice>();
	m_logicalDevice = std::make_shared<LogicalDevice>(m_physicalDevice);

	Allocator::Init();

	m_swapchain = std::make_shared<Swapchain>(m_logicalDevice);
	m_pipeline = std::make_shared<Pipeline>(m_logicalDevice);
	
	uint32_t vertexBufferSize = sizeof(Vertex) * vertices.size();
	m_vertexBuffer = std::make_shared<VertexBuffer>((void*)vertices.data(), vertexBufferSize);

	uint32_t indexBufferSize = sizeof(uint32_t) * indices.size();
	m_indexBuffer = std::make_shared<IndexBuffer>((void*)indices.data(), indexBufferSize);

	m_uniformBuffer = std::make_shared<UniformBuffer>(m_logicalDevice);
}

void Application::Run()
{
	while (!glfwWindowShouldClose(m_window))
	{
		glfwPollEvents();

		m_swapchain->BeginFrame();
		BeginFrame();
		m_swapchain->Present();
	}

	vkDeviceWaitIdle(m_logicalDevice->GetNativeDevice());

	Shutdown();
}

void Application::Shutdown()
{
	m_swapchain->Cleanup();
	m_pipeline->Destroy();
	m_swapchain->Destroy();
	m_logicalDevice->Destroy();

	if (VulkanConfig::EnableValidation)
		DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);

	m_swapchain->DestroySurface();
	vkDestroyInstance(m_instance, nullptr);

	glfwDestroyWindow(m_window);
	glfwTerminate();
}

void Application::BeginFrame()
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0;
	beginInfo.pInheritanceInfo = nullptr;

	VK_CHECK(vkBeginCommandBuffer(m_swapchain->GetRenderCommandBuffer(), &beginInfo), "Failed to begin command buffer!");

	VkExtent2D extent = m_swapchain->GetExtent();

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = m_swapchain->GetRenderPass();
	renderPassInfo.framebuffer = m_swapchain->GetCurrentFramebuffer();
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = extent;

	VkClearValue clearColor = { {{ 0.0f, 0.0f, 0.0f, 1.0f }} }; // TODO: This is weird.
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	vkCmdBeginRenderPass(m_swapchain->GetRenderCommandBuffer(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(m_swapchain->GetRenderCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline->GetPipeline());

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)extent.width;
	viewport.height = (float)extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(m_swapchain->GetRenderCommandBuffer(), 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = extent;
	vkCmdSetScissor(m_swapchain->GetRenderCommandBuffer(), 0, 1, &scissor);

	VkBuffer vbo[]{ m_vertexBuffer->GetBuffer() };
	VkDeviceSize offsets[]{ 0 };
	vkCmdBindVertexBuffers(m_swapchain->GetRenderCommandBuffer(), 0, 1, vbo, offsets);
	vkCmdBindIndexBuffer(m_swapchain->GetRenderCommandBuffer(), m_indexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);

	vkCmdDrawIndexed(m_swapchain->GetRenderCommandBuffer(), (uint32_t)indices.size(), 1, 0, 0, 0);

	vkCmdEndRenderPass(m_swapchain->GetRenderCommandBuffer());

	VK_CHECK(vkEndCommandBuffer(m_swapchain->GetRenderCommandBuffer()), "Failed to record command buffer!");
}

bool Application::HasValidationLayerSupport()
{
	uint32_t layerCount{ 0 };
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : VulkanConfig::ValidationLayers)
	{
		bool layerFound{ false };

		for (const auto& layerProperties : availableLayers)
		{
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}

		if (!layerFound)
			return false;
	}

	return true;
}

std::vector<const char*> Application::GetRequiredExtensions()
{
	uint32_t glfwExtensionCount{ 0 };
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (VulkanConfig::EnableValidation)
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	return extensions;
}
