#pragma once

#include "PhysicalDevice.h"

class LogicalDevice
{
public:
	LogicalDevice(const std::shared_ptr<PhysicalDevice>& physicalDevice);

	VkDevice GetNativeDevice() { return m_device; }
	VkQueue GetGraphicsQueue() { return m_graphicsQueue; }

private:
	std::shared_ptr<PhysicalDevice> m_physicalDevice;
	VkDevice m_device;

	VkQueue m_graphicsQueue;
};
