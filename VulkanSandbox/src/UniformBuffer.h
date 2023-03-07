#pragma once

#include "Allocator.h"
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
	~UniformBuffer();

	const std::vector<VkBuffer>& GetBuffers() const { return m_buffers; }

private:
	std::shared_ptr<LogicalDevice> m_logicalDevice;
	// size?

	std::vector<VkBuffer> m_buffers;
	std::vector<VmaAllocation> m_allocations;
	std::vector<void*> m_memoryMaps;

	friend class Swapchain; // remove
};