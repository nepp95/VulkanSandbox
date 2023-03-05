#pragma once

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

struct UniformBufferObject
{
	glm::mat4 Model;
	glm::mat4 View;
	glm::mat4 Projection;
};

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