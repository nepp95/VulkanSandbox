#pragma once

#include "Allocator.h"

class VertexBuffer
{
public:
	VertexBuffer(void* data, uint32_t size);
	~VertexBuffer();

	VkBuffer GetBuffer() const { return m_buffer; }

private:
	uint32_t m_size;

	VkBuffer m_buffer;
	VmaAllocation m_allocation;
};
