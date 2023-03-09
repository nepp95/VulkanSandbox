#pragma once

#include "Vulkan.h"

struct QueueFamilyIndices
{
	int32_t Graphics = -1;

	bool IsComplete()
	{
		return Graphics > -1;
	}
};

class PhysicalDevice
{
public:
	PhysicalDevice();

	const QueueFamilyIndices& GetQueueFamilyIndices() const { return m_indices; }
	const VkPhysicalDeviceFeatures& GetDeviceFeatures() const { return m_features; }
	const VkPhysicalDeviceProperties& GetDeviceProperties() const { return m_properties; }
	const VkPhysicalDevice GetNativeDevice() const { return m_physicalDevice; }

	bool IsExtensionSupported(std::string_view name);

private:
	QueueFamilyIndices FindQueueFamilyIndices();

private:
	VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
	VkPhysicalDeviceProperties m_properties;
	VkPhysicalDeviceFeatures m_features;
	VkPhysicalDeviceMemoryProperties m_memoryProperties;

	QueueFamilyIndices m_indices;

	std::vector<std::string> m_supportedExtensions;
};