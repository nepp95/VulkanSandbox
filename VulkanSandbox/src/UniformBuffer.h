#pragma once

#include "LogicalDevice.h"

struct UniformBufferObject
{
	glm::mat4 Model;
	glm::mat4 View;
	glm::mat4 Projection;
};

class UniformBuffer
{
public:
	UniformBuffer(const std::shared_ptr<LogicalDevice>& device);

private:
	std::shared_ptr<LogicalDevice> m_logicalDevice;

	std::vector<VkBuffer> m_buffers;
	std::vector<VkDeviceMemory> m_memories;
	std::vector<void*> m_memoryMaps;
};