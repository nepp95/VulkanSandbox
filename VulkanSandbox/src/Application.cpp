#include "Application.h"

#include "Memory/Allocator.h"
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
	{ { -0.5f, -0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f } },
	{ {  0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } },
	{ {  0.5f,  0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } },
	{ { -0.5f,  0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f } },

	{ { -0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f } },
	{ {  0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } },
	{ {  0.5f,  0.5f, -0.5f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } },
	{ { -0.5f,  0.5f, -0.5f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f } }
};

const std::vector<uint32_t> indices{
	0, 1, 2, 2, 3, 0,
	4, 5, 6, 6, 7, 4
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
	
	// Buffers
	uint32_t vertexBufferSize = sizeof(Vertex) * vertices.size();
	m_vertexBuffer = std::make_shared<VertexBuffer>((void*)vertices.data(), vertexBufferSize);

	uint32_t indexBufferSize = sizeof(uint32_t) * indices.size();
	m_indexBuffer = std::make_shared<IndexBuffer>((void*)indices.data(), indexBufferSize);

	m_uniformBuffer = std::make_shared<UniformBuffer>(m_logicalDevice);

	// Load a texture
	m_image = std::make_shared<Image>("textures/texture.jpg");

	// Sampler
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = m_physicalDevice->GetDeviceProperties().limits.maxSamplerAnisotropy;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;

	VK_CHECK(vkCreateSampler(m_logicalDevice->GetNativeDevice(), &samplerInfo, nullptr, &m_sampler), "Failed to create sampler!");

	// Descriptor pool
	VkDescriptorPoolSize poolSizeUbo{};
	poolSizeUbo.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizeUbo.descriptorCount = VulkanConfig::MaxFramesInFlight;

	VkDescriptorPoolSize poolSizeSampler{};
	poolSizeSampler.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizeSampler.descriptorCount = VulkanConfig::MaxFramesInFlight;

	std::array<VkDescriptorPoolSize, 2> poolSizes = { poolSizeUbo, poolSizeSampler };
	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = (uint32_t)poolSizes.size();
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = VulkanConfig::MaxFramesInFlight;

	VK_CHECK(vkCreateDescriptorPool(m_logicalDevice->GetNativeDevice(), &poolInfo, nullptr, &m_descriptorPool), "Failed to create descriptor pool!");

	// Descriptor sets
	std::vector<VkDescriptorSetLayout> layouts(VulkanConfig::MaxFramesInFlight, m_pipeline->GetDescriptorLayout());
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = m_descriptorPool;
	allocInfo.descriptorSetCount = (uint32_t)layouts.size();
	allocInfo.pSetLayouts = layouts.data();

	m_descriptorSets.resize(VulkanConfig::MaxFramesInFlight);
	VK_CHECK(vkAllocateDescriptorSets(m_logicalDevice->GetNativeDevice(), &allocInfo, m_descriptorSets.data()), "Failed to allocate descriptor sets!");

	for (uint32_t i = 0; i < VulkanConfig::MaxFramesInFlight; i++)
	{
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = m_uniformBuffer->GetBuffers()[i];
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObject);

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = m_image->GetImageView(); // TODO: MOVE THIS!!!
		imageInfo.sampler = m_sampler;

		VkWriteDescriptorSet writeDescriptorUbo{};
		writeDescriptorUbo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorUbo.dstSet = m_descriptorSets[i];
		writeDescriptorUbo.dstBinding = 0;
		writeDescriptorUbo.dstArrayElement = 0;
		writeDescriptorUbo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptorUbo.descriptorCount = 1;
		writeDescriptorUbo.pBufferInfo = &bufferInfo;
		writeDescriptorUbo.pImageInfo = nullptr;
		writeDescriptorUbo.pTexelBufferView = nullptr;

		VkWriteDescriptorSet writeDescriptorImage{};
		writeDescriptorImage.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorImage.dstSet = m_descriptorSets[i];
		writeDescriptorImage.dstBinding = 1;
		writeDescriptorImage.dstArrayElement = 0;
		writeDescriptorImage.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeDescriptorImage.descriptorCount = 1;
		writeDescriptorImage.pBufferInfo = nullptr;
		writeDescriptorImage.pImageInfo = &imageInfo;
		writeDescriptorImage.pTexelBufferView = nullptr;

		std::array<VkWriteDescriptorSet, 2> writeDescriptors = { writeDescriptorUbo, writeDescriptorImage };
		vkUpdateDescriptorSets(m_logicalDevice->GetNativeDevice(), (uint32_t)writeDescriptors.size(), writeDescriptors.data(), 0, nullptr);
	}
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
	VkDevice device = m_logicalDevice->GetNativeDevice();

	vkDestroySampler(device, m_sampler, nullptr);
	m_swapchain->Cleanup();
	vkDestroyDescriptorPool(device, m_descriptorPool, nullptr);
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

	vkCmdBindDescriptorSets(m_swapchain->GetRenderCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline->GetPipelineLayout(), 0, 1, &m_descriptorSets[m_swapchain->GetCurrentImageIndex()], 0, nullptr);
	
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
