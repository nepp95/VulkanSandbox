#include "Allocator.h"

#include "Application.h"

struct Vma
{
	VmaAllocator Allocator;
	uint64_t MemoryUsed;
};

static Vma* s_data = nullptr;

VmaAllocation Allocator::AllocateBuffer(VkBuffer& buffer, VkBufferCreateInfo createInfo, VmaMemoryUsage usage)
{
	VmaAllocationCreateInfo allocCreateInfo{};
	allocCreateInfo.usage = usage;

	VmaAllocationInfo allocationInfo{};

	VmaAllocation allocation;
	vmaCreateBuffer(s_data->Allocator, &createInfo, &allocCreateInfo, &buffer, &allocation, &allocationInfo);

	return allocation;
}

VmaAllocation Allocator::AllocateImage(VkImage& image, VkImageCreateInfo createInfo, VmaMemoryUsage usage)
{
	VmaAllocationCreateInfo allocCreateInfo{};
	allocCreateInfo.usage = usage;

	VmaAllocationInfo allocationInfo{};

	VmaAllocation allocation;
	vmaCreateImage(s_data->Allocator, &createInfo, &allocCreateInfo, &image, &allocation, &allocationInfo);

	return allocation;
}

void Allocator::DestroyBuffer(VkBuffer buffer, VmaAllocation allocation)
{
	vmaDestroyBuffer(s_data->Allocator, buffer, allocation);
}

void Allocator::DestroyImage(VkImage image, VmaAllocation allocation)
{
	vmaDestroyImage(s_data->Allocator, image, allocation);
}

void* Allocator::MapMemory(VmaAllocation allocation)
{
	void* data;
	vmaMapMemory(s_data->Allocator, allocation, &data);
	return data;
}

void Allocator::UnmapMemory(VmaAllocation allocation)
{
	vmaUnmapMemory(s_data->Allocator, allocation);
}

void Allocator::Init()
{
	s_data = new Vma();

	auto app = Application::Get();

	VmaAllocatorCreateInfo vmaInfo{};
	vmaInfo.instance = app.GetInstance();
	vmaInfo.device = app.GetDevice()->GetNativeDevice();
	vmaInfo.physicalDevice = app.GetDevice()->GetPhysicalDevice()->GetNativeDevice();
	vmaInfo.vulkanApiVersion = VK_API_VERSION_1_3;

	VK_CHECK(vmaCreateAllocator(&vmaInfo, &s_data->Allocator), "Failed to create VMA!");
}

void Allocator::Destroy()
{
	vmaDestroyAllocator(s_data->Allocator);
	delete s_data;
}
