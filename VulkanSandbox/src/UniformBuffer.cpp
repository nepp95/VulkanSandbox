#include "UniformBuffer.h"

UniformBuffer::UniformBuffer(const std::shared_ptr<LogicalDevice>& device)
	: m_logicalDevice(device)
{
	VkDeviceSize size = sizeof(UniformBufferObject);

	m_buffers.resize(VulkanConfig::MaxFramesInFlight);
	m_memories.resize(VulkanConfig::MaxFramesInFlight);
	m_memoryMaps.resize(VulkanConfig::MaxFramesInFlight);

	for (size_t i = 0; i < VulkanConfig::MaxFramesInFlight; i++)
	{
		CreateBuffer()
	}
}