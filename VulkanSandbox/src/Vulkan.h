#pragma once

#include <vulkan/vulkan.h>

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
