#pragma once

#include "../Memory/Allocator.h"

class VertexBuffer
{
public:
	VertexBuffer(void* data, uint32_t size);
	VertexBuffer(uint32_t size);
	~VertexBuffer();

	void SetData(void* data, uint32_t size);

	VkBuffer GetBuffer() const { return m_buffer; }

private:
	void CreateBuffer(void* data, uint32_t size);

private:
	uint32_t m_size;

	VkBuffer m_buffer;
	VmaAllocation m_allocation;
};
