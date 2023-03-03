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
}
