#include "IndexBuffer.h"

#include "../Application.h"

IndexBuffer::IndexBuffer(void* data, uint32_t size)
	: m_size(size)
{
	VkBufferCreateInfo stagingBufferInfo{};
	stagingBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	stagingBufferInfo.size = size;
	stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	stagingBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkBuffer stagingBuffer;
	VmaAllocation stagingBufferAlloc = Allocator::AllocateBuffer(stagingBuffer, stagingBufferInfo, VMA_MEMORY_USAGE_CPU_TO_GPU);

	void* memData = Allocator::MapMemory(stagingBufferAlloc);
	memcpy(memData, data, (size_t)size);
	Allocator::UnmapMemory(stagingBufferAlloc);

	VkBufferCreateInfo indexBufferInfo{};
	indexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	indexBufferInfo.size = size;
	indexBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	indexBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	m_allocation = Allocator::AllocateBuffer(m_buffer, indexBufferInfo, VMA_MEMORY_USAGE_GPU_ONLY);

	auto& device = Application::Get().GetDevice();
	VkCommandBuffer commandBuffer = device->GetCommandBuffer(true);
	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = size;

	vkCmdCopyBuffer(commandBuffer, stagingBuffer, m_buffer, 1, &copyRegion);
	device->FlushCommandBuffer(commandBuffer);

	Allocator::DestroyBuffer(stagingBuffer, stagingBufferAlloc);
}

IndexBuffer::~IndexBuffer()
{
	Allocator::DestroyBuffer(m_buffer, m_allocation);
}
