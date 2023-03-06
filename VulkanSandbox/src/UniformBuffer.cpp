#include "UniformBuffer.h"

UniformBuffer::UniformBuffer(const std::shared_ptr<LogicalDevice>& device)
	: m_logicalDevice(device)
{
	VkDeviceSize size = sizeof(UniformBufferObject);

	m_buffers.resize(VulkanConfig::MaxFramesInFlight);
	m_allocations.resize(VulkanConfig::MaxFramesInFlight);
	m_memoryMaps.resize(VulkanConfig::MaxFramesInFlight);

	VkBufferCreateInfo uboInfo{};
	uboInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	uboInfo.size = size;
	uboInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	uboInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	for (size_t i = 0; i < VulkanConfig::MaxFramesInFlight; i++)
	{
		LOG("Uniform created");
		m_allocations[i] = Allocator::AllocateBuffer(m_buffers[i], uboInfo, VMA_MEMORY_USAGE_CPU_ONLY);
		m_memoryMaps[i] = Allocator::MapMemory(m_allocations[i]);
	}
}

UniformBuffer::~UniformBuffer()
{
	for (uint32_t i = 0; i < VulkanConfig::MaxFramesInFlight; i++)
		Allocator::DestroyBuffer(m_buffers[i], m_allocations[i]);	
}
