#pragma once

#include "../Memory/Allocator.h"

class IndexBuffer
{
public:
	IndexBuffer(void* data, uint32_t size);
	~IndexBuffer();

	VkBuffer GetBuffer() const { return m_buffer; }

private:
	uint32_t m_size;

	VkBuffer m_buffer;
	VmaAllocation m_allocation;
};