#include "LogicalDevice.h"

LogicalDevice::LogicalDevice(const std::shared_ptr<PhysicalDevice>& physicalDevice)
	: m_physicalDevice(physicalDevice)
{
	bool extensionsSupported = true;
	for (const auto& extension : VulkanConfig::DeviceExtensions)
		if (!m_physicalDevice->IsExtensionSupported(extension))
			extensionsSupported = false;

	if (!extensionsSupported)
		throw std::runtime_error("Not all device extensions were supported!");

	QueueFamilyIndices indices = m_physicalDevice->GetQueueFamilyIndices();
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	float queuePriority = 1.0f;

	VkDeviceQueueCreateInfo graphicsQueueCreateInfo{};
	graphicsQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	graphicsQueueCreateInfo.queueFamilyIndex = indices.Graphics;
	graphicsQueueCreateInfo.queueCount = 1;
	graphicsQueueCreateInfo.pQueuePriorities = &queuePriority;
	queueCreateInfos.push_back(graphicsQueueCreateInfo);

	VkPhysicalDeviceFeatures deviceFeatures = m_physicalDevice->GetDeviceFeatures();

	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.queueCreateInfoCount = (uint32_t)queueCreateInfos.size();
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount = (uint32_t)VulkanConfig::DeviceExtensions.size();
	createInfo.ppEnabledExtensionNames = VulkanConfig::DeviceExtensions.data();
	if (VulkanConfig::EnableValidation)
	{
		createInfo.enabledLayerCount = (uint32_t)VulkanConfig::ValidationLayers.size();
		createInfo.ppEnabledLayerNames = VulkanConfig::ValidationLayers.data();
	} else
	{
		createInfo.enabledLayerCount = 0;
	}
	
	VK_CHECK(vkCreateDevice(m_physicalDevice->GetNativeDevice(), &createInfo, nullptr, &m_device), "Failed to create logical device!");

	vkGetDeviceQueue(m_device, indices.Graphics, 0, &m_graphicsQueue);

	VkCommandPoolCreateInfo commandPoolInfo{};
	commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolInfo.queueFamilyIndex = indices.Graphics;
	commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	VK_CHECK(vkCreateCommandPool(m_device, &commandPoolInfo, nullptr, &m_commandPool), "Failed to create command pool!");
}

void LogicalDevice::Destroy()
{
	vkDestroyDevice(m_device, nullptr);
}

VkCommandBuffer LogicalDevice::GetCommandBuffer(bool begin)
{
	VkCommandBuffer commandBuffer;

	VkCommandBufferAllocateInfo commandBufferInfo{};
	commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferInfo.commandPool = m_commandPool;
	commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferInfo.commandBufferCount = 1;

	VK_CHECK(vkAllocateCommandBuffers(m_device, &commandBufferInfo, &commandBuffer), "Failed to allocate command buffer!");

	if (begin)
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		
		VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo), "Failed to begin command buffer!");
	}

	return commandBuffer;
}

void LogicalDevice::FlushCommandBuffer(VkCommandBuffer commandBuffer)
{
	VK_CHECK(vkEndCommandBuffer(commandBuffer), "Failed to end command buffer requested from logical device!");

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = 0;
	
	// We need this to be done when this function end so we sync it
	VkFence fence;
	VK_CHECK(vkCreateFence(m_device, &fenceInfo, nullptr, &fence), "Failed to create temporary fence!");

	// Submit work
	VK_CHECK(vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, fence), "Failed to submit queue!");
	vkWaitForFences(m_device, 1, &fence, VK_TRUE, UINT64_MAX);

	// Clean up
	vkDestroyFence(m_device, fence, nullptr);
	vkFreeCommandBuffers(m_device, m_commandPool, 1, &commandBuffer);
}
