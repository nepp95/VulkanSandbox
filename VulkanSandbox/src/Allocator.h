#pragma once

#include "Vma.h"

class Allocator
{
public:
	static VmaAllocation AllocateBuffer(VkBuffer& buffer, VkBufferCreateInfo createInfo, VmaMemoryUsage usage = VMA_MEMORY_USAGE_AUTO);
	static void DestroyBuffer(VkBuffer buffer, VmaAllocation allocation);

	static void* MapMemory(VmaAllocation allocation);
	static void UnmapMemory(VmaAllocation allocation);

	static void Init();
	static void Destroy();
};