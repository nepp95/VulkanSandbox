#include "Application.h"

#include <algorithm>
#include <set>

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

Application* Application::s_instance = nullptr;

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


	// TODO: Surface?

	m_physicalDevice = std::make_shared<PhysicalDevice>();
	m_logicalDevice = std::make_shared<LogicalDevice>(m_physicalDevice);

	CreateSurface();
	CreateSwapchain();
	CreateImageViews();
	CreateRenderPass();
	CreateGraphicsPipeline();
	CreateFramebuffers();
	CreateCommandPool();
	CreateVertexBuffer();
	CreateIndexBuffer();
	CreateCommandBuffers();
	CreateSyncObjects();
}

void Application::Run()
{
	while (!glfwWindowShouldClose(m_window))
	{
		glfwPollEvents();
		DrawFrame();
	}

	vkDeviceWaitIdle(m_logicalDevice->GetNativeDevice());

	Shutdown();
}

void Application::Shutdown()
{
	CleanupSwapchain();

	vkDestroyBuffer(m_logicalDevice->GetNativeDevice(), m_indexBuffer, nullptr);
	vkFreeMemory(m_logicalDevice->GetNativeDevice(), m_indexBufferMemory, nullptr);

	vkDestroyBuffer(m_logicalDevice->GetNativeDevice(), m_vertexBuffer, nullptr);
	vkFreeMemory(m_logicalDevice->GetNativeDevice(), m_vertexBufferMemory, nullptr);

	vkDestroyPipeline(m_logicalDevice->GetNativeDevice(), m_graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(m_logicalDevice->GetNativeDevice(), m_pipelineLayout, nullptr);
	vkDestroyRenderPass(m_logicalDevice->GetNativeDevice(), m_renderPass, nullptr);

	for (uint32_t i = 0; i < VulkanConfig::MaxFramesInFlight; i++)
	{
		vkDestroySemaphore(m_logicalDevice->GetNativeDevice(), m_imageAvailableSemaphores[i], nullptr);
		vkDestroySemaphore(m_logicalDevice->GetNativeDevice(), m_renderFinishedSemaphores[i], nullptr);
		vkDestroyFence(m_logicalDevice->GetNativeDevice(), m_inFlightFences[i], nullptr);
	}

	vkDestroyCommandPool(m_logicalDevice->GetNativeDevice(), m_commandPool, nullptr);

	vkDestroyDevice(m_logicalDevice->GetNativeDevice(), nullptr);

	if (VulkanConfig::EnableValidation)
		DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);

	vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
	vkDestroyInstance(m_instance, nullptr);

	glfwDestroyWindow(m_window);
	glfwTerminate();
}

void Application::DrawFrame()
{
	LOG("Frame Begin");

	vkWaitForFences(m_logicalDevice->GetNativeDevice(), 1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);

	uint32_t imageIndex{ 0 };
	VkResult result = vkAcquireNextImageKHR(m_logicalDevice->GetNativeDevice(), m_swapchain, UINT64_MAX, m_imageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE, &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		RecreateSwapchain();
		LOG("Swapchain recreated. Ending this frame before draw");
		return; // Don't continue drawing this frame
	} // VK_SUBOPTIMAL_KHR can be used for rendering, but recreating is advised
	
	vkResetFences(m_logicalDevice->GetNativeDevice(), 1, &m_inFlightFences[m_currentFrame]);

	vkResetCommandBuffer(m_commandBuffers[m_currentFrame], 0);
	RecordCommandBuffer(m_commandBuffers[m_currentFrame], imageIndex);

	VkSemaphore waitSemaphores[]{ m_imageAvailableSemaphores[m_currentFrame] };
	VkSemaphore signalSemaphores[]{ m_renderFinishedSemaphores[m_currentFrame] };
	VkPipelineStageFlags waitStages[]{ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_commandBuffers[m_currentFrame];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	VK_CHECK(vkQueueSubmit(m_logicalDevice->GetGraphicsQueue(), 1, &submitInfo, m_inFlightFences[m_currentFrame]), "Failed to submit queue!");

	VkSwapchainKHR swapchains[]{ m_swapchain };

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapchains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;

	result = vkQueuePresentKHR(m_logicalDevice->GetGraphicsQueue(), &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_framebufferResized)
	{
		m_framebufferResized = false;
		RecreateSwapchain();
	}

	m_currentFrame = (m_currentFrame + 1) % VulkanConfig::MaxFramesInFlight;

	LOG("Frame End");
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

//bool Application::IsDeviceSuitable(VkPhysicalDevice device)
//{
//	// Device extensions
//	bool deviceExtensionsSupported = false;
//	uint32_t extensionCount{ 0 };
//	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
//	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
//	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());
//
//	std::set<std::string> requiredExtensions(VulkanConfig::DeviceExtensions.begin(), DEVICE_EXTENSIONS.end());
//	for (const auto& extension : availableExtensions)
//		requiredExtensions.erase(extension.extensionName);
//
//	if (!requiredExtensions.empty())
//		return false;
//
//	// Swapchain support -- This can only happen if we have the required extension
//	SwapchainSupportDetails swapchainSupport = QuerySwapchainSupport(device);
//	if (swapchainSupport.Formats.empty() || swapchainSupport.PresentModes.empty())
//		return false;
//	
//	return true;
//}

void Application::CreateSurface()
{
	VK_CHECK(glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface), "Failed to create surface!");
}

SwapchainSupportDetails Application::QuerySwapchainSupport(VkPhysicalDevice device)
{
	SwapchainSupportDetails details;

	// Capabilities
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &details.Capabilities);

	// Formats
	uint32_t formatCount{ 0 };
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, nullptr);

	if (formatCount != 0)
	{
		details.Formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, details.Formats.data());
	}

	// Presentation modes
	uint32_t presentModeCount{ 0 };
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, nullptr);

	if (presentModeCount != 0)
	{
		details.PresentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, details.PresentModes.data());
	}

	return details;
}

VkSurfaceFormatKHR Application::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	for (const auto& format : availableFormats)
	{
		if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			return format;
	}
	
	return availableFormats[0];
}

VkPresentModeKHR Application::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	for (const auto& presentMode : availablePresentModes)
	{
		if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			return presentMode;
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Application::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	if (capabilities.currentExtent.width != UINT32_MAX)
		return capabilities.currentExtent;
	
	int width{ 0 }, height{ 0 };
	glfwGetFramebufferSize(m_window, &width, &height);

	VkExtent2D actualExtent = { (uint32_t)width, (uint32_t)height };

	actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
	actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

	return actualExtent;
}

void Application::CreateSwapchain()
{
	SwapchainSupportDetails swapchainSupport = QuerySwapchainSupport(m_physicalDevice->GetNativeDevice());

	VkSurfaceFormatKHR format = ChooseSwapSurfaceFormat(swapchainSupport.Formats);
	VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapchainSupport.PresentModes);
	VkExtent2D extent = ChooseSwapExtent(swapchainSupport.Capabilities);

	uint32_t imageCount = swapchainSupport.Capabilities.minImageCount + 1;
	if (swapchainSupport.Capabilities.maxImageCount > 0 && imageCount > swapchainSupport.Capabilities.maxImageCount)
		imageCount = swapchainSupport.Capabilities.maxImageCount;

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = m_surface;
	createInfo.preTransform = swapchainSupport.Capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = format.format;
	createInfo.imageColorSpace = format.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = m_physicalDevice->GetQueueFamilyIndices();
	uint32_t queueFamilyIndices[] = { indices.Graphics };
	/*if (indices.GraphicsFamily != indices.PresentFamily)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	} else*/
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}

	VK_CHECK(vkCreateSwapchainKHR(m_logicalDevice->GetNativeDevice(), &createInfo, nullptr, &m_swapchain), "Failed to create swapchain!");

	vkGetSwapchainImagesKHR(m_logicalDevice->GetNativeDevice(), m_swapchain, &imageCount, nullptr);
	m_swapchainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(m_logicalDevice->GetNativeDevice(), m_swapchain, &imageCount, m_swapchainImages.data());

	m_swapchainExtent = extent;
	m_swapchainImageFormat = format.format;
}

void Application::RecreateSwapchain()
{
	int width{ 0 }, height{ 0 };
	glfwGetFramebufferSize(m_window, &width, &height);

	while (width == 0 || height == 0) // Window minimized
	{
		glfwGetFramebufferSize(m_window, &width, &height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(m_logicalDevice->GetNativeDevice());

	CleanupSwapchain();

	CreateSwapchain();
	CreateImageViews();
	CreateFramebuffers();
}

void Application::CleanupSwapchain()
{
	for (auto& framebuffer : m_swapchainFramebuffers)
		vkDestroyFramebuffer(m_logicalDevice->GetNativeDevice(), framebuffer, nullptr);

	for (auto& imageView : m_swapchainImageViews)
		vkDestroyImageView(m_logicalDevice->GetNativeDevice(), imageView, nullptr);

	vkDestroySwapchainKHR(m_logicalDevice->GetNativeDevice(), m_swapchain, nullptr);
}

void Application::CreateFramebuffers()
{
	m_swapchainFramebuffers.resize(m_swapchainImageViews.size());

	for (size_t i = 0; i < m_swapchainImageViews.size(); i++)
	{
		VkImageView attachments[] = {
			m_swapchainImageViews[i]
		};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = m_renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = m_swapchainExtent.width;
		framebufferInfo.height = m_swapchainExtent.height;
		framebufferInfo.layers = 1;

		VK_CHECK(vkCreateFramebuffer(m_logicalDevice->GetNativeDevice(), &framebufferInfo, nullptr, &m_swapchainFramebuffers[i]), "Failed to create framebuffer!");
	}
}

void Application::OnResize()
{
	m_framebufferResized = true;
}

void Application::CreateImageViews()
{
	m_swapchainImageViews.resize(m_swapchainImages.size());
	for (size_t i = 0; i < m_swapchainImages.size(); i++)
	{
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = m_swapchainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = m_swapchainImageFormat;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		VK_CHECK(vkCreateImageView(m_logicalDevice->GetNativeDevice(), &createInfo, nullptr, &m_swapchainImageViews[i]), "Failed to create image view!");
	}
}

void Application::CreateGraphicsPipeline()
{
	auto vertShader = ReadBytes("shaders/vert.spv");
	auto fragShader = ReadBytes("shaders/frag.spv");

	VkShaderModule vertShaderModule = CreateShaderModule(vertShader);
	VkShaderModule fragShaderModule = CreateShaderModule(fragShader);

	VkPipelineShaderStageCreateInfo vertShaderCreateInfo{};
	vertShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderCreateInfo.module = vertShaderModule;
	vertShaderCreateInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderCreateInfo{};
	fragShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderCreateInfo.module = fragShaderModule;
	fragShaderCreateInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStagers[] = { vertShaderCreateInfo, fragShaderCreateInfo };

	// Vertex input
	auto bindingDescription = Vertex::GetBindingDescription();
	auto attributeDescriptions = Vertex::GetAttributeDescriptions();

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = (uint32_t)attributeDescriptions.size();;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	// Input assembly
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
	inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)m_swapchainExtent.width;
	viewport.height = (float)m_swapchainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = m_swapchainExtent;

	VkPipelineViewportStateCreateInfo viewportStateInfo{};
	viewportStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportStateInfo.viewportCount = 1;
	viewportStateInfo.pViewports = &viewport;
	viewportStateInfo.scissorCount = 1;
	viewportStateInfo.pScissors = &scissor;

	// Rasterizer
	VkPipelineRasterizationStateCreateInfo rasterizerInfo{};
	rasterizerInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizerInfo.depthClampEnable = VK_FALSE;
	rasterizerInfo.rasterizerDiscardEnable = VK_FALSE;
	rasterizerInfo.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizerInfo.lineWidth = 1.0f;
	rasterizerInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizerInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizerInfo.depthBiasEnable = VK_FALSE;
	rasterizerInfo.depthBiasConstantFactor = 0.0f;
	rasterizerInfo.depthBiasClamp = 0.0f;
	rasterizerInfo.depthBiasSlopeFactor = 0.0f;

	// Multisampling
	VkPipelineMultisampleStateCreateInfo multisamplingInfo{};
	multisamplingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisamplingInfo.sampleShadingEnable = VK_FALSE;
	multisamplingInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisamplingInfo.minSampleShading = 1.0f;
	multisamplingInfo.pSampleMask = nullptr;
	multisamplingInfo.alphaToCoverageEnable = VK_FALSE;
	multisamplingInfo.alphaToOneEnable = VK_FALSE;

	// Color blending
	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlendInfo{};
	colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendInfo.logicOpEnable = VK_FALSE;
	colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;
	colorBlendInfo.attachmentCount = 1;
	colorBlendInfo.pAttachments = &colorBlendAttachment;
	colorBlendInfo.blendConstants[0] = 0.0f;
	colorBlendInfo.blendConstants[1] = 0.0f;
	colorBlendInfo.blendConstants[2] = 0.0f;
	colorBlendInfo.blendConstants[3] = 0.0f;

	std::vector<VkDynamicState> dynamicStates = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = (uint32_t)dynamicStates.size();
	dynamicState.pDynamicStates = dynamicStates.data();

	// Pipeline layout
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;
	pipelineLayoutInfo.pSetLayouts = nullptr;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;

	VK_CHECK(vkCreatePipelineLayout(m_logicalDevice->GetNativeDevice(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout), "Failed to create pipeline layout!");

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStagers;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
	pipelineInfo.pViewportState = &viewportStateInfo;
	pipelineInfo.pRasterizationState = &rasterizerInfo;
	pipelineInfo.pMultisampleState = &multisamplingInfo;
	pipelineInfo.pDepthStencilState = nullptr;
	pipelineInfo.pColorBlendState = &colorBlendInfo;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.layout = m_pipelineLayout;
	pipelineInfo.renderPass = m_renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;

	VK_CHECK(vkCreateGraphicsPipelines(m_logicalDevice->GetNativeDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_graphicsPipeline), "Failed to create graphics pipeline!");

	vkDestroyShaderModule(m_logicalDevice->GetNativeDevice(), vertShaderModule, nullptr);
	vkDestroyShaderModule(m_logicalDevice->GetNativeDevice(), fragShaderModule, nullptr);
}

VkShaderModule Application::CreateShaderModule(const std::vector<char>& byteCode)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = byteCode.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(byteCode.data());

	VkShaderModule shaderModule;
	VK_CHECK(vkCreateShaderModule(m_logicalDevice->GetNativeDevice(), &createInfo, nullptr, &shaderModule), "Failed to create shader module!");

	return shaderModule;
}

void Application::CreateRenderPass()
{
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = m_swapchainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpassDescription{};
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pColorAttachments = &colorAttachmentRef;

	VkSubpassDependency subpassDependency{};
	subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependency.dstSubpass = 0;
	subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependency.srcAccessMask = 0;
	subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpassDescription;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &subpassDependency;

	VK_CHECK(vkCreateRenderPass(m_logicalDevice->GetNativeDevice(), &renderPassInfo, nullptr, &m_renderPass), "Failed to create render pass!");
}

void Application::CreateCommandPool()
{
	QueueFamilyIndices indices = m_physicalDevice->GetQueueFamilyIndices();

	VkCommandPoolCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	createInfo.queueFamilyIndex = indices.Graphics;

	VK_CHECK(vkCreateCommandPool(m_logicalDevice->GetNativeDevice(), &createInfo, nullptr, &m_commandPool), "Failed to create command pool!");
}

void Application::CreateCommandBuffers()
{
	m_commandBuffers.resize(VulkanConfig::MaxFramesInFlight);

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = m_commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)m_commandBuffers.size();

	VK_CHECK(vkAllocateCommandBuffers(m_logicalDevice->GetNativeDevice(), &allocInfo, m_commandBuffers.data()), "Failed to create command buffer!");
}

void Application::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0;
	beginInfo.pInheritanceInfo = nullptr;

	VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo), "Failed to begin command buffer!");

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = m_renderPass;
	renderPassInfo.framebuffer = m_swapchainFramebuffers[imageIndex];
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = m_swapchainExtent;
	
	VkClearValue clearColor = {{{ 0.0f, 0.0f, 0.0f, 1.0f }}}; // TODO: This is weird.
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)m_swapchainExtent.width;
	viewport.height = (float)m_swapchainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = m_swapchainExtent;
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	VkBuffer vertexBuffers[]{ m_vertexBuffer };
	VkDeviceSize offsets[]{ 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
	vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);

	vkCmdDrawIndexed(commandBuffer, (uint32_t)indices.size(), 1, 0, 0, 0);

	vkCmdEndRenderPass(commandBuffer);

	VK_CHECK(vkEndCommandBuffer(commandBuffer), "Failed to record command buffer!");
}

void Application::CreateSyncObjects()
{
	m_imageAvailableSemaphores.resize(VulkanConfig::MaxFramesInFlight);
	m_renderFinishedSemaphores.resize(VulkanConfig::MaxFramesInFlight);
	m_inFlightFences.resize(VulkanConfig::MaxFramesInFlight);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (uint32_t i = 0; i < VulkanConfig::MaxFramesInFlight; i++)
	{
		VK_CHECK(vkCreateSemaphore(m_logicalDevice->GetNativeDevice(), &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]), "Failed to create semaphore!");
		VK_CHECK(vkCreateSemaphore(m_logicalDevice->GetNativeDevice(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]), "Failed to create semaphore!");
		VK_CHECK(vkCreateFence(m_logicalDevice->GetNativeDevice(), &fenceInfo, nullptr, &m_inFlightFences[i]), "Failed to create fence!");
	}
}

void Application::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags propertyFlags, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usageFlags;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VK_CHECK(vkCreateBuffer(m_logicalDevice->GetNativeDevice(), &bufferInfo, nullptr, &buffer), "Failed to create buffer!");

	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(m_logicalDevice->GetNativeDevice(), buffer, &memoryRequirements);

	VkMemoryAllocateInfo allocateInfo{};
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize = memoryRequirements.size;
	allocateInfo.memoryTypeIndex = FindMemoryType(memoryRequirements.memoryTypeBits, propertyFlags);

	VK_CHECK(vkAllocateMemory(m_logicalDevice->GetNativeDevice(), &allocateInfo, nullptr, &bufferMemory), "Failed to allocate vertex buffer memory!");

	vkBindBufferMemory(m_logicalDevice->GetNativeDevice(), buffer, bufferMemory, 0);
}

void Application::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkCommandBufferAllocateInfo allocateInfo{};
	allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocateInfo.commandPool = m_commandPool;
	allocateInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(m_logicalDevice->GetNativeDevice(), &allocateInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = size;

	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(m_logicalDevice->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(m_logicalDevice->GetGraphicsQueue());

	vkFreeCommandBuffers(m_logicalDevice->GetNativeDevice(), m_commandPool, 1, &commandBuffer);
}

void Application::CreateVertexBuffer()
{
	VkDeviceSize bufferSize = sizeof(Vertex) * vertices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(m_logicalDevice->GetNativeDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vertices.data(), (size_t)bufferSize);
	vkUnmapMemory(m_logicalDevice->GetNativeDevice(), stagingBufferMemory);

	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_vertexBuffer, m_vertexBufferMemory);
	CopyBuffer(stagingBuffer, m_vertexBuffer, bufferSize);

	vkDestroyBuffer(m_logicalDevice->GetNativeDevice(), stagingBuffer, nullptr);
	vkFreeMemory(m_logicalDevice->GetNativeDevice(), stagingBufferMemory, nullptr);
}

void Application::CreateIndexBuffer()
{
	VkDeviceSize bufferSize = sizeof(uint32_t) * indices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
	
	void* data;
	vkMapMemory(m_logicalDevice->GetNativeDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, indices.data(), (size_t)bufferSize);
	vkUnmapMemory(m_logicalDevice->GetNativeDevice(), stagingBufferMemory);

	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_indexBuffer, m_indexBufferMemory);
	CopyBuffer(stagingBuffer, m_indexBuffer, bufferSize);

	vkDestroyBuffer(m_logicalDevice->GetNativeDevice(), stagingBuffer, nullptr);
	vkFreeMemory(m_logicalDevice->GetNativeDevice(), stagingBufferMemory, nullptr);
}

uint32_t Application::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties(m_physicalDevice->GetNativeDevice(), &memoryProperties);

	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
		if (typeFilter & (1 << i) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
			return i;

	LOG("Failed to find suitable memory type!");
	__debugbreak;

	return 0;
}
