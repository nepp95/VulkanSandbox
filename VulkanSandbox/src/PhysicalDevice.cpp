#include "PhysicalDevice.h"

#include "Application.h"

PhysicalDevice::PhysicalDevice()
{
	auto instance = Application::Get().GetInstance();

	// Select GPU
	uint32_t deviceCount{ 0 };
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	if (deviceCount == 0)
		throw std::runtime_error("Failed to find GPU's with Vulkan support!");

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	for (const auto& device : devices)
	{
		vkGetPhysicalDeviceProperties(device, &m_properties);
		if (m_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			m_physicalDevice = device;
			break;
		}
	}

	if (!m_physicalDevice)
	{
		LOG("No discrete GPU found!");
		m_physicalDevice = devices.back();
	}

	if (!m_physicalDevice)
		throw std::runtime_error("Failed to find a suitable GPU!");

	// Get properties from selected device
	vkGetPhysicalDeviceFeatures(m_physicalDevice, &m_features);
	vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &m_memoryProperties);

	// Queue Family Indices
	m_indices = FindQueueFamilyIndices();

	// Extensions
	uint32_t extensionCount{ 0 };
	vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extensionCount, availableExtensions.data());

	std::stringstream ss;
	ss << "Selected device has " << availableExtensions.size() << " extensions";
	LOG(ss.str());

	for (const auto& extension : availableExtensions)
		m_supportedExtensions.emplace_back(extension.extensionName);
}

bool PhysicalDevice::IsExtensionSupported(std::string_view name)
{
	return std::find(m_supportedExtensions.begin(), m_supportedExtensions.end(), name) != m_supportedExtensions.end();
}

QueueFamilyIndices PhysicalDevice::FindQueueFamilyIndices()
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount{ 0 };
	vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, queueFamilies.data());

	for (size_t i = 0; i < queueFamilies.size(); i++)
	{
		if (queueFamilies[i].queueFlags && VK_QUEUE_GRAPHICS_BIT)
			indices.Graphics = i;

		//VkBool32 presentSupport = false;
		//vkGetPhysicalDeviceSurfaceSupportKHR(m_physicalDevice, i, m_surface, &presentSupport);

		//if (presentSupport)
		//	indices.PresentFamily = i;

		if (indices.IsComplete())
			break;
	}

	return indices;
}
