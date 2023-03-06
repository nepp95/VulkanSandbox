#pragma once

#include "Vulkan.h"

class Buffer
{
public:
	Buffer(uint64_t size);

	void Allocate(uint64_t size);
	void Release();

private:
	VkBuffer m_buffer;
	VkDeviceMemory m_memory;
};