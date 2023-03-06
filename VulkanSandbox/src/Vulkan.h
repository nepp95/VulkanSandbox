#pragma once

class LogicalDevice;

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#define LOG(x) std::cout << x << std::endl
#define VK_CHECK(func, msg) { if (func != VK_SUCCESS) LOG(msg); }

struct VulkanConfig
{
	inline static const uint32_t Width = 800;
	inline static const uint32_t Height = 600;

	inline static const bool EnableValidation = true;
	inline static const uint32_t MaxFramesInFlight = 2;
	inline static const std::vector<const char*> ValidationLayers{ "VK_LAYER_KHRONOS_validation" };
	inline static const std::vector<const char*> DeviceExtensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };
};

static uint32_t FindMemoryType(VkPhysicalDevice device, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{

	VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties(device, &memoryProperties);

	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
		if (typeFilter & (1 << i) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
			return i;

	LOG("Failed to find suitable memory type!");
	__debugbreak;

	return 0;
}

inline static void CreateBuffer(const std::shared_ptr<LogicalDevice>& device, VkDeviceSize size, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags propertyFlags, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usageFlags;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VK_CHECK(vkCreateBuffer(device->GetNativeDevice(), &bufferInfo, nullptr, &buffer), "Failed to create buffer!");

	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(device->GetNativeDevice(), buffer, &memoryRequirements);

	VkMemoryAllocateInfo allocateInfo{};
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize = memoryRequirements.size;
	allocateInfo.memoryTypeIndex = FindMemoryType(device->GetPhysicalDevice()->GetNativeDevice(), memoryRequirements.memoryTypeBits, propertyFlags);

	VK_CHECK(vkAllocateMemory(device->GetNativeDevice(), &allocateInfo, nullptr, &bufferMemory), "Failed to allocate vertex buffer memory!");

	vkBindBufferMemory(device->GetNativeDevice(), buffer, bufferMemory, 0);
}

inline static void CopyBuffer(const std::shared_ptr<LogicalDevice>& device, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkCommandBufferAllocateInfo allocateInfo{};
	allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocateInfo.commandPool = m_commandPool;
	allocateInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device->GetNativeDevice(), &allocateInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = size;

	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(device->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(device->GetGraphicsQueue());

	vkFreeCommandBuffers(device->GetNativeDevice(), m_commandPool, 1, &commandBuffer);
}

// File reading
static std::vector<char> ReadBytes(const std::string& filepath)
{
	std::ifstream stream(filepath, std::ios::binary | std::ios::ate);
	if (!stream)
		return {};

	std::streampos end = stream.tellg();
	stream.seekg(0, std::ios::beg);
	uint32_t fileSize = (uint32_t)(end - stream.tellg());

	if (fileSize == 0)
		return {};

	std::vector<char> fileData(fileSize);
	stream.read(fileData.data(), fileSize);
	stream.close();

	return fileData;
}