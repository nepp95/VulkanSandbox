#include "Swapchain.h"

#include "Application.h"

#include <algorithm>

Swapchain::Swapchain(const std::shared_ptr<LogicalDevice>& device)
	: m_logicalDevice(device)
{
	Create();

	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = m_format;
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

void Swapchain::Create()
{
	auto app = Application::Get();
	VkInstance instance = app.GetInstance();
	VkDevice logicalDevice = m_logicalDevice->GetNativeDevice();

	// Window surface
	VK_CHECK(glfwCreateWindowSurface(instance, app.GetWindow(), nullptr, &m_surface), "Failed to create window surface!");
	
	// Swapchain
	SwapchainSupportDetails details = QuerySwapchainSupport();

	VkSurfaceFormatKHR format = ChooseSwapSurfaceFormat(details.Formats);
	VkPresentModeKHR presentMode = ChooseSwapPresentMode(details.PresentModes);
	VkExtent2D extent = ChooseSwapExtent(details.Capabilities);

	uint32_t imageCount = details.Capabilities.minImageCount + 1;
	if (details.Capabilities.maxImageCount > 0 && imageCount > details.Capabilities.maxImageCount)
		imageCount = details.Capabilities.maxImageCount;

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = m_surface;
	createInfo.preTransform = details.Capabilities.currentTransform;
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

	QueueFamilyIndices indices = m_logicalDevice->GetPhysicalDevice()->GetQueueFamilyIndices();
	uint32_t queueFamilyIndices[] = { indices.Graphics };

	createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.queueFamilyIndexCount = 0;
	createInfo.pQueueFamilyIndices = nullptr;

	VK_CHECK(vkCreateSwapchainKHR(logicalDevice, &createInfo, nullptr, &m_swapchain), "Failed to create swapchain!");

	// Images
	vkGetSwapchainImagesKHR(logicalDevice, m_swapchain, &imageCount, nullptr);
	m_images.resize(imageCount);
	vkGetSwapchainImagesKHR(logicalDevice, m_swapchain, &imageCount, m_images.data());

	m_width = extent.width;
	m_height = extent.height;
	m_extent = extent;
	m_format = format.format;

	// Image views
	m_imageViews.resize(m_images.size());

	for (size_t i = 0; i < m_images.size(); i++)
	{
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = m_images[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = m_format;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		VK_CHECK(vkCreateImageView(logicalDevice, &createInfo, nullptr, &m_imageViews[i]), "Failed to create image view!");
	}

	// Framebuffers
	m_framebuffers.resize(m_imageViews.size());
	
	for (size_t i = 0; i < m_imageViews.size(); i++)
	{
		VkImageView attachments[] = {
			m_imageViews[i]
		};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = m_renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = m_extent.width;
		framebufferInfo.height = m_extent.height;
		framebufferInfo.layers = 1;

		VK_CHECK(vkCreateFramebuffer(logicalDevice, &framebufferInfo, nullptr, &m_framebuffers[i]), "Failed to create framebuffer!");
	}

	// Command pool
	VkCommandPoolCreateInfo commandPoolInfo{};
	commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	commandPoolInfo.queueFamilyIndex = indices.Graphics;

	VK_CHECK(vkCreateCommandPool(logicalDevice, &commandPoolInfo, nullptr, &m_commandPool), "Failed to create command pool!");

	// Command buffers
	m_commandBuffers.resize(VulkanConfig::MaxFramesInFlight);

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = m_commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)m_commandBuffers.size();

	VK_CHECK(vkAllocateCommandBuffers(logicalDevice, &allocInfo, m_commandBuffers.data()), "Failed to create command buffer!");

	m_isCleanedUp = false;

	// Sync objects
	m_fences.resize(VulkanConfig::MaxFramesInFlight);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VK_CHECK(vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &m_presentSemaphore), "Failed to create present semaphore!");
	VK_CHECK(vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &m_renderSemaphore), "Failed to create render semaphore!");

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (uint32_t i = 0; i < VulkanConfig::MaxFramesInFlight; i++)
		VK_CHECK(vkCreateFence(logicalDevice, &fenceInfo, nullptr, &m_fences[i]), "Failed to create fence!");
}

void Swapchain::Recreate()
{
	GLFWwindow* window = Application::Get().GetWindow();
	int width = 0, height = 0;
	glfwGetFramebufferSize(window, &width, &height);

	while (width == 0 || height == 0) // Window minimized
	{
		glfwGetFramebufferSize(window, &width, &height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(m_logicalDevice->GetNativeDevice());

	Cleanup();
	Create();

	m_recreateNeeded = false;
}

void Swapchain::Cleanup()
{
	if (m_isCleanedUp)
		return;

	auto logicalDevice = m_logicalDevice->GetNativeDevice();

	for (auto& framebuffer : m_framebuffers)
		vkDestroyFramebuffer(logicalDevice, framebuffer, nullptr);

	for (auto& imageView : m_imageViews)
		vkDestroyImageView(logicalDevice, imageView, nullptr);

	vkDestroySwapchainKHR(logicalDevice, m_swapchain, nullptr);

	m_isCleanedUp = true;
}

void Swapchain::Destroy()
{
	auto logicalDevice = m_logicalDevice->GetNativeDevice();

	Cleanup();
	
	vkDestroyRenderPass(logicalDevice, m_renderPass, nullptr);

	vkDestroySemaphore(logicalDevice, m_presentSemaphore, nullptr);
	vkDestroySemaphore(logicalDevice, m_renderSemaphore, nullptr);
	
	for (uint32_t i = 0; i < VulkanConfig::MaxFramesInFlight; i++)
		vkDestroyFence(logicalDevice, m_fences[i], nullptr);

	vkDestroyCommandPool(logicalDevice, m_commandPool, nullptr);
}

void Swapchain::DestroySurface()
{
	auto instance = Application::Get().GetInstance();

	vkDestroySurfaceKHR(instance, m_surface, nullptr);
}

void Swapchain::BeginFrame()
{
	if (m_recreateNeeded)
		Recreate();

	m_currentIndex = GetNextImage();
}

void Swapchain::Present()
{
	vkResetFences(m_logicalDevice->GetNativeDevice(), 1, &m_fences[m_currentFrameIndex]);

	vkResetCommandBuffer(m_commandBuffers[m_currentFrameIndex], 0);
	RecordCommandBuffer(m_commandBuffers[m_currentFrameIndex], m_currentIndex);

	VkResult result;
	VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &m_presentSemaphore;
	submitInfo.pWaitDstStageMask = &waitStage;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_commandBuffers[m_currentFrameIndex];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &m_renderSemaphore;
	
	VK_CHECK(vkQueueSubmit(m_logicalDevice->GetGraphicsQueue(), 1, &submitInfo, m_fences[m_currentFrameIndex]), "Failed to submit queue!");

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &m_renderSemaphore;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &m_swapchain;
	presentInfo.pImageIndices = &m_currentIndex;
	presentInfo.pResults = nullptr;

	result = vkQueuePresentKHR(m_logicalDevice->GetGraphicsQueue(), &presentInfo);
	
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
		OnResize();

	m_currentFrameIndex = (m_currentFrameIndex + 1) % VulkanConfig::MaxFramesInFlight;

	vkWaitForFences(m_logicalDevice->GetNativeDevice(), 1, &m_fences[m_currentFrameIndex], VK_TRUE, UINT64_MAX);
}

void Swapchain::OnResize()
{
	auto logicalDevice = m_logicalDevice->GetNativeDevice();
	vkDeviceWaitIdle(logicalDevice);
	Recreate();
	vkDeviceWaitIdle(logicalDevice);
}

uint32_t Swapchain::GetNextImage()
{
	uint32_t imageIndex{ 0 };
	VkResult result = vkAcquireNextImageKHR(m_logicalDevice->GetNativeDevice(), m_swapchain, UINT64_MAX, m_presentSemaphore, VK_NULL_HANDLE, &imageIndex);

	// TODO: Defer this?
	// VK_SUBOPTIMAL_KHR can be used for rendering, but recreating is advised
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
		m_recreateNeeded = true;

	return imageIndex;
}

void Swapchain::RecordCommandBuffer()
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0;
	beginInfo.pInheritanceInfo = nullptr;

	VK_CHECK(vkBeginCommandBuffer(m_commandBuffers[m_currentFrameIndex], &beginInfo), "Failed to begin command buffer!");

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = m_renderPass;
	renderPassInfo.framebuffer = m_framebuffers[m_currentFrameIndex];
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = m_extent;

	VkClearValue clearColor = { {{ 0.0f, 0.0f, 0.0f, 1.0f }} }; // TODO: This is weird.
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	vkCmdBeginRenderPass(m_commandBuffers[m_currentFrameIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(m_commandBuffers[m_currentFrameIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)m_extent.width;
	viewport.height = (float)m_extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(m_commandBuffers[m_currentFrameIndex], 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = m_extent;
	vkCmdSetScissor(m_commandBuffers[m_currentFrameIndex], 0, 1, &scissor);

	// VBO etc
}

SwapchainSupportDetails Swapchain::QuerySwapchainSupport()
{
	SwapchainSupportDetails details;
	VkPhysicalDevice device = m_logicalDevice->GetPhysicalDevice()->GetNativeDevice();

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

VkSurfaceFormatKHR Swapchain::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	for (const auto& format : availableFormats)
	{
		if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			return format;
	}

	return availableFormats[0];
}

VkPresentModeKHR Swapchain::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	for (const auto& presentMode : availablePresentModes)
	{
		if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			return presentMode;
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Swapchain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	if (capabilities.currentExtent.width != UINT32_MAX)
		return capabilities.currentExtent;

	int width{ 0 }, height{ 0 };
	glfwGetFramebufferSize(Application::Get().GetWindow(), &width, &height);

	VkExtent2D actualExtent = { (uint32_t)width, (uint32_t)height };

	actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
	actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

	return actualExtent;
}
